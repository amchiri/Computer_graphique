# Compiler settings
CXX = g++
CXXFLAGS = -Wall -I. -Isrc -Isrc/core -Isrc/rendering -Isrc/ui -Isrc/ui/imgui -Isrc/utils -DGLEW_STATIC
LDFLAGS = -lglew32 -lglfw3 -lopengl32 -lglu32 -lcomdlg32

# Source directories
SRC_DIR = src
CORE_DIR = $(SRC_DIR)/core
RENDERING_DIR = $(SRC_DIR)/rendering
UI_DIR = $(SRC_DIR)/ui
UTILS_DIR = $(SRC_DIR)/utils
IMGUI_DIR = $(UI_DIR)/imgui

# Source files by directory
CORE_SOURCES = $(CORE_DIR)/BasicShader.cpp \
		$(CORE_DIR)/Mat4.cpp \
		$(CORE_DIR)/ResourceManager.cpp \
		$(CORE_DIR)/SceneManager.cpp

RENDERING_SOURCES = $(RENDERING_DIR)/GLShader.cpp \
		$(RENDERING_DIR)/Mesh.cpp \
		$(RENDERING_DIR)/Planet.cpp \
		$(RENDERING_DIR)/Skybox.cpp

UI_SOURCES = $(UI_DIR)/UI.cpp \
		$(IMGUI_DIR)/imgui.cpp \
		$(IMGUI_DIR)/imgui_draw.cpp \
		$(IMGUI_DIR)/imgui_widgets.cpp \
		$(IMGUI_DIR)/imgui_tables.cpp \
		$(IMGUI_DIR)/imgui_impl_glfw.cpp \
		$(IMGUI_DIR)/imgui_impl_opengl3.cpp

UTILS_SOURCES = $(UTILS_DIR)/CameraController.cpp \
		$(UTILS_DIR)/tiny_obj_loader.cpp

# Combine all sources
SOURCES = $(CORE_SOURCES) $(RENDERING_SOURCES) $(UI_SOURCES) $(UTILS_SOURCES)

# Object files (putting them in their respective directories)
OBJECTS = $(SOURCES:.cpp=.o)

# Output executable
TARGET = BasicShader.exe

# Default target
all: $(TARGET)

# Linking rule
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compilation rules with proper include paths
$(CORE_DIR)/%.o: $(CORE_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(RENDERING_DIR)/%.o: $(RENDERING_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(UI_DIR)/%.o: $(UI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(IMGUI_DIR)/%.o: $(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(UTILS_DIR)/%.o: $(UTILS_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule with correct paths
clean:
	rm -f $(OBJECTS) $(TARGET)

# Phony targets
.PHONY: all clean

# Dependencies
$(CORE_DIR)/BasicShader.o: $(CORE_DIR)/BasicShader.cpp
$(CORE_DIR)/Mat4.o: $(CORE_DIR)/Mat4.cpp $(CORE_DIR)/Mat4.h
$(CORE_DIR)/ResourceManager.o: $(CORE_DIR)/ResourceManager.cpp $(CORE_DIR)/ResourceManager.h
$(CORE_DIR)/SceneManager.o: $(CORE_DIR)/SceneManager.cpp $(CORE_DIR)/SceneManager.h

$(RENDERING_DIR)/GLShader.o: $(RENDERING_DIR)/GLShader.cpp $(RENDERING_DIR)/GLShader.h
$(RENDERING_DIR)/Mesh.o: $(RENDERING_DIR)/Mesh.cpp $(RENDERING_DIR)/Mesh.h
$(RENDERING_DIR)/Planet.o: $(RENDERING_DIR)/Planet.cpp $(RENDERING_DIR)/Planet.h
$(RENDERING_DIR)/Skybox.o: $(RENDERING_DIR)/Skybox.cpp $(RENDERING_DIR)/Skybox.h

$(UI_DIR)/UI.o: $(UI_DIR)/UI.cpp $(UI_DIR)/UI.h

$(UTILS_DIR)/CameraController.o: $(UTILS_DIR)/CameraController.cpp $(UTILS_DIR)/CameraController.h
$(UTILS_DIR)/tiny_obj_loader.o: $(UTILS_DIR)/tiny_obj_loader.cpp $(UTILS_DIR)/tiny_obj_loader.h