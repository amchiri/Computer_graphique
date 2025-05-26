#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <filesystem>
#include <map>
#include <memory>

#include "GLShader.h"
#include "Mesh.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Mat4.h"
#include "UI.h"
#include "Skybox.h"
#include "CameraController.h"
#include "Planet.h"
#include "ResourceManager.h"
#include "SceneManager.h"

// Variables globales principales
std::unique_ptr<UI> g_UI;
std::unique_ptr<Skybox> g_Skybox;
std::unique_ptr<CameraController> g_Camera;
SceneManager* g_SceneManager = nullptr;

GLShader* g_BasicShader = nullptr;
GLShader* g_ColorShader = nullptr;
GLShader* g_EnvMapShader = nullptr;
GLFWwindow* g_Window = nullptr;

// Paramètres de rendu
int width = 1200, height = 900;
float fov = 60 * (3.14159f / 180.0f);
float cam_far = 1000.0f;
float cam_near = 0.1f;

// Variables d'éclairage
float light_color[3] = { 1.0f, 1.0f, 1.0f };
float light_intensity = 1.0f;

// Variables de temps et debug
float fps = 0.0f;
float elapsed_time = 0.0f;
float last_frame_time = 0.0f;

// Prototypes de fonctions
void createSkybox();
void initializeScenes();

void processInput(GLFWwindow* window) {
    if (g_Camera) {
        g_Camera->Update(elapsed_time);
    }

    // Changement de scène avec les touches 1, 2, etc.
    static bool key1_pressed = false;
    static bool key2_pressed = false;
    static bool keyN_pressed = false;
    static bool keyP_pressed = false;

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !key1_pressed) {
        key1_pressed = true;
        if (g_SceneManager) {
            g_SceneManager->SetActiveScene("Solar System");
            std::cout << "Switched to Solar System scene" << std::endl;
        }
    } else if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE) {
        key1_pressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !key2_pressed) {
        key2_pressed = true;
        if (g_SceneManager) {
            g_SceneManager->SetActiveScene("Demo Scene");
            std::cout << "Switched to Demo scene" << std::endl;
        }
    } else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE) {
        key2_pressed = false;
    }

    // Navigation avec N (Next) et P (Previous)
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS && !keyN_pressed) {
        keyN_pressed = true;
        if (g_SceneManager) {
            g_SceneManager->NextScene();
            std::cout << "Switched to next scene" << std::endl;
        }
    } else if (glfwGetKey(window, GLFW_KEY_N) == GLFW_RELEASE) {
        keyN_pressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !keyP_pressed) {
        keyP_pressed = true;
        if (g_SceneManager) {
            g_SceneManager->PreviousScene();
            std::cout << "Switched to previous scene" << std::endl;
        }
    } else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
        keyP_pressed = false;
    }
}

void Render() {
    // Calcul du FPS
    float current_time = glfwGetTime();
    elapsed_time = current_time - last_frame_time;
    last_frame_time = current_time;
    fps = 1.0f / elapsed_time;

    // Configuration de rendu
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Matrices de projection et vue
    Mat4 projectionMatrix = Mat4::perspective(fov, (float)width / height, cam_near, cam_far);
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

    // Mise à jour et rendu de la scène active
    if (g_SceneManager) {
        g_SceneManager->Update(elapsed_time);
        g_SceneManager->Render(projectionMatrix, viewMatrix, g_BasicShader, g_ColorShader, g_EnvMapShader);
    }

    // Rendu de la skybox
    if (g_Skybox) {
        g_Skybox->Draw(viewMatrix, projectionMatrix);
    }

    // Interface utilisateur
    glDisable(GL_FRAMEBUFFER_SRGB);
    
    // Mise à jour des données UI avec la scène active
    Scene* activeScene = g_SceneManager ? g_SceneManager->GetActiveScene() : nullptr;
    if (activeScene && g_UI) {
        const std::vector<Mesh*>& sceneObjects = activeScene->GetObjects();
        Mesh* sun = activeScene->GetSun();
        const std::vector<Planet>& planets = activeScene->GetPlanets();
        
        g_UI->SetSceneObjects(sceneObjects, sun, planets);
        g_UI->RenderUI(fps, g_Camera->GetPosition(), g_Camera->GetFront());
        
    }
    
    glEnable(GL_FRAMEBUFFER_SRGB);
}

