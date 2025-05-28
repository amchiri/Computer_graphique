#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <memory>

#include "rendering/GLShader.h"
#include "rendering/Mesh.h"
#include "ui/imgui/imgui.h"
#include "ui/imgui/imgui_impl_glfw.h"
#include "ui/imgui/imgui_impl_opengl3.h"
#include "core/Mat4.h"
#include "ui/UI.h"
#include "rendering/Skybox.h"
#include "utils/CameraController.h"
#include "rendering/Planet.h"
#include "core/ResourceManager.h"
#include "core/SceneManager.h"

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
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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

    // Rendu de la scène active
    if (g_SceneManager) {
        g_SceneManager->Update(elapsed_time);
        g_SceneManager->Render(projectionMatrix, viewMatrix);
    }

    // Rendu de la skybox
    if (g_Skybox) {
        g_Skybox->Draw(viewMatrix, projectionMatrix);
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

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, 
                              GLsizei length, const GLchar* message, const void* userParam) {
    std::string severityStr;
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH: severityStr = "HIGH"; break;
        case GL_DEBUG_SEVERITY_MEDIUM: severityStr = "MEDIUM"; break;
        case GL_DEBUG_SEVERITY_LOW: severityStr = "LOW"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: return; // Skip notifications
        default: severityStr = "UNKNOWN"; break;
    }
    
    std::string typeStr;
    switch (type) {
        case GL_DEBUG_TYPE_ERROR: typeStr = "ERROR"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "DEPRECATED"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: typeStr = "UNDEFINED"; break;
        case GL_DEBUG_TYPE_PORTABILITY: typeStr = "PORTABILITY"; break;
        case GL_DEBUG_TYPE_PERFORMANCE: typeStr = "PERFORMANCE"; break;
        default: typeStr = "OTHER"; break;
    }

    fprintf(stderr, "GL %s [%s] (ID: %u): %s\n", 
            severityStr.c_str(), typeStr.c_str(), id, message);
}

bool initializeOpenGL() {
    // Configuration de la fenêtre GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE); // Enable debug context
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    g_Window = glfwCreateWindow(width, height, "OpenGL Scene Manager", nullptr, nullptr);
    if (!g_Window) {
        const char* error_description;
        glfwGetError(&error_description);
        std::cerr << "Erreur : Impossible de créer la fenêtre GLFW: " << error_description << std::endl;
        return false;
    }
    
    glfwMakeContextCurrent(g_Window);

    // Initialize GLEW
    glewExperimental = GL_TRUE; // Enable experimental features
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Erreur GLEW: " << glewGetErrorString(err) << std::endl;
        return false;
    }

    // Setup debug callback
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(MessageCallback, 0);

    // Print OpenGL version info
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLEW Version: " << glewGetString(GLEW_VERSION) << std::endl;
    std::cout << "GLFW Version: " << glfwGetVersionString() << std::endl;
    std::cout << "GPU Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "GPU Renderer: " << glGetString(GL_RENDERER) << std::endl;

    // Clear any existing errors
    while (glGetError() != GL_NO_ERROR);

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
        {"basic", "Basic.vs", "Basic.fs"},
        {"color", "Color.vs", "Color.fs"},
        {"envmap", "EnvMap.vs", "EnvMap.fs"}
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
    std::cout << "Initializing skybox..." << std::endl;
    g_Skybox = std::make_unique<Skybox>();
    if (!g_Skybox->Initialize("src/resources/textures/space.png")) {
        std::cerr << "Failed to initialize skybox with texture: src/resources/textures/space.png" << std::endl;
        return false;
    }
    std::cout << "Skybox initialized successfully" << std::endl;
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
    if (!initializeOpenGL() || 
        !initializeCamera() || 
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
}

int main() {
    // Set error callback before init
    glfwSetErrorCallback([](int error, const char* description) {
        std::cerr << "GLFW Error " << error << ": " << description << std::endl;
    });

    if (!glfwInit()) {
        std::cerr << "Erreur : Impossible d'initialiser GLFW" << std::endl;
        return -1;
    }

    try {
        if (!Initialize()) {
            std::cerr << "Erreur : Impossible d'initialiser l'application" << std::endl;
            glfwTerminate();
            return -1;
        }

        last_frame_time = glfwGetTime();

        // Boucle principale
        while (!glfwWindowShouldClose(g_Window)) {
            try {
                processInput(g_Window);
                Render();
                glfwSwapBuffers(g_Window);
                glfwPollEvents();
            } catch (const std::exception& e) {
                std::cerr << "Error in main loop: " << e.what() << std::endl;
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
    }

    // Nettoyage
    Cleanup();
    glfwDestroyWindow(g_Window);
    glfwTerminate();
    
    return 0;
}