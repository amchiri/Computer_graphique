#include "UI.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <windows.h>
#include <commdlg.h>
#include <filesystem>
#include <GL/glew.h>

UI::UI(GLFWwindow* window, int width, int height) 
    : m_Window(window)
    , m_Width(width)
    , m_Height(height)
    , m_ShowDebugWindow(true)
    , m_ShowSettings(true)
    , m_WireframeMode(false)
    , m_SelectedObject(-1)
    , m_SceneObjects(nullptr)
    , m_Sun(nullptr)
    , m_Planets(nullptr)
    , m_BasicShader(nullptr)
    , m_ColorShader(nullptr)
    , m_EnvMapShader(nullptr)
{
    m_LightColor[0] = m_LightColor[1] = m_LightColor[2] = 1.0f;
    m_LightIntensity = 1.0f;
}

UI::~UI() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

bool UI::Initialize() {
    // Only initialize if not already initialized
    ImGuiContext* context = ImGui::GetCurrentContext();
    if (!context) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.FontGlobalScale = 1.5f;

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowPadding = ImVec2(15, 15);
        style.FramePadding = ImVec2(5, 5);
        style.ItemSpacing = ImVec2(12, 8);
        style.WindowRounding = 12.0f;
        style.FrameRounding = 5.0f;
        style.ScrollbarSize = 18;
        style.GrabMinSize = 20;

        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
        ImGui_ImplOpenGL3_Init("#version 130");
    }
    
    return true;
}

void UI::RenderUI(float fps, const float* cameraPos, const float* cameraDir) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ShowMainWindow(fps, cameraPos, cameraDir);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UI::ShowMainWindow(float fps, const float* cameraPos, const float* cameraDir) {
    if (m_ShowDebugWindow) {
        ImGui::Begin("Debug Info");
        if (ImGui::CollapsingHeader("Camera")) {
            ImGui::Text("Position: (%.2f, %.2f, %.2f)", cameraPos[0], cameraPos[1], cameraPos[2]);
            ImGui::Text("Direction: (%.2f, %.2f, %.2f)", cameraDir[0], cameraDir[1], cameraDir[2]);
        }
        ImGui::Text("FPS: %.1f", fps);
        ShowLightSettings();
        ShowObjectControls();
        ShowShaderSettings();
        ImGui::End();
    }
}

void UI::ShowLightSettings() {
    if (!m_ShowSettings || !m_Sun) return;

    ImGui::ColorEdit3("Light Color", m_LightColor);
    ImGui::SliderFloat("Light Intensity", &m_LightIntensity, 0.0f, 10.0f);

    float lightPos[3];
    memcpy(lightPos, m_Sun->getPosition(), 3 * sizeof(float));
    if (ImGui::SliderFloat3("Light Position", lightPos, -50.0f, 50.0f)) {
        m_Sun->setPosition(lightPos[0], lightPos[1], lightPos[2]);
    }
}

