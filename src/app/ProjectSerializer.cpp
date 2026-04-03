// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "app/ProjectSerializer.h"

#include "app/DependencyGraph.h"
#include "engine/RecomputeEngine.h"
#include "model/Beam.h"
#include "model/DeadWeightLoad.h"
#include "model/DisplacementLoad.h"
#include "model/FixedPoint.h"
#include "model/Flange.h"
#include "model/Gasket.h"
#include "model/LoadCase.h"
#include "model/LoadCombination.h"
#include "model/PipePoint.h"
#include "model/PipeSpec.h"
#include "model/PressureLoad.h"
#include "model/ProjectConfig.h"
#include "model/Route.h"
#include "model/SealRing.h"
#include "model/SeismicLoad.h"
#include "model/Segment.h"
#include "model/Support.h"
#include "model/ThermalLoad.h"
#include "model/UserDefinedLoad.h"
#include "model/WindLoad.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <functional>
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

std::array<double, 3> vec3ToArray(const foundation::math::Vec3& v) {
    return {v.x, v.y, v.z};
}

foundation::math::Vec3 arrayToVec3(const json& j) {
    return foundation::math::Vec3{j.at(0).get<double>(), j.at(1).get<double>(), j.at(2).get<double>()};
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

std::string combineMethodToString(model::CombineMethod method) {
    switch (method) {
    case model::CombineMethod::Algebraic: return "Algebraic";
    case model::CombineMethod::Absolute: return "Absolute";
    case model::CombineMethod::SRSS: return "SRSS";
    case model::CombineMethod::Envelope: return "Envelope";
    }
    return "Algebraic";
}

model::CombineMethod stringToCombineMethod(const std::string& s) {
    if (s == "Algebraic") return model::CombineMethod::Algebraic;
    if (s == "Absolute") return model::CombineMethod::Absolute;
    if (s == "SRSS") return model::CombineMethod::SRSS;
    if (s == "Envelope") return model::CombineMethod::Envelope;
    throw std::runtime_error("unsupported CombineMethod: " + s);
}

std::string stressCategoryToString(model::StressCategory category) {
    switch (category) {
    case model::StressCategory::Sustained: return "Sustained";
    case model::StressCategory::Expansion: return "Expansion";
    case model::StressCategory::Occasional: return "Occasional";
    case model::StressCategory::Operating: return "Operating";
    case model::StressCategory::Hydrotest: return "Hydrotest";
    }
    return "Sustained";
}

model::StressCategory stringToStressCategory(const std::string& s) {
    if (s == "Sustained") return model::StressCategory::Sustained;
    if (s == "Expansion") return model::StressCategory::Expansion;
    if (s == "Occasional") return model::StressCategory::Occasional;
    if (s == "Operating") return model::StressCategory::Operating;
    if (s == "Hydrotest") return model::StressCategory::Hydrotest;
    throw std::runtime_error("unsupported StressCategory: " + s);
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

std::vector<model::Load*> sortedLoads(const Document& doc) {
    auto loads = doc.findByType<model::Load>();
    std::sort(loads.begin(), loads.end(), [](const auto* a, const auto* b) {
        return a->id().toString() < b->id().toString();
    });
    return loads;
}

std::vector<model::LoadCase*> sortedLoadCases(const Document& doc) {
    auto loadCases = doc.findByType<model::LoadCase>();
    std::sort(loadCases.begin(), loadCases.end(), [](const auto* a, const auto* b) {
        return a->id().toString() < b->id().toString();
    });
    return loadCases;
}

std::vector<model::LoadCombination*> sortedLoadCombinations(const Document& doc) {
    auto loadCombinations = doc.findByType<model::LoadCombination>();
    std::sort(loadCombinations.begin(), loadCombinations.end(), [](const auto* a, const auto* b) {
        return a->id().toString() < b->id().toString();
    });
    return loadCombinations;
}

std::shared_ptr<model::Load> createLoadByType(const std::string& loadType, const std::string& name) {
    if (loadType == "DeadWeight") return std::make_shared<model::DeadWeightLoad>(name);
    if (loadType == "Thermal") return std::make_shared<model::ThermalLoad>(name);
    if (loadType == "Pressure") return std::make_shared<model::PressureLoad>(name);
    if (loadType == "Wind") return std::make_shared<model::WindLoad>(name);
    if (loadType == "Seismic") return std::make_shared<model::SeismicLoad>(name);
    if (loadType == "Displacement") return std::make_shared<model::DisplacementLoad>(name);
    if (loadType == "UserDefined") return std::make_shared<model::UserDefinedLoad>(name);
    throw std::runtime_error("unsupported load type: " + loadType);
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

        root["loads"] = json::array();
        for (const auto* load : sortedLoads(document)) {
            json loadJson = {
                {"id", load->id().toString()},
                {"name", load->name()},
                {"loadType", load->loadType()},
                {"affectedObjectIds", json::array()}
            };

            for (const auto& affectedId : load->affectedObjects()) {
                loadJson["affectedObjectIds"].push_back(affectedId.toString());
            }

            if (const auto* thermal = dynamic_cast<const model::ThermalLoad*>(load)) {
                loadJson["installTemp"] = thermal->installTemp();
                loadJson["operatingTemp"] = thermal->operatingTemp();
            } else if (const auto* pressure = dynamic_cast<const model::PressureLoad*>(load)) {
                loadJson["pressure"] = pressure->pressure();
                loadJson["isExternal"] = pressure->isExternal();
            } else if (const auto* wind = dynamic_cast<const model::WindLoad*>(load)) {
                loadJson["speed"] = wind->speed();
                loadJson["direction"] = vec3ToArray(wind->direction());
            } else if (const auto* seismic = dynamic_cast<const model::SeismicLoad*>(load)) {
                loadJson["acceleration"] = seismic->acceleration();
                loadJson["direction"] = vec3ToArray(seismic->direction());
            } else if (const auto* displacement = dynamic_cast<const model::DisplacementLoad*>(load)) {
                loadJson["translation"] = vec3ToArray(displacement->translation());
                loadJson["rotation"] = vec3ToArray(displacement->rotation());
            } else if (const auto* userDefined = dynamic_cast<const model::UserDefinedLoad*>(load)) {
                loadJson["force"] = vec3ToArray(userDefined->force());
                loadJson["moment"] = vec3ToArray(userDefined->moment());
            }

            root["loads"].push_back(std::move(loadJson));
        }

        root["loadCases"] = json::array();
        for (const auto* loadCase : sortedLoadCases(document)) {
            json loadCaseJson = {
                {"id", loadCase->id().toString()},
                {"name", loadCase->name()},
                {"entries", json::array()}
            };

            for (const auto& entry : loadCase->entries()) {
                loadCaseJson["entries"].push_back(json{
                    {"loadId", entry.loadId.toString()},
                    {"factor", entry.factor}
                });
            }

            root["loadCases"].push_back(std::move(loadCaseJson));
        }

        root["loadCombinations"] = json::array();
        for (const auto* combination : sortedLoadCombinations(document)) {
            json combinationJson = {
                {"id", combination->id().toString()},
                {"name", combination->name()},
                {"category", stressCategoryToString(combination->category())},
                {"method", combineMethodToString(combination->method())},
                {"caseEntries", json::array()}
            };

            for (const auto& caseEntry : combination->caseEntries()) {
                combinationJson["caseEntries"].push_back(json{
                    {"caseId", caseEntry.caseId.toString()},
                    {"factor", caseEntry.factor}
                });
            }

            root["loadCombinations"].push_back(std::move(combinationJson));
        }

        std::ofstream out(filePath);
        if (!out.is_open()) return false;
        out << root.dump(2) << '\n';
        return true;
    } catch (...) {
        return false;
    }
}

std::unique_ptr<Document> ProjectSerializer::load(const std::string& filePath,
                                                  DependencyGraph* dependencyGraph) {
    try {
        std::ifstream in(filePath);
        if (!in.is_open()) return nullptr;

        json root;
        in >> root;

        auto document = std::make_unique<Document>();

        std::unordered_map<std::string, std::shared_ptr<model::PipeSpec>> specsById;
        std::unordered_map<std::string, std::shared_ptr<model::PipePoint>> pointsById;
        std::unordered_map<std::string, std::shared_ptr<model::Load>> loadsById;
        std::unordered_map<std::string, std::shared_ptr<model::LoadCase>> loadCasesById;

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

        if (root.contains("loads") && root["loads"].is_array()) {
            for (const auto& loadJson : root["loads"]) {
                const std::string loadType = loadJson.value("loadType", "DeadWeight");
                const std::string loadName = loadJson.value("name", loadType);

                auto load = createLoadByType(loadType, loadName);
                if (loadJson.contains("id")) {
                    load->setIdForDeserialization(parseUuid(loadJson.at("id").get<std::string>()));
                }

                if (loadJson.contains("affectedObjectIds") && loadJson["affectedObjectIds"].is_array()) {
                    for (const auto& idJson : loadJson["affectedObjectIds"]) {
                        load->addAffectedObject(parseUuid(idJson.get<std::string>()));
                    }
                }

                if (auto* thermal = dynamic_cast<model::ThermalLoad*>(load.get())) {
                    thermal->setInstallTemp(loadJson.value("installTemp", thermal->installTemp()));
                    thermal->setOperatingTemp(loadJson.value("operatingTemp", thermal->operatingTemp()));
                } else if (auto* pressure = dynamic_cast<model::PressureLoad*>(load.get())) {
                    pressure->setPressure(loadJson.value("pressure", pressure->pressure()));
                    pressure->setIsExternal(loadJson.value("isExternal", pressure->isExternal()));
                } else if (auto* wind = dynamic_cast<model::WindLoad*>(load.get())) {
                    wind->setSpeed(loadJson.value("speed", wind->speed()));
                    if (loadJson.contains("direction") && loadJson["direction"].is_array()) {
                        wind->setDirection(arrayToVec3(loadJson["direction"]));
                    }
                } else if (auto* seismic = dynamic_cast<model::SeismicLoad*>(load.get())) {
                    seismic->setAcceleration(loadJson.value("acceleration", seismic->acceleration()));
                    if (loadJson.contains("direction") && loadJson["direction"].is_array()) {
                        seismic->setDirection(arrayToVec3(loadJson["direction"]));
                    }
                } else if (auto* displacement = dynamic_cast<model::DisplacementLoad*>(load.get())) {
                    if (loadJson.contains("translation") && loadJson["translation"].is_array()) {
                        displacement->setTranslation(arrayToVec3(loadJson["translation"]));
                    }
                    if (loadJson.contains("rotation") && loadJson["rotation"].is_array()) {
                        displacement->setRotation(arrayToVec3(loadJson["rotation"]));
                    }
                } else if (auto* userDefined = dynamic_cast<model::UserDefinedLoad*>(load.get())) {
                    if (loadJson.contains("force") && loadJson["force"].is_array()) {
                        userDefined->setForce(arrayToVec3(loadJson["force"]));
                    }
                    if (loadJson.contains("moment") && loadJson["moment"].is_array()) {
                        userDefined->setMoment(arrayToVec3(loadJson["moment"]));
                    }
                }

                const std::string id = load->id().toString();
                loadsById[id] = load;
                document->addObject(load);
            }
        }

        if (root.contains("loadCases") && root["loadCases"].is_array()) {
            for (const auto& loadCaseJson : root["loadCases"]) {
                auto loadCase = std::make_shared<model::LoadCase>(loadCaseJson.value("name", ""));
                if (loadCaseJson.contains("id")) {
                    loadCase->setIdForDeserialization(parseUuid(loadCaseJson.at("id").get<std::string>()));
                }

                if (loadCaseJson.contains("entries") && loadCaseJson["entries"].is_array()) {
                    for (const auto& entryJson : loadCaseJson["entries"]) {
                        const std::string loadIdStr = entryJson.value("loadId", "");
                        if (loadIdStr.empty()) continue;
                        if (loadsById.find(loadIdStr) == loadsById.end()) continue;

                        model::LoadEntry entry;
                        entry.loadId = parseUuid(loadIdStr);
                        entry.factor = entryJson.value("factor", 1.0);
                        loadCase->addEntry(entry);
                    }
                }

                const std::string id = loadCase->id().toString();
                loadCasesById[id] = loadCase;
                document->addObject(loadCase);
            }
        }

        if (root.contains("loadCombinations") && root["loadCombinations"].is_array()) {
            for (const auto& comboJson : root["loadCombinations"]) {
                auto combination = std::make_shared<model::LoadCombination>(
                    comboJson.value("name", ""),
                    stringToStressCategory(comboJson.value("category", "Sustained")),
                    stringToCombineMethod(comboJson.value("method", "Algebraic")));

                if (comboJson.contains("id")) {
                    combination->setIdForDeserialization(parseUuid(comboJson.at("id").get<std::string>()));
                }

                if (comboJson.contains("caseEntries") && comboJson["caseEntries"].is_array()) {
                    for (const auto& entryJson : comboJson["caseEntries"]) {
                        const std::string caseIdStr = entryJson.value("caseId", "");
                        if (caseIdStr.empty()) continue;
                        if (loadCasesById.find(caseIdStr) == loadCasesById.end()) continue;

                        model::CaseEntry entry;
                        entry.caseId = parseUuid(caseIdStr);
                        entry.factor = entryJson.value("factor", 1.0);
                        combination->addCaseEntry(entry);
                    }
                }

                document->addObject(combination);
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

        if (dependencyGraph) {
            dependencyGraph->rebuildLoadDependencyChain(*document);
        }

        DependencyGraph recomputeGraph;
        engine::RecomputeEngine recompute(*document, recomputeGraph);
        recompute.recomputeAll();

        return document;
    } catch (...) {
        return nullptr;
    }
}

} // namespace app