bool Initialise() {
    // Configuration de la fenêtre
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    g_Window = glfwCreateWindow(width, height, "OpenGL Scene Manager", nullptr, nullptr);
    if (!g_Window) {
        std::cerr << "Erreur : Impossible de créer la fenêtre GLFW" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(g_Window);

    // Initialisation de la caméra
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

    // Callback de redimensionnement
    glfwSetFramebufferSizeCallback(g_Window, [](GLFWwindow* window, int w, int h) {
        width = w;
        height = h;
        glViewport(0, 0, width, height);
    });

    // Initialisation de GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Erreur : Impossible d'initialiser GLEW" << std::endl;
        return false;
    }

    // Chargement des shaders
    auto& rm = ResourceManager::Get();
    
    if (!rm.LoadShader("basic", "Basic.vs", "Basic.fs") ||
        !rm.LoadShader("color", "Color.vs", "Color.fs") ||
        !rm.LoadShader("envmap", "EnvMap.vs", "EnvMap.fs")) {
        std::cerr << "Failed to load shaders" << std::endl;
        return false;
    }

    g_BasicShader = rm.GetShader("basic");
    g_ColorShader = rm.GetShader("color");
    g_EnvMapShader = rm.GetShader("envmap");

    if (!g_BasicShader || !g_ColorShader || !g_EnvMapShader) {
        std::cerr << "Failed to get shader pointers" << std::endl;
        return false;
    }

    // Initialisation des objets de la scène
    createSkybox();
    initializeScenes();

    // Initialisation de l'interface utilisateur
    g_UI = std::make_unique<UI>(g_Window, width, height);
    g_UI->Initialize();
    g_UI->SetShaders(g_BasicShader, g_ColorShader, g_EnvMapShader);
    g_UI->SetLightParameters(light_color, &light_intensity);

    std::cout << "=== Scene Manager Initialized ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "1: Switch to Solar System Scene" << std::endl;
    std::cout << "2: Switch to Demo Scene" << std::endl;
    std::cout << "N: Next Scene" << std::endl;
    std::cout << "P: Previous Scene" << std::endl;
    std::cout << "=================================" << std::endl;

    return true;
}

void createSkybox() {
    g_Skybox = std::make_unique<Skybox>();
    if (!g_Skybox->Initialize("space.png")) {
        std::cerr << "Failed to initialize skybox" << std::endl;
    }
}

void initializeScenes() {
    g_SceneManager = &SceneManager::GetInstance();
    
    // Création et ajout de la scène du système solaire
    auto solarSystemScene = std::make_unique<SolarSystemScene>();
    g_SceneManager->AddScene(std::move(solarSystemScene));
    
    // Création et ajout de la scène de démonstration
    auto demoScene = std::make_unique<DemoScene>();
    g_SceneManager->AddScene(std::move(demoScene));
    
    // Initialisation de toutes les scènes
    if (!g_SceneManager->Initialize()) {
        std::cerr << "Failed to initialize scene manager" << std::endl;
    }
    
    // Définir la scène du système solaire comme scène active par défaut
    g_SceneManager->SetActiveScene("Solar System");
}

void Terminate() {
    // Nettoyage du gestionnaire de scènes
    if (g_SceneManager) {
        g_SceneManager->Cleanup();
        g_SceneManager = nullptr;
    }

    // Nettoyage des ressources
    g_UI.reset();
    g_Skybox.reset();
    g_Camera.reset();
}

int main() {
    // Initialisation de GLFW
    if (!glfwInit()) {
        std::cerr << "Erreur : Impossible d'initialiser GLFW" << std::endl;
        return -1;
    }

    // Initialisation
    if (!Initialise()) {
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
    Terminate();
    glfwDestroyWindow(g_Window);
    glfwTerminate();
    return 0;
}