void UI::ShowObjectControls() {
    if (!m_SceneObjects || !m_Sun || !m_Planets) return;

    if (ImGui::CollapsingHeader("Scene Objects")) {
        // Contrôles du Soleil
        if (ImGui::TreeNode("Sun")) {
            float pos[3] = {m_Sun->getPosition()[0], m_Sun->getPosition()[1], m_Sun->getPosition()[2]};
            float scale[3] = {m_Sun->getScale()[0], m_Sun->getScale()[1], m_Sun->getScale()[2]};

            if (ImGui::DragFloat3("Position", pos, 0.1f)) {
                m_Sun->setPosition(pos[0], pos[1], pos[2]);
            }
            if (ImGui::DragFloat3("Scale", scale, 0.1f, 0.1f, 100.0f)) {
                m_Sun->setScale(scale[0], scale[1], scale[2]);
            }
            ImGui::TreePop();
        }

        // Contrôles des planètes
        if (ImGui::TreeNode("Planets")) {
            for (size_t i = 0; i < m_Planets->size(); i++) {
                char label[32];
                sprintf(label, "Planet %zu", i);
                if (ImGui::TreeNode(label)) {
                    Planet& planet = (*m_Planets)[i];
                    float orbitRadius = planet.orbitRadius;
                    float rotationSpeed = planet.rotationSpeed;
                    float size = planet.size;

                    ImGui::DragFloat("Orbit Radius", &orbitRadius, 0.1f, 1.0f, 200.0f);
                    ImGui::DragFloat("Size", &size, 0.1f, 0.1f, 100.0f);
                    ImGui::DragFloat("Orbital Speed", &rotationSpeed, 0.01f, 0.0f, 2.0f);

                    planet.orbitRadius = orbitRadius;
                    planet.size = size;
                    planet.rotationSpeed = rotationSpeed;

                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }

        // Objets personnalisés
        if (ImGui::TreeNode("Custom Objects")) {
            for (size_t i = 0; i < m_SceneObjects->size(); i++) {
                Mesh* obj = (*m_SceneObjects)[i];
                if (obj == m_Sun) continue;
                
                bool isPlanet = false;
                for (const auto& planet : *m_Planets) {
                    if (planet.mesh == obj) {
                        isPlanet = true;
                        break;
                    }
                }
                if (isPlanet) continue;

                char label[32];
                sprintf(label, "Object %zu", i);
                if (ImGui::TreeNode(label)) {
                    float pos[3] = {obj->getPosition()[0], obj->getPosition()[1], obj->getPosition()[2]};
                    float scale[3] = {obj->getScale()[0], obj->getScale()[1], obj->getScale()[2]};

                    if (ImGui::DragFloat3("Position", pos, 0.1f)) {
                        obj->setPosition(pos[0], pos[1], pos[2]);
                    }
                    if (ImGui::DragFloat3("Scale", scale, 0.1f, 0.1f, 100.0f)) {
                        obj->setScale(scale[0], scale[1], scale[2]);
                    }

                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
    }
}

void UI::ShowShaderSettings() {
    if (!m_SceneObjects || !m_BasicShader || !m_ColorShader || !m_EnvMapShader) return;

    if (ImGui::CollapsingHeader("Shader Settings")) {
        // Liste des objets
        for (size_t i = 0; i < m_SceneObjects->size(); i++) {
            Mesh* obj = (*m_SceneObjects)[i];
            char label[32];
            sprintf(label, "Object %zu", i);
            
            // Créer un arbre pour chaque objet
            if (ImGui::TreeNode(label)) {
                // Détecter le shader actuel
                GLShader* currentShader = obj->getCurrentShader();
                int current_shader = 0;
                
                if (currentShader == m_ColorShader) current_shader = 1;
                else if (currentShader == m_EnvMapShader) current_shader = 2;

                bool shaderChanged = false;
                
                if (ImGui::RadioButton("Basic", current_shader == 0)) {
                    current_shader = 0;
                    shaderChanged = true;
                }
                ImGui::SameLine();
                
                if (ImGui::RadioButton("Color", current_shader == 1)) {
                    current_shader = 1;
                    shaderChanged = true;
                }
                ImGui::SameLine();
                
                if (ImGui::RadioButton("EnvMap", current_shader == 2)) {
                    current_shader = 2;
                    shaderChanged = true;
                }

                // Mettre à jour le shader si changé
                if (shaderChanged) {
                    switch (current_shader) {
                        case 0: obj->setCurrentShader(m_BasicShader); break;
                        case 1: obj->setCurrentShader(m_ColorShader); break;
                        case 2: obj->setCurrentShader(m_EnvMapShader); break;
                    }
                }

                // Options spécifiques au shader
                if (current_shader == 1) { // Color shader
                    Material mat = obj->getMaterial();
                    if (ImGui::ColorEdit3("Color", mat.diffuse)) {
                        obj->setMaterial(mat);
                    }
                }

                ImGui::TreePop();
            }
        }
    }
}

void UI::SetSceneObjects(std::vector<Mesh*>& objects, Mesh* sun, std::vector<Planet>& planets) {
    m_SceneObjects = &objects;
    m_Sun = sun;
    m_Planets = &planets;
}

void UI::SetShaders(GLShader* basic, GLShader* color, GLShader* envmap) {
    m_BasicShader = basic;
    m_ColorShader = color;
    m_EnvMapShader = envmap;
}
