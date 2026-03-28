#include "app/ProjectSerializer.h"

#include "app/DependencyGraph.h"
#include "engine/RecomputeEngine.h"
#include "model/Beam.h"
#include "model/FixedPoint.h"
#include "model/Flange.h"
#include "model/Gasket.h"
#include "model/PipePoint.h"
#include "model/PipeSpec.h"
#include "model/ProjectConfig.h"
#include "model/Route.h"
#include "model/SealRing.h"
#include "model/Segment.h"
#include "model/Support.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace app {
namespace {

using json = nlohmann::json;

std::array<double, 3> pointToArray(const gp_Pnt& p) {
    return {p.X(), p.Y(), p.Z()};
}

gp_Pnt arrayToPoint(const json& j) {
    return gp_Pnt(j.at(0).get<double>(), j.at(1).get<double>(), j.at(2).get<double>());
}

std::array<double, 3> vecToArray(const gp_Vec& v) {
    return {v.X(), v.Y(), v.Z()};
}

gp_Vec arrayToVec(const json& j) {
    return gp_Vec(j.at(0).get<double>(), j.at(1).get<double>(), j.at(2).get<double>());
}

json variantToJson(const foundation::Variant& value) {
    if (const auto* d = std::get_if<double>(&value)) {
        return json{{"type", "double"}, {"value", *d}};
    }
    if (const auto* i = std::get_if<int>(&value)) {
        return json{{"type", "int"}, {"value", *i}};
    }
    return json{{"type", "string"}, {"value", std::get<std::string>(value)}};
}

foundation::Variant jsonToVariant(const json& j) {
    const std::string type = j.at("type").get<std::string>();
    if (type == "double") return j.at("value").get<double>();
    if (type == "int") return j.at("value").get<int>();
    if (type == "string") return j.at("value").get<std::string>();
    throw std::runtime_error("unsupported variant type: " + type);
}

json variantMapToJson(const std::map<std::string, foundation::Variant>& fields) {
    json j = json::object();
    for (const auto& [k, v] : fields) {
        j[k] = variantToJson(v);
    }
    return j;
}

void jsonToVariantMap(const json& j, const std::function<void(const std::string&, const foundation::Variant&)>& setter) {
    for (auto it = j.begin(); it != j.end(); ++it) {
        setter(it.key(), jsonToVariant(it.value()));
    }
}

std::string pipePointTypeToString(model::PipePointType type) {
    switch (type) {
    case model::PipePointType::Run: return "Run";
    case model::PipePointType::Bend: return "Bend";
    case model::PipePointType::Reducer: return "Reducer";
    case model::PipePointType::Tee: return "Tee";
    case model::PipePointType::Valve: return "Valve";
    case model::PipePointType::FlexJoint: return "FlexJoint";
    }
    return "Run";
}

model::PipePointType stringToPipePointType(const std::string& s) {
    if (s == "Run") return model::PipePointType::Run;
    if (s == "Bend") return model::PipePointType::Bend;
    if (s == "Reducer") return model::PipePointType::Reducer;
    if (s == "Tee") return model::PipePointType::Tee;
    if (s == "Valve") return model::PipePointType::Valve;
    if (s == "FlexJoint") return model::PipePointType::FlexJoint;
    throw std::runtime_error("unsupported PipePointType: " + s);
}

std::string supportTypeToString(model::SupportType type) {
    switch (type) {
    case model::SupportType::Rod: return "Rod";
    case model::SupportType::Spring: return "Spring";
    case model::SupportType::Rigid: return "Rigid";
    case model::SupportType::Guide: return "Guide";
    }
    return "Rigid";
}

model::SupportType stringToSupportType(const std::string& s) {
    if (s == "Rod") return model::SupportType::Rod;
    if (s == "Spring") return model::SupportType::Spring;
    if (s == "Rigid") return model::SupportType::Rigid;
    if (s == "Guide") return model::SupportType::Guide;
    throw std::runtime_error("unsupported SupportType: " + s);
}

std::string beamSectionTypeToString(model::BeamSectionType type) {
    switch (type) {
    case model::BeamSectionType::Rectangular: return "Rectangular";
    case model::BeamSectionType::HSection: return "HSection";
    }
    return "Rectangular";
}

model::BeamSectionType stringToBeamSectionType(const std::string& s) {
    if (s == "Rectangular") return model::BeamSectionType::Rectangular;
    if (s == "HSection") return model::BeamSectionType::HSection;
    throw std::runtime_error("unsupported BeamSectionType: " + s);
}

std::string unitSystemToString(foundation::UnitSystem us) {
    return (us == foundation::UnitSystem::Imperial) ? "Imperial" : "SI";
}

foundation::UnitSystem stringToUnitSystem(const std::string& s) {
    if (s == "Imperial") return foundation::UnitSystem::Imperial;
    return foundation::UnitSystem::SI;
}

int hexValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    throw std::runtime_error("invalid hex character in UUID");
}

