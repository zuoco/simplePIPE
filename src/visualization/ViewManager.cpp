#include "visualization/ViewManager.h"

namespace visualization {

ViewManager::ViewManager() {
    // 默认所有 Category 可见
    visibility_[Category::PipePoints]    = true;
    visibility_[Category::Segments]      = true;
    visibility_[Category::Accessories]   = true;
    visibility_[Category::Supports]      = true;
    visibility_[Category::Beams]         = true;
    visibility_[Category::Annotations]   = true;
    visibility_[Category::LoadArrows]    = true;
    visibility_[Category::StressContour] = true;
}

void ViewManager::setVsgComponents(SceneManager* scene, CameraController* camera, SceneFurniture* furniture) {
    vsgScene_  = scene;
    vsgCamera_ = camera;
    vsgFurni_  = furniture;
}

// === 视口路由 ===

void ViewManager::setActiveViewport(ActiveViewport vp) {
    activeVp_ = vp;
}

ViewManager::ActiveViewport ViewManager::activeViewport() const {
    return activeVp_;
}

// === 相机控制 ===

void ViewManager::fitAll() {
    if (activeVp_ == ActiveViewport::VSG && vsgCamera_ && vsgScene_) {
        vsgCamera_->fitAll(vsgScene_->root());
    }
    // VTK 分支：T38/T42 时实现
}

void ViewManager::setViewPreset(ViewPreset preset) {
    if (activeVp_ == ActiveViewport::VSG && vsgCamera_) {
        vsgCamera_->setViewPreset(preset);
    }
    // VTK 分支：T38/T42 时实现
}

void ViewManager::saveViewState(const std::string& workbenchId) {
    if (activeVp_ == ActiveViewport::VSG && vsgCamera_) {
        auto la = vsgCamera_->lookAt();
        if (la) {
            CameraState state;
            state.eye    = la->eye;
            state.center = la->center;
            state.up     = la->up;
            viewStateCache_[workbenchId] = state;
        }
    }
    // VTK 分支：预留
}

void ViewManager::restoreViewState(const std::string& workbenchId) {
    auto it = viewStateCache_.find(workbenchId);
    if (it == viewStateCache_.end()) return;

    const auto& state = it->second;
    if (activeVp_ == ActiveViewport::VSG && vsgCamera_) {
        auto la = vsgCamera_->lookAt();
        if (la) {
            la->eye    = state.eye;
            la->center = state.center;
            la->up     = state.up;
        }
    }
    // VTK 分支：预留
}

// === 渲染模式 ===

void ViewManager::setRenderMode(RenderMode mode) {
    renderMode_ = mode;
}

ViewManager::RenderMode ViewManager::renderMode() const {
    return renderMode_;
}

// === 可见性控制 ===

void ViewManager::setCategoryVisible(Category cat, bool visible) {
    visibility_[cat] = visible;
}

bool ViewManager::isCategoryVisible(Category cat) const {
    auto it = visibility_.find(cat);
    return it != visibility_.end() ? it->second : true;
}

// === 场景装饰 ===

void ViewManager::setGridVisible(bool visible) {
    if (vsgFurni_) {
        vsgFurni_->setGridVisible(visible);
    }
}

bool ViewManager::isGridVisible() const {
    if (vsgFurni_) {
        return vsgFurni_->isGridVisible();
    }
    return true;
}

void ViewManager::setTriadVisible(bool visible) {
    triadVisible_ = visible;
    // 坐标轴节点的显隐控制预留（SceneFurniture 当前无 Switch 包装）
}

bool ViewManager::isTriadVisible() const {
    return triadVisible_;
}

// === LOD ===

void ViewManager::setLodLevel(LodLevel level) {
    lodLevel_ = level;
}

ViewManager::LodLevel ViewManager::lodLevel() const {
    return lodLevel_;
}

// === StatusBar 数据 ===

void ViewManager::setMouseWorldPos(const gp_Pnt& pos) {
    mouseWorldPos_ = pos;
}

gp_Pnt ViewManager::currentMouseWorldPos() const {
    return mouseWorldPos_;
}

// === 截图 ===

bool ViewManager::captureImage(const std::string& /*path*/) {
    // 预留：T42/T44 时实现离屏渲染截图
    return false;
}

} // namespace visualization
