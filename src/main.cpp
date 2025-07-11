#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <memory>

#include "../include/GLShader.h"
#include "../include/Mesh.h"
#include "../include/Mat4.h"
#include "../include/UI.h"
#include "../include/Skybox.h"
#include "../include/CameraController.h"
#include "../include/Planet.h"
#include "../include/ResourceManager.h"
#include "../include/SceneManager.h"
#include "../include/UBOManager.h"

// Variables globales principales
std::unique_ptr<UI> g_UI;
std::unique_ptr<Skybox> g_Skybox;
std::unique_ptr<CameraController> g_Camera;
SceneManager* g_SceneManager = nullptr;
GLFWwindow* g_Window = nullptr;

// Paramètres de rendu
int width = 1200, height = 900;
const float FOV = 60.0f * (3.14159f / 180.0f);
const float CAM_FAR = 1000.0f;
const float CAM_NEAR = 0.1f;

// Variables de temps
float fps = 0.0f;
float elapsed_time = 0.0f;
float last_frame_time = 0.0f;

void processInput(GLFWwindow* window) {
    if (g_Camera) {
        g_Camera->Update(elapsed_time);
    }

    // Gestion des touches pour changement de scène
    static bool key_states[4] = {false}; // 1, 2, N, P
    
    struct KeyMapping {
        int key;
        int index;
        const char* scene_name;
        void (SceneManager::*action)();
    };
    
    KeyMapping mappings[] = {
        {GLFW_KEY_1, 0, "Solar System", nullptr},
        {GLFW_KEY_2, 1, "Demo Scene", nullptr},
        {GLFW_KEY_N, 2, nullptr, &SceneManager::NextScene},
        {GLFW_KEY_P, 3, nullptr, &SceneManager::PreviousScene}
    };
    
    for (auto& mapping : mappings) {
        bool pressed = glfwGetKey(window, mapping.key) == GLFW_PRESS;
        
        if (pressed && !key_states[mapping.index] && g_SceneManager) {
            if (mapping.scene_name) {
                g_SceneManager->SetActiveScene(mapping.scene_name);
                std::cout << "Switched to " << mapping.scene_name << " scene" << std::endl;
            } else if (mapping.action) {
                (g_SceneManager->*mapping.action)();
                std::cout << "Scene changed" << std::endl;
            }
        }
        key_states[mapping.index] = pressed;
    }
}

void Render() {
    // Calcul du FPS et temps écoulé
    float current_time = glfwGetTime();
    elapsed_time = current_time - last_frame_time;
    last_frame_time = current_time;
    fps = 1.0f / elapsed_time;

    // Configuration OpenGL
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // Couleur de fond temporaire pour debug
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Matrices de transformation
    Mat4 projectionMatrix = Mat4::perspective(FOV, (float)width / height, CAM_NEAR, CAM_FAR);
    Mat4 viewMatrix;
    
    if (g_Camera) {
        const float* camPos = g_Camera->GetPosition();
        const float* camFront = g_Camera->GetFront();
        const float* camUp = g_Camera->GetUp();

        float camTarget[3] = {
            camPos[0] + camFront[0],
            camPos[1] + camFront[1],
            camPos[2] + camFront[2]
        };
        viewMatrix = Mat4::lookAt(camPos, camTarget, camUp);
    }
    
    // Mettre à jour les UBOs avec les nouvelles matrices
    UBOManager::Get().UpdateProjectionView(projectionMatrix.data(), viewMatrix.data());

    // Rendu de la skybox en premier (avec états spéciaux)
    if (g_Skybox) {
        g_Skybox->Draw(viewMatrix, projectionMatrix);
    }

    // Rendu du reste de la scène
    if (g_SceneManager) {
        g_SceneManager->Update(elapsed_time);
        g_SceneManager->Render(projectionMatrix, viewMatrix);
    }

    // Interface utilisateur
    Scene* activeScene = g_SceneManager ? g_SceneManager->GetActiveScene() : nullptr;
    if (activeScene && g_UI) {
        g_UI->SetSceneObjects(
            activeScene->GetObjects(), 
            activeScene->GetSun(), 
            activeScene->GetPlanets()
        );
        g_UI->RenderUI(fps, g_Camera->GetPosition(), g_Camera->GetFront());
    }
}