foundation::UUID parseUuid(const std::string& s) {
    std::string hex;
    hex.reserve(32);
    for (char c : s) {
        if (c == '-') continue;
        hex.push_back(c);
    }
    if (hex.size() != 32) {
        throw std::runtime_error("invalid UUID string length");
    }

    foundation::UUID id;
    for (std::size_t i = 0; i < 16; ++i) {
        const int hi = hexValue(hex[2 * i]);
        const int lo = hexValue(hex[2 * i + 1]);
        id.data[i] = static_cast<uint8_t>((hi << 4) | lo);
    }
    return id;
}

std::string accessoryClassName(const model::Accessory& acc) {
    if (dynamic_cast<const model::FixedPoint*>(&acc)) return "FixedPoint";
    if (dynamic_cast<const model::Support*>(&acc)) return "Support";
    if (dynamic_cast<const model::Flange*>(&acc)) return "Flange";
    if (dynamic_cast<const model::Gasket*>(&acc)) return "Gasket";
    if (dynamic_cast<const model::SealRing*>(&acc)) return "SealRing";
    return "Accessory";
}

std::vector<model::PipeSpec*> sortedPipeSpecs(const Document& doc) {
    auto specs = doc.findByType<model::PipeSpec>();
    std::sort(specs.begin(), specs.end(), [](const auto* a, const auto* b) {
        return a->id().toString() < b->id().toString();
    });
    return specs;
}

std::vector<model::Route*> sortedRoutes(const Document& doc) {
    auto routes = doc.findByType<model::Route>();
    std::sort(routes.begin(), routes.end(), [](const auto* a, const auto* b) {
        return a->id().toString() < b->id().toString();
    });
    return routes;
}

std::vector<model::Accessory*> sortedAccessories(const Document& doc) {
    auto accessories = doc.findByType<model::Accessory>();
    std::sort(accessories.begin(), accessories.end(), [](const auto* a, const auto* b) {
        return a->id().toString() < b->id().toString();
    });
    return accessories;
}

std::vector<model::Beam*> sortedBeams(const Document& doc) {
    auto beams = doc.findByType<model::Beam>();
    std::sort(beams.begin(), beams.end(), [](const auto* a, const auto* b) {
        return a->id().toString() < b->id().toString();
    });
    return beams;
}

} // namespace

