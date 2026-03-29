import re

with open("src/visualization/ViewManager.h", "r") as f:
    content = f.read()

content = content.replace("namespace visualization {", "namespace vtk_vis { class VtkSceneManager; }\n\nnamespace visualization {")
content = content.replace("// VtkSceneManager* vtkScene_ = nullptr;", "vtk_vis::VtkSceneManager* vtkScene_ = nullptr;")
content = content.replace("void setVsgComponents(SceneManager* scene, CameraController* camera, SceneFurniture* furniture);", "void setVsgComponents(SceneManager* scene, CameraController* camera, SceneFurniture* furniture);\n    void setVtkComponents(vtk_vis::VtkSceneManager* vtkScene);")

with open("src/visualization/ViewManager.h", "w") as f:
    f.write(content)

with open("src/visualization/ViewManager.cpp", "r") as f:
    cpp_content = f.read()

cpp_content = cpp_content.replace("#include \"visualization/ViewManager.h\"", "#include \"visualization/ViewManager.h\"\n#include \"vtk-visualization/VtkSceneManager.h\"\n#include <vtkRenderer.h>\n#include <vtkCamera.h>")

cpp_content = cpp_content.replace("void ViewManager::setVsgComponents", "void ViewManager::setVtkComponents(vtk_vis::VtkSceneManager* vtkScene) {\n    vtkScene_ = vtkScene;\n}\n\nvoid ViewManager::setVsgComponents")

cpp_content = cpp_content.replace("""    // VTK 分支：T38/T42 时实现
}

void ViewManager::setViewPreset(ViewPreset preset)""", """    } else if (activeVp_ == ActiveViewport::VTK && vtkScene_ && vtkScene_->renderer()) {
        vtkScene_->renderer()->ResetCamera();
    }
}

void ViewManager::setViewPreset(ViewPreset preset)""")

with open("src/visualization/ViewManager.cpp", "w") as f:
    f.write(cpp_content)