bool initializeOpenGL() {
    // Configuration de la fenêtre GLFW
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    g_Window = glfwCreateWindow(width, height, "OpenGL Scene Manager", nullptr, nullptr);
    if (!g_Window) {
        std::cerr << "Erreur : Impossible de créer la fenêtre GLFW" << std::endl;
        return false;
    }
    glfwMakeContextCurrent(g_Window);

    // Callback de redimensionnement
    glfwSetFramebufferSizeCallback(g_Window, [](GLFWwindow*, int w, int h) {
        width = w;
        height = h;
        glViewport(0, 0, width, height);
    });

    // Initialisation de GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Erreur : Impossible d'initialiser GLEW" << std::endl;
        return false;
    }

    return true;
}

bool initializeCamera() {
    g_Camera = std::make_unique<CameraController>(g_Window);
    if (!g_Camera) {
        std::cerr << "Failed to create camera controller" << std::endl;
        return false;
    }

    g_Camera->SetPosition(0.0f, 50.0f, 100.0f);
    g_Camera->SetMovementSpeed(0.05f);
    g_Camera->SetSensitivity(0.1f);
    g_Camera->SetRotation(-30.0f, -90.0f);
    g_Camera->Initialize();

    return true;
}

bool initializeShaders() {
    auto& rm = ResourceManager::Get();
    
    const char* shaders[][3] = {
        {"basic", "assets/shaders/Basic.vs", "assets/shaders/Basic.fs"},
        {"color", "assets/shaders/Color.vs", "assets/shaders/Color.fs"},
        {"envmap", "assets/shaders/EnvMap.vs", "assets/shaders/EnvMap.fs"}
    };
    
    for (auto& shader : shaders) {
        if (!rm.LoadShader(shader[0], shader[1], shader[2])) {
            std::cerr << "Failed to load shader: " << shader[0] << std::endl;
            return false;
        }
    }

    return true;
}

bool initializeSkybox() {
    g_Skybox = std::make_unique<Skybox>();
    if (!g_Skybox->Initialize("assets/textures/space.png")) {
        std::cerr << "Failed to initialize skybox" << std::endl;
        return false;
    }
    return true;
}

bool initializeScenes() {
    g_SceneManager = &SceneManager::GetInstance();
    
    // Ajout des scènes
    g_SceneManager->AddScene(std::make_unique<SolarSystemScene>());
    g_SceneManager->AddScene(std::make_unique<DemoScene>());
    
    if (!g_SceneManager->Initialize()) {
        std::cerr << "Failed to initialize scene manager" << std::endl;
        return false;
    }
    
    g_SceneManager->SetActiveScene("Solar System");
    return true;
}

bool initializeUI() {
    auto& rm = ResourceManager::Get();
    
    g_UI = std::make_unique<UI>(g_Window, width, height);
    g_UI->Initialize();
    g_UI->SetShaders(
        rm.GetShader("basic"), 
        rm.GetShader("color"), 
        rm.GetShader("envmap")
    );
    
    // Configuration de l'éclairage par défaut
    float light_color[3] = {1.0f, 1.0f, 1.0f};
    float light_intensity = 1.0f;
    g_UI->SetLightParameters(light_color, &light_intensity);

    return true;
}

bool Initialize() {
    if (!initializeOpenGL()) {
        return false;
    }

    // Configuration OpenGL globale
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialiser l'UBO Manager immédiatement après OpenGL
    UBOManager::Get().Initialize();

    if (!initializeCamera() || 
        !initializeShaders() || 
        !initializeSkybox() || 
        !initializeScenes() || 
        !initializeUI()) {
        return false;
    }

    // Affichage des contrôles
    std::cout << "=== Scene Manager Initialized ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "1: Solar System Scene" << std::endl;
    std::cout << "2: Demo Scene" << std::endl;
    std::cout << "N: Next Scene" << std::endl;
    std::cout << "P: Previous Scene" << std::endl;
    std::cout << "=================================" << std::endl;

    return true;
}

void Cleanup() {
    if (g_SceneManager) {
        g_SceneManager->Cleanup();
        g_SceneManager = nullptr;
    }
    
    g_UI.reset();
    g_Skybox.reset();
    g_Camera.reset();
    UBOManager::Get().Cleanup();
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Erreur : Impossible d'initialiser GLFW" << std::endl;
        return -1;
    }

    if (!Initialize()) {
        std::cerr << "Erreur : Impossible d'initialiser l'application" << std::endl;
        glfwTerminate();
        return -1;
    }

    last_frame_time = glfwGetTime();

    // Boucle principale
    while (!glfwWindowShouldClose(g_Window)) {
        processInput(g_Window);
        Render();
        glfwSwapBuffers(g_Window);
        glfwPollEvents();
    }

    // Nettoyage
    Cleanup();
    glfwDestroyWindow(g_Window);
    glfwTerminate();
    
    return 0;
}

