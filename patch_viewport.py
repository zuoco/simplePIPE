with open("src/vtk-visualization/VtkViewport.cpp", "r") as f:
    content = f.read()

content = content.replace("renderWindow_->SetIsAllocated(true);", "")
content = content.replace("viewport_->window()->resetOpenGLState();", "viewport_->window()->resetOpenGLState();") # It's not in QQuickWindow in Qt6

content = content.replace("""        // Save Qt state
        viewport_->window()->resetOpenGLState();

        renderWindow_->Render();

        viewport_->window()->resetOpenGLState();""", """        renderWindow_->Render();""")

with open("src/vtk-visualization/VtkViewport.cpp", "w") as f:
    f.write(content)
