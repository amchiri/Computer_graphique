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

// Variables globales principales
std::unique_ptr<UI> g_UI;
std::unique_ptr<Skybox> g_Skybox;
std::unique_ptr<CameraController> g_Camera;
std::vector<Mesh*> sceneObjects;
Mesh* sun = nullptr;

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

// Collections d'objets
std::vector<Planet> planets;
std::map<Mesh*, GLShader*> objectShaders;

// Prototypes de fonctions
void createPlanets();
void loadPlanetTextures();
void updatePlanets();
void initializeSolarSystem();
void createSkybox();
void createDemoObjects();

void processInput(GLFWwindow* window) {
    if (g_Camera) {
        g_Camera->Update(elapsed_time);
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

    // Mise à jour des planètes
    updatePlanets();

    // Rendu des objets
    for (Mesh* obj : sceneObjects) {
        GLShader* currentShader = obj->getCurrentShader();
        if (!currentShader) {
            currentShader = g_BasicShader;
            obj->setCurrentShader(currentShader);
        }

        GLuint program = currentShader->GetProgram();
        glUseProgram(program);

        // Matrices communes
        GLint loc_proj = glGetUniformLocation(program, "u_projection");
        if (loc_proj >= 0) glUniformMatrix4fv(loc_proj, 1, GL_FALSE, projectionMatrix.data());
        
        GLint loc_view = glGetUniformLocation(program, "u_view");
        if (loc_view >= 0) glUniformMatrix4fv(loc_view, 1, GL_FALSE, viewMatrix.data());

        // Matrice de transformation de l'objet
        float modelMatrix[16];
        obj->calculateModelMatrix(modelMatrix);
        GLint loc_transform = glGetUniformLocation(program, "u_transform");
        if (loc_transform >= 0) glUniformMatrix4fv(loc_transform, 1, GL_FALSE, modelMatrix);

        // Configuration spécifique selon le shader
        if (currentShader == g_ColorShader) {
            GLint loc_color = glGetUniformLocation(program, "u_color");
            if (loc_color >= 0) {
                const Material& mat = obj->getMaterial();
                glUniform3fv(loc_color, 1, mat.diffuse);
            }
        }
        else if (currentShader == g_BasicShader) {
            // Éclairage
            const float* sunPos = sun->getPosition();
            GLint loc_lightDir = glGetUniformLocation(program, "u_light.direction");
            if (loc_lightDir >= 0) glUniform3f(loc_lightDir, sunPos[0], sunPos[1], sunPos[2]);
            
            float lightDiffuse[3] = {
                light_color[0] * light_intensity * 2.0f,
                light_color[1] * light_intensity * 2.0f,
                light_color[2] * light_intensity * 2.0f
            };

            GLint loc_lightDiffuse = glGetUniformLocation(program, "u_light.diffuseColor");
            GLint loc_lightSpecular = glGetUniformLocation(program, "u_light.specularColor");
            GLint loc_intensity = glGetUniformLocation(program, "u_intensity");
            
            if (loc_intensity >= 0) glUniform1f(loc_intensity, light_intensity);
            if (loc_lightDiffuse >= 0) glUniform3fv(loc_lightDiffuse, 1, lightDiffuse);
            if (loc_lightSpecular >= 0) glUniform3fv(loc_lightSpecular, 1, lightDiffuse);

            // Matériau
            float matDiffuse[3] = {0.8f, 0.8f, 0.8f};
            float matSpecular[3] = {1.0f, 1.0f, 1.0f};
            float matShininess = 20.0f;
            
            GLint loc_matDiffuse = glGetUniformLocation(program, "u_material.diffuseColor");
            GLint loc_matSpecular = glGetUniformLocation(program, "u_material.specularColor");
            GLint loc_matShininess = glGetUniformLocation(program, "u_material.shininess");
            
            if (loc_matDiffuse >= 0) glUniform3fv(loc_matDiffuse, 1, matDiffuse);
            if (loc_matSpecular >= 0) glUniform3fv(loc_matSpecular, 1, matSpecular);
            if (loc_matShininess >= 0) glUniform1f(loc_matShininess, matShininess);

            // Position de la caméra
            GLint loc_viewPos = glGetUniformLocation(program, "u_viewPos");
            if (loc_viewPos >= 0) glUniform3fv(loc_viewPos, 1, g_Camera->GetPosition());

            // Objets émissifs
            GLint loc_isEmissive = glGetUniformLocation(program, "u_material.isEmissive");
            if (loc_isEmissive >= 0) {
                bool isEmissive = (obj == sun);
                glUniform1i(loc_isEmissive, isEmissive ? 1 : 0);
            }
        }
        else if (currentShader == g_EnvMapShader) {
            GLint loc_viewPos = glGetUniformLocation(program, "u_viewPos");
            if (loc_viewPos >= 0) glUniform3fv(loc_viewPos, 1, g_Camera->GetPosition());
        }

        obj->draw(*currentShader);
    }

    // Rendu de la skybox
    if (g_Skybox) {
        g_Skybox->Draw(viewMatrix, projectionMatrix);
    }

    // Interface utilisateur
    glDisable(GL_FRAMEBUFFER_SRGB);
    g_UI->RenderUI(fps, g_Camera->GetPosition(), g_Camera->GetFront());
    glEnable(GL_FRAMEBUFFER_SRGB);
}

void createDemoObjects() {
    // Cube couleur
    Mesh* colorCube = new Mesh();
    colorCube->createSphere(1.0f, 32, 32);
    Material matColor;
    matColor.diffuse[0] = 0.2f; matColor.diffuse[1] = 0.8f; matColor.diffuse[2] = 0.2f;
    matColor.specular[0] = matColor.specular[1] = matColor.specular[2] = 0.0f;
    matColor.shininess = 1.0f;
    matColor.isEmissive = false;
    colorCube->setMaterial(matColor);
    colorCube->setPosition(-20, 5, 0);
    sceneObjects.push_back(colorCube);
    objectShaders[colorCube] = g_ColorShader;

    // Cube texturé
    Mesh* texCube = new Mesh();
    texCube->createSphere(1.0f, 32, 32);
    texCube->loadTexture("models/earth.png");
    Material matTex;
    matTex.diffuse[0] = matTex.diffuse[1] = matTex.diffuse[2] = 1.0f;
    matTex.specular[0] = matTex.specular[1] = matTex.specular[2] = 0.5f;
    matTex.shininess = 16.0f;
    matTex.isEmissive = false;
    texCube->setMaterial(matTex);
    texCube->setPosition(0, 5, 0);
    sceneObjects.push_back(texCube);
    objectShaders[texCube] = g_BasicShader;

    // Cube environment mapping
    Mesh* envCube = new Mesh();
    envCube->createSphere(1.0f, 32, 32);
    Material matEnv;
    matEnv.diffuse[0] = matEnv.diffuse[1] = matEnv.diffuse[2] = 1.0f;
    matEnv.specular[0] = matEnv.specular[1] = matEnv.specular[2] = 1.0f;
    matEnv.shininess = 64.0f;
    matEnv.isEmissive = false;
    envCube->setMaterial(matEnv);
    envCube->setPosition(20, 5, 0);
    sceneObjects.push_back(envCube);
    objectShaders[envCube] = g_EnvMapShader;
}

bool Initialise() {
    // Configuration de la fenêtre
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    g_Window = glfwCreateWindow(width, height, "OpenGL Solar System", nullptr, nullptr);
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
    initializeSolarSystem();
    createDemoObjects();

    // Initialisation de l'interface utilisateur
    g_UI = std::make_unique<UI>(g_Window, width, height);
    g_UI->Initialize();
    g_UI->SetSceneObjects(sceneObjects, sun, planets);
    g_UI->SetShaders(g_BasicShader, g_ColorShader, g_EnvMapShader);
    g_UI->SetLightParameters(light_color, &light_intensity);

    return true;
}

void createSkybox() {
    g_Skybox = std::make_unique<Skybox>();
    if (!g_Skybox->Initialize("space.png")) {
        std::cerr << "Failed to initialize skybox" << std::endl;
    }
}

void initializeSolarSystem() {
    // Création du soleil
    sun = new Mesh();
    sun->createSphere(1.0f, 32, 32);

    const float sunScale = 8.0f;
    sun->setScale(sunScale, sunScale, sunScale);
    sun->setPosition(0.0f, 0.0f, 0.0f);

    Material sunMaterial;
    sunMaterial.diffuse[0] = sunMaterial.diffuse[1] = sunMaterial.diffuse[2] = 1.0f;
    sunMaterial.specular[0] = sunMaterial.specular[1] = sunMaterial.specular[2] = 0.0f;
    sunMaterial.shininess = 0.0f;
    sunMaterial.isEmissive = true;
    sun->setMaterial(sunMaterial);
    sun->loadTexture("models/sun.png");

    sceneObjects.push_back(sun);
    createPlanets();
    loadPlanetTextures();
}

void createPlanets() {
    // Paramètres: rayon orbital, vitesse orbitale, taille
    float planetData[][3] = {
        {15.0f, 0.8f, 0.8f},    // Mercure
        {20.0f, 0.6f, 1.2f},    // Vénus
        {28.0f, 0.4f, 1.5f},    // Terre
        {35.0f, 0.3f, 1.2f},    // Mars
        {50.0f, 0.15f, 4.0f},   // Jupiter
        {65.0f, 0.12f, 3.5f},   // Saturne
        {80.0f, 0.08f, 2.5f},   // Uranus
    };

    for(int i = 0; i < 7; i++) {
        planets.emplace_back();
        planets.back().Initialize(
            planetData[i][0],    // rayon orbital
            planetData[i][1],    // vitesse de rotation
            planetData[i][2]     // taille
        );
        sceneObjects.push_back(planets.back().GetMesh());
    }
}

void updatePlanets() {
    for(auto& planet : planets) {
        planet.Update(elapsed_time);
    }
}

void loadPlanetTextures() {
    const char* textures[] = {
        "models/mercury.png",
        "models/venus.png",
        "models/earth.png",
        "models/mars.png",
        "models/jupiter.png",
        "models/saturn.png",
        "models/uranus.png"
    };

    for(size_t i = 0; i < planets.size(); i++) {
        Planet& planet = planets[i];
        Material planetMaterial;
        planetMaterial.diffuse[0] = planetMaterial.diffuse[1] = planetMaterial.diffuse[2] = 1.0f;
        planetMaterial.specular[0] = planetMaterial.specular[1] = planetMaterial.specular[2] = 0.5f;
        planetMaterial.shininess = 32.0f;
        planetMaterial.isEmissive = false;
        planet.SetMaterial(planetMaterial);
        
        if (!planet.LoadTexture(textures[i])) {
            std::cerr << "Error: Could not load texture for planet " << i << std::endl;
        }
    }
}

void Terminate() {
    // Nettoyage des objets qui ne sont pas des planètes
    std::vector<Mesh*> meshesToDelete;
    for(Mesh* obj : sceneObjects) {
        bool isPlanetMesh = false;
        for(const auto& planet : planets) {
            if(planet.GetMesh() == obj) {
                isPlanetMesh = true;
                break;
            }
        }
        
        if(!isPlanetMesh && obj != sun) {
            meshesToDelete.push_back(obj);
        }
    }

    for(Mesh* obj : meshesToDelete) {
        delete obj;
    }

    delete sun;
    sceneObjects.clear();
    planets.clear();

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