#include "../include/UI.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "../include/SceneManager.h"
#include <windows.h>
#include <commdlg.h>
#include <filesystem>
#include <GL/glew.h>
#include <algorithm>
#include <iostream>

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
            // Ajouter l'option wireframe
            if (ImGui::Checkbox("Wireframe", &m_WireframeMode)) {
                glPolygonMode(GL_FRONT_AND_BACK, m_WireframeMode ? GL_LINE : GL_FILL);
            }
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
        for (size_t objIndex = 0; objIndex < m_SceneObjects->size(); objIndex++) {
            Mesh* obj = (*m_SceneObjects)[objIndex];
            
            // Créer un label unique pour chaque objet avec un ID unique
            std::string label;
            std::string uniqueID;
            
            if (obj == m_Sun) {
                label = "Sun";
                uniqueID = "sun_shader_settings";
            } else {
                // Chercher si c'est une planète
                auto it = std::find_if(m_Planets->begin(), m_Planets->end(),
                    [obj](const Planet& p) { return p.GetMesh() == obj; });
                if (it != m_Planets->end()) {
                    size_t planetIndex = it - m_Planets->begin();
                    const char* planetNames[] = {"Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus"};
                    if (planetIndex < 7) {
                        label = planetNames[planetIndex];
                    } else {
                        label = "Planet " + std::to_string(planetIndex);
                    }
                    uniqueID = "planet_" + std::to_string(planetIndex) + "_shader_settings";
                } else {
                    // Objet personnalisé
                    label = "Custom Object " + std::to_string(objIndex);
                    uniqueID = "custom_obj_" + std::to_string(objIndex) + "_shader_settings";
                }
            }

            // Utiliser un ID unique pour éviter les conflits
            ImGui::PushID(uniqueID.c_str());
            
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
                    // Gestion automatique de l'activation/désactivation de texture selon le shader
                    Material mat = obj->getMaterial();
                    
                    switch (current_shader) {
                        case 0: // Basic shader
                            obj->setCurrentShader(basicShader);
                            obj->bindTexture();
                            break;
                        case 1: // Color shader
                            obj->setCurrentShader(colorShader);
                            obj->unbindTexture();
                            break;
                        case 2: // EnvMap shader
                            obj->setCurrentShader(envMapShader);
                            obj->unbindTexture();
                            break;
                    }
                    currentShader = obj->getCurrentShader();
                }

                ImGui::Separator();

                // Paramètres spécifiques selon le shader sélectionné
                Material mat = obj->getMaterial();
                bool materialChanged = false;

                if (currentShader == colorShader) {
                    // SHADER COLOR - Paramètres simples
                    ImGui::Text("Color Shader Parameters");
                    if (ImGui::ColorEdit3("Object Color", mat.diffuse)) {
                        materialChanged = true;
                    }
                    // Propriété émissive pour le shader Color
                    if (ImGui::Checkbox("Emissive", &mat.isEmissive)) materialChanged = true;
                    if (mat.isEmissive) {
                        if (ImGui::ColorEdit3("Emissive Color", mat.lightColor)) materialChanged = true;
                        if (ImGui::SliderFloat("Emissive Intensity", &mat.emissiveIntensity, 0.0f, 10.0f)) materialChanged = true;
                    }
                } 
                else if (currentShader == envMapShader) {
                    // SHADER ENVMAP - Paramètres de réflexion et cubemap
                    ImGui::Text("Environment Mapping Parameters");
                    
                    if (ImGui::ColorEdit3("Base Color", mat.diffuse)) {
                        materialChanged = true;
                    }
                    
                    // Option pour afficher la texture de l'objet dans le shader EnvMap
                    bool hasTexture = (mat.diffuseMap != 0);
                    if (hasTexture) {
                        if (ImGui::Checkbox("Use Object Texture", &mat.useTextureInEnvMapShader)) {
                            materialChanged = true;
                        }
                        if (mat.useTextureInEnvMapShader) {
                            ImGui::Text("Texture will be mixed with base color before reflection");
                        } else {
                            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Object texture ignored - using base color only");
                        }
                    } else {
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No texture available");
                    }
                    
                    if (ImGui::SliderFloat("Reflection Strength", &mat.specularStrength, 0.0f, 2.0f)) {
                        materialChanged = true;
                    }
                    
                    if (ImGui::ColorEdit3("Specular Tint", mat.specular)) {
                        materialChanged = true;
                    }
                    
                    if (ImGui::SliderFloat("Surface Roughness", &mat.shininess, 1.0f, 128.0f)) {
                        materialChanged = true;
                    }
                    
                    ImGui::Separator();
                    ImGui::Text("CubeMap Controls");
                    
                    // Boutons pour gérer le cubemap
                    if (ImGui::Button("Reload CubeMap")) {
                        currentScene->GetCubeMap().Reload();
                    }
                    
                    ImGui::SameLine();
                    
                    if (ImGui::Button("Load CubeMap Files")) {
                        // Ouvrir une boîte de dialogue pour sélectionner les 6 fichiers
                        OPENFILENAMEA ofn;
                        char szFile[256] = { 0 };
                        
                        ZeroMemory(&ofn, sizeof(ofn));
                        ofn.lStructSize = sizeof(ofn);
                        ofn.hwndOwner = glfwGetWin32Window(m_Window);
                        ofn.lpstrFile = szFile;
                        ofn.nMaxFile = sizeof(szFile);
                        ofn.lpstrFilter = "Image Files\0*.png;*.jpg;*.jpeg;*.bmp\0All Files\0*.*\0";
                        ofn.nFilterIndex = 1;
                        ofn.lpstrFileTitle = NULL;
                        ofn.nMaxFileTitle = 0;
                        ofn.lpstrInitialDir = NULL;
                        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                        ofn.lpstrTitle = "Select CubeMap +X face";

                        std::vector<std::string> faces(6);
                        const char* faceNames[] = {"+X (Right)", "-X (Left)", "+Y (Top)", "-Y (Bottom)", "+Z (Back)", "-Z (Front)"};
                        
                        bool allSelected = true;
                        for (int i = 0; i < 6; i++) {
                            ofn.lpstrTitle = faceNames[i];
                            if (GetOpenFileNameA(&ofn)) {
                                faces[i] = szFile;
                            } else {
                                allSelected = false;
                                break;
                            }
                        }
                        
                        if (allSelected) {
                            if (currentScene->GetCubeMap().LoadFromImages(faces)) {
                                std::cout << "CubeMap loaded successfully!" << std::endl;
                            } else {
                                std::cerr << "Failed to load CubeMap!" << std::endl;
                            }
                        }
                    }
                    
                    // Afficher l'état du cubemap
                    if (currentScene->GetCubeMap().IsLoaded()) {
                        ImGui::TextColored(ImVec4(0, 1, 0, 1), "CubeMap: Loaded");
                    } else {
                        ImGui::TextColored(ImVec4(1, 0, 0, 1), "CubeMap: Not Loaded");
                    }
                }
                else {
                    // SHADER BASIC - Paramètres complets d'éclairage
                    ImGui::Text("Basic Shader Parameters");
                    
                    // Couleurs du matériau
                    if (ImGui::ColorEdit3("Diffuse Color", mat.diffuse)) materialChanged = true;
                    
                    // Option pour afficher la texture de l'objet dans le shader Basic
                    bool hasTexture = (mat.diffuseMap != 0);
                    if (hasTexture) {
                        if (ImGui::Checkbox("Use Object Texture", &mat.useTextureInBasicShader)) {
                            materialChanged = true;
                        }
                        if (mat.useTextureInBasicShader) {
                            ImGui::Text("Texture will be applied to the material");
                        } else {
                            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Object texture ignored - using diffuse color only");
                        }
                    } else {
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No texture available");
                    }
                    
                    // Modèle d'illumination
                    const char* illuminationModels[] = {"Lambert", "Phong", "Blinn-Phong"};
                    int currentModel = static_cast<int>(mat.illuminationModel);
                    if (ImGui::Combo("Illumination Model", &currentModel, illuminationModels, 3)) {
                        mat.illuminationModel = static_cast<Material::IlluminationModel>(currentModel);
                        materialChanged = true;
                    }

                    // Paramètres spéculaires (seulement pour Phong et Blinn-Phong)
                    if (currentModel > 0) {
                        if (ImGui::ColorEdit3("Specular Color", mat.specular)) materialChanged = true;
                        if (ImGui::SliderFloat("Specular Strength", &mat.specularStrength, 0.0f, 1.0f)) materialChanged = true;
                        if (ImGui::SliderFloat("Shininess", &mat.shininess, 1.0f, 128.0f)) materialChanged = true;
                    }
                    
                    // Propriétés émissives
                    if (ImGui::Checkbox("Emissive", &mat.isEmissive)) materialChanged = true;
                    if (mat.isEmissive) {
                        if (ImGui::ColorEdit3("Emissive Color", mat.lightColor)) materialChanged = true;
                        if (ImGui::SliderFloat("Emissive Intensity", &mat.emissiveIntensity, 0.0f, 10.0f)) materialChanged = true;
                    }
                }

                // Appliquer les changements du matériau
                if (materialChanged) {
                    obj->setMaterial(mat);
                }

                ImGui::TreePop();
            }
            
            ImGui::PopID(); // Important : libérer l'ID unique
        }
    }
}