bool ProjectSerializer::save(const Document& document, const std::string& filePath) {
    try {
        json root;
        root["version"] = "0.1.0";

        json projectConfig = json::object();
        auto cfgs = document.findByType<model::ProjectConfig>();
        if (!cfgs.empty()) {
            const auto* cfg = cfgs.front();
            projectConfig["id"] = cfg->id().toString();
            projectConfig["name"] = cfg->name();
            projectConfig["projectName"] = cfg->projectName();
            projectConfig["author"] = cfg->author();
            projectConfig["standard"] = cfg->standard();
            projectConfig["unitSystem"] = unitSystemToString(cfg->unitSystem());
            projectConfig["fields"] = variantMapToJson(cfg->fields());
        } else {
            projectConfig["name"] = document.name();
            projectConfig["projectName"] = document.name();
            projectConfig["author"] = "";
            projectConfig["standard"] = "";
            projectConfig["unitSystem"] = "SI";
            projectConfig["fields"] = json::object();
        }
        root["projectConfig"] = projectConfig;

        root["pipeSpecs"] = json::array();
        for (const auto* spec : sortedPipeSpecs(document)) {
            root["pipeSpecs"].push_back(json{
                {"id", spec->id().toString()},
                {"name", spec->name()},
                {"fields", variantMapToJson(spec->fields())}
            });
        }

        root["routes"] = json::array();
        for (const auto* route : sortedRoutes(document)) {
            json routeJson = {
                {"id", route->id().toString()},
                {"name", route->name()},
                {"segments", json::array()}
            };

            for (const auto& seg : route->segments()) {
                json segJson = {
                    {"id", seg->id().toString()},
                    {"name", seg->name()},
                    {"segmentId", seg->name()},
                    {"pipePoints", json::array()}
                };

                for (const auto& pp : seg->points()) {
                    json ppJson = {
                        {"id", pp->id().toString()},
                        {"name", pp->name()},
                        {"type", pipePointTypeToString(pp->type())},
                        {"position", pointToArray(pp->position())},
                        {"typeParams", variantMapToJson(pp->typeParams())}
                    };
                    if (pp->pipeSpec()) {
                        ppJson["pipeSpecId"] = pp->pipeSpec()->id().toString();
                    } else {
                        ppJson["pipeSpecId"] = "";
                    }
                    segJson["pipePoints"].push_back(std::move(ppJson));
                }

                routeJson["segments"].push_back(std::move(segJson));
            }

            root["routes"].push_back(std::move(routeJson));
        }

        root["accessories"] = json::array();
        for (const auto* acc : sortedAccessories(document)) {
            json accJson = {
                {"id", acc->id().toString()},
                {"name", acc->name()},
                {"class", accessoryClassName(*acc)},
                {"position", pointToArray(acc->position())},
                {"offset", vecToArray(acc->offset())},
                {"pipePointId", acc->pipePoint() ? acc->pipePoint()->id().toString() : ""}
            };

            if (const auto* support = dynamic_cast<const model::Support*>(acc)) {
                accJson["supportType"] = supportTypeToString(support->supportType());
                const gp_Dir& d = support->loadDirection();
                accJson["loadDirection"] = std::array<double, 3>{d.X(), d.Y(), d.Z()};
            } else if (const auto* flange = dynamic_cast<const model::Flange*>(acc)) {
                accJson["rating"] = flange->rating();
                accJson["faceType"] = flange->faceType();
                accJson["boltHoleCount"] = flange->boltHoleCount();
            } else if (const auto* gasket = dynamic_cast<const model::Gasket*>(acc)) {
                accJson["gasketMaterial"] = gasket->gasketMaterial();
                accJson["thickness"] = gasket->thickness();
            } else if (const auto* seal = dynamic_cast<const model::SealRing*>(acc)) {
                accJson["sealMaterial"] = seal->sealMaterial();
                accJson["crossSectionDiameter"] = seal->crossSectionDiameter();
            }

            root["accessories"].push_back(std::move(accJson));
        }

        root["beams"] = json::array();
        for (const auto* beam : sortedBeams(document)) {
            root["beams"].push_back(json{
                {"id", beam->id().toString()},
                {"name", beam->name()},
                {"position", pointToArray(beam->position())},
                {"sectionType", beamSectionTypeToString(beam->sectionType())},
                {"width", beam->width()},
                {"height", beam->height()},
                {"startPointId", beam->startPoint() ? beam->startPoint()->id().toString() : ""},
                {"endPointId", beam->endPoint() ? beam->endPoint()->id().toString() : ""}
            });
        }

        std::ofstream out(filePath);
        if (!out.is_open()) return false;
        out << root.dump(2) << '\n';
        return true;
    } catch (...) {
        return false;
    }
}

