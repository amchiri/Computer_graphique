#include "UI.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "SceneManager.h"
#include <windows.h>
#include <commdlg.h>
#include <filesystem>
#include <GL/glew.h>
#include <algorithm>

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

    // Mettre à jour les objets de la scène courante
    auto& sceneManager = SceneManager::GetInstance();
    Scene* currentScene = sceneManager.GetActiveScene();
    if (currentScene) {
        SetSceneObjects(
            currentScene->GetObjects(),
            currentScene->GetSun(),
            currentScene->GetPlanets()
        );
    }

    ShowMainWindow(fps, cameraPos, cameraDir);
    ShowSceneManagerWindow();
    if (m_ShowNewSceneDialog) {
        ShowNewSceneDialog();
    }

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
        ShowObjectControls();
        ShowShaderSettings();
        ShowSceneControls();
        ImGui::End();
    }
}

void UI::SetLightParameters(float* lightColor, float* lightIntensity) {
    m_GlobalLightColor = lightColor;
    m_GlobalLightIntensity = lightIntensity;
    
    // Copier les valeurs initiales
    if (lightColor) memcpy(m_LightColor, lightColor, 3 * sizeof(float));
    if (lightIntensity) m_LightIntensity = *lightIntensity;
}


void UI::ShowObjectControls() {
    if (!m_SceneObjects || !m_Sun || !m_Planets) return;

    if (ImGui::CollapsingHeader("Solar System Objects")) {
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
                    float orbitRadius = planet.GetOrbitRadius();
                    float rotationSpeed = planet.GetRotationSpeed();
                    float size = planet.GetSize();

                    ImGui::DragFloat("Orbit Radius", &orbitRadius, 0.1f, 1.0f, 200.0f);
                    ImGui::DragFloat("Size", &size, 0.1f, 0.1f, 100.0f);
                    ImGui::DragFloat("Orbital Speed", &rotationSpeed, 0.01f, 0.0f, 2.0f);

                    // Update using setters
                    planet.SetOrbitRadius(orbitRadius);
                    planet.SetSize(size);
                    planet.SetRotationSpeed(rotationSpeed);

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
                    if (planet.GetMesh() == obj) {
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
    if (!m_SceneObjects) return;

    if (ImGui::CollapsingHeader("Shader Settings")) {
        auto& sceneManager = SceneManager::GetInstance();
        Scene* currentScene = sceneManager.GetActiveScene();
        if (!currentScene) return;

        // Contrôles pour chaque objet (soleil, planètes, etc.)
        for (Mesh* obj : *m_SceneObjects) {
            // Créer un label unique pour chaque objet
            std::string label = (obj == m_Sun) ? "Sun" : "Object";
            if (obj != m_Sun) {
                // Chercher si c'est une planète
                auto it = std::find_if(m_Planets->begin(), m_Planets->end(),
                    [obj](const Planet& p) { return p.GetMesh() == obj; });
                if (it != m_Planets->end()) {
                    label = "Planet " + std::to_string(it - m_Planets->begin());
                }
            }

            if (ImGui::TreeNode(label.c_str())) {
                // Sélection du shader
                GLShader* currentShader = obj->getCurrentShader();
                GLShader* basicShader = &currentScene->GetBasicShader();
                GLShader* colorShader = &currentScene->GetColorShader();
                GLShader* envMapShader = &currentScene->GetEnvMapShader();
                
                int current_shader = 0;
                if (currentShader == colorShader) current_shader = 1;
                else if (currentShader == envMapShader) current_shader = 2;

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

                if (shaderChanged) {
                    switch (current_shader) {
                        case 0: obj->setCurrentShader(basicShader); break;
                        case 1: obj->setCurrentShader(colorShader); break;
                        case 2: obj->setCurrentShader(envMapShader); break;
                    }
                }

                // Paramètres du matériau selon le shader
                Material mat = obj->getMaterial();
                bool materialChanged = false;

                if (currentShader == colorShader) {
                    if (ImGui::ColorEdit3("Color", mat.diffuse)) {
                        materialChanged = true;
                    }
                } 
                else if (currentShader == envMapShader) {
                    if (ImGui::ColorEdit3("Reflection Color", mat.specular)) {
                        materialChanged = true;
                    }
                    if (ImGui::SliderFloat("Reflection Strength", &mat.specularStrength, 0.0f, 1.0f)) {
                        materialChanged = true;
                    }
                }
                else {
                    // Paramètres pour le shader basic
                    if (ImGui::ColorEdit3("Diffuse Color", mat.diffuse)) materialChanged = true;
                    if (ImGui::ColorEdit3("Specular Color", mat.specular)) materialChanged = true;
                    if (ImGui::SliderFloat("Shininess", &mat.shininess, 1.0f, 128.0f)) materialChanged = true;
                }

                // Propriété émissive disponible pour tous les shaders
                if (ImGui::Checkbox("Emissive", &mat.isEmissive)) materialChanged = true;
                if (mat.isEmissive) {
                    if (ImGui::ColorEdit3("Light Color", mat.lightColor)) materialChanged = true;
                    if (ImGui::SliderFloat("Light Intensity", &mat.emissiveIntensity, 0.0f, 10.0f)) materialChanged = true;
                }

                if (materialChanged) {
                    obj->setMaterial(mat);
                    if (GLShader* shader = obj->getCurrentShader()) {
                        shader->Use();
                        shader->SetVec3("u_material.diffuseColor", mat.diffuse);
                        shader->SetVec3("u_material.specularColor", mat.specular);
                        shader->SetFloat("u_material.shininess", mat.shininess);
                        shader->SetBool("u_material.isEmissive", mat.isEmissive);
                        shader->SetFloat("u_material.emissiveIntensity", mat.emissiveIntensity);
                        shader->SetVec3("u_material.lightColor", mat.lightColor);
                        shader->SetFloat("u_material.specularStrength", mat.specularStrength);
                    }
                }

                ImGui::TreePop();
            }
        }
    }
}

void UI::SetSceneObjects(const std::vector<Mesh*>& objects, Mesh* sun, const std::vector<Planet>& planets) {
    m_SceneObjects = const_cast<std::vector<Mesh*>*>(&objects);
    m_Sun = sun;
    m_Planets = const_cast<std::vector<Planet>*>(&planets);
}

void UI::SetShaders(GLShader* basic, GLShader* color, GLShader* envmap) {
    m_BasicShader = basic;
    m_ColorShader = color;
    m_EnvMapShader = envmap;
}

void UI::ShowSceneManagerWindow() {
    if (ImGui::Begin("Scene Manager")) {
        auto& sceneManager = SceneManager::GetInstance();
        std::string currentScene = sceneManager.GetActiveScene() ? 
            sceneManager.GetActiveScene()->GetName() : "None";

        if (ImGui::BeginCombo("Current Scene", currentScene.c_str())) {
            auto sceneNames = sceneManager.GetSceneNames();
            for (const auto& name : sceneNames) {
                bool isSelected = (name == currentScene);
                if (ImGui::Selectable(name.c_str(), isSelected)) {
                    sceneManager.SetActiveScene(name);
                }
            }
            ImGui::EndCombo();
        }

        // Boutons de gestion des scènes
        if (ImGui::Button("New Scene")) {
            m_ShowNewSceneDialog = true;
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Delete Scene") && !currentScene.empty()) {
            if (currentScene != "Solar System" && currentScene != "Demo Scene") {
                sceneManager.RemoveScene(currentScene);
            }
        }

        ImGui::Separator();

        // Navigation entre les scènes
        if (ImGui::Button("Previous Scene")) {
            sceneManager.PreviousScene();
        }
        ImGui::SameLine();
        if (ImGui::Button("Next Scene")) {
            sceneManager.NextScene();
        }
    }
    ImGui::End();
}

void UI::ShowNewSceneDialog() {
    if (ImGui::Begin("New Scene", &m_ShowNewSceneDialog)) {
        char tempName[256] = {0};
        if (strlen(m_NewSceneName) < sizeof(tempName)) {
            strcpy(tempName, m_NewSceneName);
        }
        
        if (ImGui::InputText("Scene Name", tempName, sizeof(tempName)-1)) {
            strncpy(m_NewSceneName, tempName, sizeof(m_NewSceneName)-1);
            m_NewSceneName[sizeof(m_NewSceneName)-1] = '\0';
        }
        
        if (ImGui::Button("Create")) {
            if (strlen(m_NewSceneName) > 0) {
                auto& sceneManager = SceneManager::GetInstance();
                // Créer une nouvelle scène vide au lieu d'une copie de DemoScene
                auto newScene = std::make_unique<EmptyScene>(m_NewSceneName);
                if (newScene->Initialize()) {
                    sceneManager.AddScene(std::move(newScene));
                    sceneManager.SetActiveScene(m_NewSceneName);
                }
                
                m_ShowNewSceneDialog = false;
                memset(m_NewSceneName, 0, sizeof(m_NewSceneName));
            }
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Cancel")) {
            m_ShowNewSceneDialog = false;
            memset(m_NewSceneName, 0, sizeof(m_NewSceneName));
        }
    }
    ImGui::End();
}

void UI::ShowSceneControls() {
    auto scene = SceneManager::GetInstance().GetActiveScene();
    if (!scene) return;

    ImGui::Text("Scene Objects");
    
    // Liste des objets dans la scène
    if (ImGui::TreeNode("Objects")) {
        const auto& objects = scene->GetObjects();
        for (size_t i = 0; i < objects.size(); i++) {
            Mesh* obj = objects[i];
            char label[32];
            sprintf(label, "Object %zu", i);
            
            if (ImGui::TreeNode(label)) {
                // Position
                float pos[3];
                memcpy(pos, obj->getPosition(), sizeof(float) * 3);
                if (ImGui::DragFloat3("Position", pos, 0.1f)) {
                    obj->setPosition(pos[0], pos[1], pos[2]);
                }

                // Scale
                float scale[3];
                memcpy(scale, obj->getScale(), sizeof(float) * 3);
                if (ImGui::DragFloat3("Scale", scale, 0.1f)) {
                    obj->setScale(scale[0], scale[1], scale[2]);
                }

                // Rotation (Nouveau)
                static float rotation[3] = {0.0f, 0.0f, 0.0f};
                if (ImGui::DragFloat3("Rotation", rotation, 1.0f)) {
                    obj->setRotation(rotation[0], rotation[1], rotation[2]);
                }

                // Shader selection
                GLShader* currentShader = obj->getCurrentShader();
                GLShader* basicShader = &scene->GetBasicShader();
                GLShader* colorShader = &scene->GetColorShader();
                GLShader* envMapShader = &scene->GetEnvMapShader();
                
                int current_shader = 0;
                if (currentShader == colorShader) current_shader = 1;
                else if (currentShader == envMapShader) current_shader = 2;

                if (ImGui::RadioButton("Basic", current_shader == 0)) {
                    obj->setCurrentShader(basicShader);
                }
                ImGui::SameLine();
                if (ImGui::RadioButton("Color", current_shader == 1)) {
                    obj->setCurrentShader(colorShader);
                }
                ImGui::SameLine();
                if (ImGui::RadioButton("EnvMap", current_shader == 2)) {
                    obj->setCurrentShader(envMapShader);
                }

                // Material properties
                Material mat = obj->getMaterial();
                bool materialChanged = false;

                if (ImGui::ColorEdit3("Diffuse Color", mat.diffuse)) materialChanged = true;
                if (ImGui::ColorEdit3("Specular Color", mat.specular)) materialChanged = true;
                if (ImGui::SliderFloat("Shininess", &mat.shininess, 1.0f, 128.0f)) materialChanged = true;

                if (materialChanged) {
                    obj->setMaterial(mat);
                }

                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
}
