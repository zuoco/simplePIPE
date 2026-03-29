import re

with open("src/main.cpp", "r") as f:
    content = f.read()

content = "#include \"vtk-visualization/VtkViewport.h\"\n#include \"vtk-visualization/VtkSceneManager.h\"\n" + content

content = content.replace("qmlRegisterType<ui::VsgQuickItem>(\"PipeCAD\", 1, 0, \"VsgViewport\");", "qmlRegisterType<ui::VsgQuickItem>(\"PipeCAD\", 1, 0, \"VsgViewport\");\n    qmlRegisterType<vtk_vis::VtkViewport>(\"PipeCAD\", 1, 0, \"VtkViewportElement\");")

content = content.replace("visualization::ViewManager viewManager;", "visualization::ViewManager viewManager;\n    vtk_vis::VtkSceneManager vtkScene;\n    viewManager.setVtkComponents(&vtkScene);")

viewport_slot = """auto* viewportItem = qobject_cast<ui::VsgQuickItem*>(obj);
                         if (!viewportItem) {
                             return;
                         }
                         viewportItem->setSceneManager(&sceneManager);
                         viewportItem->setCameraController(&cameraController);
                         viewportItem->setSceneFurniture(&sceneFurniture);"""

new_viewport_slot = """auto* vsgItem = qobject_cast<ui::VsgQuickItem*>(obj);
                         if (vsgItem) {
                             vsgItem->setSceneManager(&sceneManager);
                             vsgItem->setCameraController(&cameraController);
                             vsgItem->setSceneFurniture(&sceneFurniture);
                         } else {
                             auto* vtkItem = qobject_cast<vtk_vis::VtkViewport*>(obj);
                             if (vtkItem) {
                                 vtkItem->setSceneManager(&vtkScene);
                             }
                         }"""

content = content.replace(viewport_slot, new_viewport_slot)

with open("src/main.cpp", "w") as f:
    f.write(content)