std::unique_ptr<Document> ProjectSerializer::load(const std::string& filePath) {
    try {
        std::ifstream in(filePath);
        if (!in.is_open()) return nullptr;

        json root;
        in >> root;

        auto document = std::make_unique<Document>();

        std::unordered_map<std::string, std::shared_ptr<model::PipeSpec>> specsById;
        std::unordered_map<std::string, std::shared_ptr<model::PipePoint>> pointsById;

        if (root.contains("projectConfig") && root["projectConfig"].is_object()) {
            const json& cfgJson = root["projectConfig"];
            auto cfg = std::make_shared<model::ProjectConfig>(cfgJson.value("name", ""));
            if (cfgJson.contains("id")) {
                cfg->setIdForDeserialization(parseUuid(cfgJson.at("id").get<std::string>()));
            }

            cfg->setProjectName(cfgJson.value("projectName", cfgJson.value("name", "Untitled")));
            cfg->setAuthor(cfgJson.value("author", ""));
            cfg->setStandard(cfgJson.value("standard", ""));
            cfg->setUnitSystem(stringToUnitSystem(cfgJson.value("unitSystem", "SI")));

            if (cfgJson.contains("fields") && cfgJson["fields"].is_object()) {
                jsonToVariantMap(cfgJson["fields"], [&](const std::string& k, const foundation::Variant& v) {
                    cfg->setField(k, v);
                });
            }

            document->setName(cfg->projectName());
            document->addObject(cfg);
        }

        if (root.contains("pipeSpecs") && root["pipeSpecs"].is_array()) {
            for (const auto& specJson : root["pipeSpecs"]) {
                auto spec = std::make_shared<model::PipeSpec>(specJson.value("name", ""));
                if (specJson.contains("id")) {
                    spec->setIdForDeserialization(parseUuid(specJson.at("id").get<std::string>()));
                }
                if (specJson.contains("fields") && specJson["fields"].is_object()) {
                    jsonToVariantMap(specJson["fields"], [&](const std::string& k, const foundation::Variant& v) {
                        spec->setField(k, v);
                    });
                }
                const std::string id = spec->id().toString();
                specsById[id] = spec;
                document->addObject(spec);
            }
        }

        if (root.contains("routes") && root["routes"].is_array()) {
            for (const auto& routeJson : root["routes"]) {
                auto route = std::make_shared<model::Route>(routeJson.value("name", ""));
                if (routeJson.contains("id")) {
                    route->setIdForDeserialization(parseUuid(routeJson.at("id").get<std::string>()));
                }
                document->addObject(route);

                if (!routeJson.contains("segments") || !routeJson["segments"].is_array()) continue;
                for (const auto& segJson : routeJson["segments"]) {
                    auto seg = std::make_shared<model::Segment>(segJson.value("name", ""));
                    if (segJson.contains("id")) {
                        seg->setIdForDeserialization(parseUuid(segJson.at("id").get<std::string>()));
                    }

                    route->addSegment(seg);
                    document->addObject(seg);

                    if (!segJson.contains("pipePoints") || !segJson["pipePoints"].is_array()) continue;
                    for (const auto& ppJson : segJson["pipePoints"]) {
                        const gp_Pnt pos = ppJson.contains("position")
                            ? arrayToPoint(ppJson["position"])
                            : gp_Pnt(0.0, 0.0, 0.0);

                        auto pp = std::make_shared<model::PipePoint>(
                            ppJson.value("name", ""),
                            stringToPipePointType(ppJson.value("type", "Run")),
                            pos);

                        if (ppJson.contains("id")) {
                            pp->setIdForDeserialization(parseUuid(ppJson.at("id").get<std::string>()));
                        }

                        if (ppJson.contains("pipeSpecId")) {
                            const std::string specId = ppJson.at("pipeSpecId").get<std::string>();
                            auto it = specsById.find(specId);
                            if (it != specsById.end()) {
                                pp->setPipeSpec(it->second);
                            }
                        }

                        if (ppJson.contains("typeParams") && ppJson["typeParams"].is_object()) {
                            jsonToVariantMap(ppJson["typeParams"], [&](const std::string& k, const foundation::Variant& v) {
                                pp->setParam(k, v);
                            });
                        }

                        seg->addPoint(pp);
                        pointsById[pp->id().toString()] = pp;
                        document->addObject(pp);
                    }
                }
            }
        }

        if (root.contains("accessories") && root["accessories"].is_array()) {
            for (const auto& accJson : root["accessories"]) {
                const std::string cls = accJson.value("class", "Accessory");
                const gp_Pnt pos = accJson.contains("position")
                    ? arrayToPoint(accJson["position"])
                    : gp_Pnt(0.0, 0.0, 0.0);
                const std::string name = accJson.value("name", "");

                std::shared_ptr<model::Accessory> acc;
                if (cls == "FixedPoint") {
                    acc = std::make_shared<model::FixedPoint>(name, pos);
                } else if (cls == "Support") {
                    auto support = std::make_shared<model::Support>(name, model::SupportType::Rigid, pos);
                    support->setSupportType(stringToSupportType(accJson.value("supportType", "Rigid")));
                    if (accJson.contains("loadDirection") && accJson["loadDirection"].is_array()) {
                        const auto& d = accJson["loadDirection"];
                        support->setLoadDirection(gp_Dir(
                            d.at(0).get<double>(),
                            d.at(1).get<double>(),
                            d.at(2).get<double>()));
                    }
                    acc = support;
                } else if (cls == "Flange") {
                    auto flange = std::make_shared<model::Flange>(name, pos);
                    flange->setRating(accJson.value("rating", ""));
                    flange->setFaceType(accJson.value("faceType", ""));
                    flange->setBoltHoleCount(accJson.value("boltHoleCount", 0));
                    acc = flange;
                } else if (cls == "Gasket") {
                    auto gasket = std::make_shared<model::Gasket>(name, pos);
                    gasket->setGasketMaterial(accJson.value("gasketMaterial", ""));
                    gasket->setThickness(accJson.value("thickness", 0.0));
                    acc = gasket;
                } else if (cls == "SealRing") {
                    auto seal = std::make_shared<model::SealRing>(name, pos);
                    seal->setSealMaterial(accJson.value("sealMaterial", ""));
                    seal->setCrossSectionDiameter(accJson.value("crossSectionDiameter", 0.0));
                    acc = seal;
                } else {
                    acc = std::make_shared<model::Accessory>(name, pos);
                }

                if (accJson.contains("id")) {
                    acc->setIdForDeserialization(parseUuid(accJson.at("id").get<std::string>()));
                }

                if (accJson.contains("offset") && accJson["offset"].is_array()) {
                    acc->setOffset(arrayToVec(accJson["offset"]));
                }

                if (accJson.contains("pipePointId")) {
                    const std::string ppId = accJson.at("pipePointId").get<std::string>();
                    auto it = pointsById.find(ppId);
                    if (it != pointsById.end()) {
                        acc->attachTo(it->second);
                        it->second->addAccessory(acc);
                    }
                }

                document->addObject(acc);
            }
        }

        if (root.contains("beams") && root["beams"].is_array()) {
            for (const auto& beamJson : root["beams"]) {
                const gp_Pnt pos = beamJson.contains("position")
                    ? arrayToPoint(beamJson["position"])
                    : gp_Pnt(0.0, 0.0, 0.0);

                auto beam = std::make_shared<model::Beam>(beamJson.value("name", ""), pos);
                if (beamJson.contains("id")) {
                    beam->setIdForDeserialization(parseUuid(beamJson.at("id").get<std::string>()));
                }

                beam->setSectionType(stringToBeamSectionType(beamJson.value("sectionType", "Rectangular")));
                beam->setWidth(beamJson.value("width", 100.0));
                beam->setHeight(beamJson.value("height", 200.0));

                const std::string startId = beamJson.value("startPointId", "");
                const std::string endId = beamJson.value("endPointId", "");

                auto itStart = pointsById.find(startId);
                if (itStart != pointsById.end()) {
                    beam->setStartPoint(itStart->second);
                }
                auto itEnd = pointsById.find(endId);
                if (itEnd != pointsById.end()) {
                    beam->setEndPoint(itEnd->second);
                }

                document->addObject(beam);
            }
        }

        DependencyGraph graph;
        engine::RecomputeEngine recompute(*document, graph);
        recompute.recomputeAll();

        return document;
    } catch (...) {
        return nullptr;
    }
}

} // namespace app
