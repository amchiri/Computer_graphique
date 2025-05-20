```markdown
## Compilation Instructions

To build the project, run the following command in your terminal:

```sh
g++ BasicShader.cpp GLShader.cpp MatrixUtils.cpp Mesh.cpp tiny_obj_loader.cpp \
    imgui/imgui.cpp \
    imgui/imgui_draw.cpp \
    imgui/imgui_widgets.cpp \
    imgui/imgui_tables.cpp \
    imgui/imgui_impl_glfw.cpp \
    imgui/imgui_impl_opengl3.cpp \
    -I. -Iimgui -lglew32 -lglfw3 -lopengl32 -lglu32 -o BasicShader.exe
```

Make sure you have all dependencies installed and the include/library paths are correct.
```

