CXX = g++
CXXFLAGS = -I./include -I./lib -I./imgui -DGLEW_STATIC
LDFLAGS = -lglew32 -lglfw3 -lopengl32 -lglu32 -lcomdlg32

SRC_DIR = src
BUILD_DIR = build
IMGUI_DIR = imgui

# Fichiers sources principaux
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Fichiers sources ImGui
IMGUI_SOURCES = imgui.cpp \
                imgui_draw.cpp \
                imgui_widgets.cpp \
                imgui_tables.cpp \
                imgui_impl_glfw.cpp \
                imgui_impl_opengl3.cpp

IMGUI_OBJECTS = $(addprefix $(BUILD_DIR)/imgui/,$(IMGUI_SOURCES:.cpp=.o))

TARGET = BasicShader.exe

all: check-imgui $(BUILD_DIR) $(TARGET)

check-imgui:
	@if [ ! -d "$(IMGUI_DIR)" ]; then \
		echo "Error: ImGui directory not found. Creating imgui directory structure..."; \
		mkdir -p $(IMGUI_DIR); \
	fi

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/imgui

$(TARGET): $(OBJECTS) $(IMGUI_OBJECTS)
	$(CXX) $^ -o $@ $(LDFLAGS)

# Règle pour les fichiers sources principaux
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Règle pour les fichiers ImGui
$(BUILD_DIR)/imgui/%.o: $(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean check-imgui
clean:
	rm -rf $(BUILD_DIR) $(TARGET)