void UI::SetSceneObjects(const std::vector<Mesh*>& objects, Mesh* sun, const std::vector<Planet>& planets) {
    // Reset les pointeurs avant d'assigner les nouveaux
    m_SceneObjects = nullptr;
    m_Sun = nullptr;
    m_Planets = nullptr;
    m_SelectedObject = -1;

    // Assigner les nouveaux pointeurs
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
                std::cout << "=== Creating new scene ===" << std::endl;
                std::cout << "Scene name: " << m_NewSceneName << std::endl;
                
                auto& sceneManager = SceneManager::GetInstance();
                
                // Reset l'UI avant la création de la nouvelle scène
                SetSceneObjects({}, nullptr, {});
                
                std::cout << "Creating EmptyScene..." << std::endl;
                auto newScene = std::make_unique<EmptyScene>(m_NewSceneName);
                
                std::cout << "Initializing scene..." << std::endl;
                bool initSuccess = newScene->Initialize();
                std::cout << "Init result: " << (initSuccess ? "success" : "failed") << std::endl;
                
                if (initSuccess) {
                    std::cout << "Adding scene to manager..." << std::endl;
                    sceneManager.AddScene(std::move(newScene));
                    std::cout << "Setting as active scene..." << std::endl;
                    if (sceneManager.SetActiveScene(m_NewSceneName)) {
                        std::cout << "Scene creation successful" << std::endl;
                    } else {
                        std::cerr << "Failed to set active scene!" << std::endl;
                    }
                } else {
                    std::cerr << "Failed to initialize new scene!" << std::endl;
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

    if (ImGui::CollapsingHeader("Scene Objects")) {
        if (ImGui::Button("Load 3D Model")) {
            m_ShowLoadModelDialog = true;
        }

        if (ImGui::TreeNode("Custom Objects Only")) {
            const auto& objects = scene->GetObjects();
            size_t customObjectCounter = 0;
            
            for (size_t i = 0; i < objects.size(); i++) {
                Mesh* obj = objects[i];
                
                // IGNORER le soleil et les planètes - ne montrer que les objets personnalisés
                bool isSystemObject = false;
                
                // Vérifier si c'est le soleil
                if (obj == m_Sun) {
                    isSystemObject = true;
                }
                
                // Vérifier si c'est une planète
                if (!isSystemObject && m_Planets) {
                    for (const auto& planet : *m_Planets) {
                        if (planet.GetMesh() == obj) {
                            isSystemObject = true;
                            break;
                        }
                    }
                }
                
                // Ne traiter que les objets personnalisés (non-système)
                if (!isSystemObject) {
                    std::string objectLabel = "Custom Object " + std::to_string(customObjectCounter);
                    std::string uniqueID = "custom_scene_obj_" + std::to_string(i);
                    
                    ImGui::PushID(uniqueID.c_str());
                    
                    if (ImGui::TreeNode(objectLabel.c_str())) {
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

                        // Rotation
                        static float rotation[3] = {0.0f, 0.0f, 0.0f};
                        if (ImGui::DragFloat3("Rotation", rotation, 1.0f)) {
                            obj->setRotation(rotation[0], rotation[1], rotation[2]);
                        }

                        // Bouton Delete (toujours disponible pour les objets personnalisés)
                        if (ImGui::Button("Delete Object")) {
                            scene->RemoveObject(obj);
                            ImGui::TreePop();
                            ImGui::PopID();
                            break;  // Sortir de la boucle car l'objet a été supprimé
                        }

                        // Shader selection avec ID unique
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
                    
                    ImGui::PopID();
                    customObjectCounter++;
                }
            }
            
            if (customObjectCounter == 0) {
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No custom objects in scene");
                ImGui::Text("Use 'Load 3D Model' to add objects");
            }
            
            ImGui::TreePop();
        }
    }
    
    if (m_ShowLoadModelDialog) {
        ShowLoadModelDialog();
    }
}

void UI::ShowLoadModelDialog() {
    if (ImGui::Begin("Load 3D Model", &m_ShowLoadModelDialog)) {
        static char filepath[256] = "";
        ImGui::InputText("Model Path", filepath, sizeof(filepath));

        if (ImGui::Button("Browse...")) {
            OPENFILENAMEA ofn;
            char szFile[256] = { 0 };
            
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = glfwGetWin32Window(m_Window);
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = "OBJ Files\0*.obj\0All Files\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (GetOpenFileNameA(&ofn)) {
                strncpy(filepath, szFile, sizeof(filepath) - 1);
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Load") && filepath[0] != '\0') {
            auto& sceneManager = SceneManager::GetInstance();
            Scene* currentScene = sceneManager.GetActiveScene();
            
            if (currentScene) {
                Mesh* newMesh = new Mesh();
                if (newMesh->loadFromOBJFile(filepath)) {
                    newMesh->setPosition(0.0f, 0.0f, -5.0f);
                    newMesh->setScale(1.0f, 1.0f, 1.0f);
                    newMesh->setCurrentShader(&(currentScene->GetBasicShader()));
                    currentScene->AddObject(newMesh);
                    m_ShowLoadModelDialog = false;
                    memset(filepath, 0, sizeof(filepath));
                } else {
                    delete newMesh;
                }
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            m_ShowLoadModelDialog = false;
        }

        ImGui::End();
    }
}
