#include "vtk-visualization/VtkSceneManager.h"

#include <vtkActor.h>
#include <vtkDataSet.h>
#include <vtkMapper.h>
#include <vtkPolyData.h>
#include <vtkRenderer.h>

namespace vtk_vis {

VtkSceneManager::VtkSceneManager()
    : renderer_{vtkSmartPointer<vtkRenderer>::New()}
{
}

void VtkSceneManager::addActor(const std::string& uuid, vtkSmartPointer<vtkActor> actor) {
    if (uuid.empty() || !actor) {
        return;
    }

    auto existingIt = actors_.find(uuid);
    if (existingIt != actors_.end()) {
        renderer_->RemoveActor(existingIt->second.actor);
    }

    ActorEntry entry;
    entry.actor = actor;
    entry.kind = detectActorKind(actor);
    applyVisibility(entry);

    renderer_->AddActor(actor);
    actors_[uuid] = entry;
}

void VtkSceneManager::removeActor(const std::string& uuid) {
    auto it = actors_.find(uuid);
    if (it == actors_.end()) {
        return;
    }

    renderer_->RemoveActor(it->second.actor);
    actors_.erase(it);
}

void VtkSceneManager::updateActor(const std::string& uuid, vtkSmartPointer<vtkActor> actor) {
    addActor(uuid, actor);
}

void VtkSceneManager::setRenderMode(RenderMode mode) {
    mode_ = mode;
    for (auto& [uuid, entry] : actors_) {
        (void)uuid;
        applyVisibility(entry);
    }
}

VtkSceneManager::RenderMode VtkSceneManager::renderMode() const {
    return mode_;
}

vtkSmartPointer<vtkRenderer> VtkSceneManager::renderer() const {
    return renderer_;
}

bool VtkSceneManager::hasActor(const std::string& uuid) const {
    return actors_.count(uuid) > 0;
}

size_t VtkSceneManager::actorCount() const {
    return actors_.size();
}

VtkSceneManager::ActorKind VtkSceneManager::detectActorKind(vtkActor* actor) {
    if (!actor) {
        return ActorKind::Unknown;
    }

    vtkMapper* mapper = actor->GetMapper();
    if (!mapper) {
        return ActorKind::Unknown;
    }

    vtkDataSet* dataSet = mapper->GetInput();
    if (!dataSet) {
        mapper->Update();
        dataSet = mapper->GetInput();
    }

    vtkPolyData* polyData = vtkPolyData::SafeDownCast(dataSet);
    if (!polyData) {
        return ActorKind::Unknown;
    }

    if (polyData->GetNumberOfPolys() > 0) {
        return ActorKind::Solid;
    }

    if (polyData->GetNumberOfLines() > 0) {
        return ActorKind::Beam;
    }

    return ActorKind::Unknown;
}

void VtkSceneManager::applyVisibility(ActorEntry& entry) {
    if (!entry.actor) {
        return;
    }

    bool visible = true;
    if (entry.kind == ActorKind::Solid) {
        visible = (mode_ == RenderMode::Solid);
    } else if (entry.kind == ActorKind::Beam) {
        visible = (mode_ == RenderMode::Beam);
    }

    entry.actor->SetVisibility(visible ? 1 : 0);
}

} // namespace vtk_vis