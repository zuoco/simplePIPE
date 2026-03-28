#include "engine/ComponentCatalog.h"

// 内置模板头文件
#include "engine/templates/PipeTemplate.h"
#include "engine/templates/ElbowTemplate.h"
#include "engine/templates/TeeTemplate.h"
#include "engine/templates/ReducerTemplate.h"
#include "engine/templates/GateValveTemplate.h"
#include "engine/templates/WeldNeckFlangeTemplate.h"
#include "engine/templates/RigidSupportTemplate.h"
#include "engine/templates/SpringHangerTemplate.h"

namespace engine {

ComponentCatalog& ComponentCatalog::instance() {
    static ComponentCatalog catalog;
    return catalog;
}

ComponentCatalog::ComponentCatalog() {
    registerBuiltins();
}

void ComponentCatalog::registerBuiltins() {
    registerTemplate(std::make_unique<PipeTemplate>());
    registerTemplate(std::make_unique<ElbowTemplate>());
    registerTemplate(std::make_unique<TeeTemplate>());
    registerTemplate(std::make_unique<ReducerTemplate>());
    registerTemplate(std::make_unique<GateValveTemplate>());
    registerTemplate(std::make_unique<WeldNeckFlangeTemplate>());
    registerTemplate(std::make_unique<RigidSupportTemplate>());
    registerTemplate(std::make_unique<SpringHangerTemplate>());
}

void ComponentCatalog::registerTemplate(std::unique_ptr<ComponentTemplate> tpl) {
    if (!tpl) return;
    std::string id = tpl->templateId();
    templates_[id] = std::move(tpl);
}

ComponentTemplate* ComponentCatalog::getTemplate(const std::string& templateId) const {
    auto it = templates_.find(templateId);
    return it != templates_.end() ? it->second.get() : nullptr;
}

std::vector<std::string> ComponentCatalog::allTemplateIds() const {
    std::vector<std::string> ids;
    ids.reserve(templates_.size());
    for (const auto& [id, _] : templates_) {
        ids.push_back(id);
    }
    return ids;
}

size_t ComponentCatalog::size() const {
    return templates_.size();
}

void ComponentCatalog::clear() {
    templates_.clear();
}

} // namespace engine
