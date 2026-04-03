// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <vtkSmartPointer.h>

#include <cstddef>
#include <string>
#include <unordered_map>

class vtkActor;
class vtkRenderer;

namespace vtk_vis {

/// 管理 Analysis 视口中的 VTK Actor，并支持 Solid/Beam 渲染模式切换
class VtkSceneManager {
public:
    enum class RenderMode {
        Solid,
        Beam,
    };

    VtkSceneManager();

    void addActor(const std::string& uuid, vtkSmartPointer<vtkActor> actor);
    void removeActor(const std::string& uuid);
    void updateActor(const std::string& uuid, vtkSmartPointer<vtkActor> actor);

    /// 通过可见性切换 Solid/Beam Actor，不重建场景对象
    void setRenderMode(RenderMode mode);
    RenderMode renderMode() const;

    vtkSmartPointer<vtkRenderer> renderer() const;

    // 供测试与上层查询
    bool hasActor(const std::string& uuid) const;
    size_t actorCount() const;

private:
    enum class ActorKind {
        Unknown,
        Solid,
        Beam,
    };

    struct ActorEntry {
        vtkSmartPointer<vtkActor> actor;
        ActorKind kind = ActorKind::Unknown;
    };

    static ActorKind detectActorKind(vtkActor* actor);
    void applyVisibility(ActorEntry& entry);

    vtkSmartPointer<vtkRenderer> renderer_;
    std::unordered_map<std::string, ActorEntry> actors_;
    RenderMode mode_ = RenderMode::Solid;
};

} // namespace vtk_vis