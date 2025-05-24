#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <algorithm>  // Ajouter cette ligne pour std::find_if

#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <filesystem>
#include "GLShader.h"
#include "dragonData.h"
#include "Mesh.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui/imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "Mat4.h"
#include <windows.h>
#include <commdlg.h>
#include <map>

// Skybox variables
GLuint skyboxVAO, skyboxVBO;
GLuint skyboxTexture;
GLShader g_SkyboxShader;

GLShader g_BasicShader;
GLFWwindow* g_Window = nullptr; // Variable globale pour la fenêtre
int width = 1200, height = 900; // taille de la fenêtre

float fov = 60 * (3.14159f / 180.0f); // angle d'ouverture de 60°
float cam_far = 1000.0f;  // Augmenter cette valeur (était 100.0f)
float cam_near = 0.1f;

// Structure et variables pour la caméra
struct Camera {
    float position[3] = {0.0f, 0.0f, 3.0f};
    float front[3] = {0.0f, 0.0f, -1.0f};
    float up[3] = {0.0f, 1.0f, 0.0f};
    float yaw = -90.0f;
    float pitch = 0.0f;
    float speed = 0.05f;
    float sensitivity = 0.1f;
} camera;

float lastX = width / 2.0f;
float lastY = height / 2.0f;
bool firstMouse = true;

// Variables globales pour ImGui
bool show_demo_window = true;
bool show_settings = true;
float light_color[3] = { 1.0f, 1.0f, 1.0f };
float light_intensity = 1.0f;

// Variables globales supplémentaires
float fps = 0.0f;
bool wireframe_mode = false;
bool show_debug_window = true;
bool cursor_locked = true;
char obj_path[256] = "models/dragon.obj";
float elapsed_time = 0.0f;
float last_frame_time = 0.0f;

// Variables globales ajoutées
bool camera_enabled = true;
char selected_file[256] = "";
char selected_texture[256] = "";
int selected_object = -1;

// Structure pour les planètes
struct Planet {
    Mesh* mesh;
    float orbitRadius;
    float rotationSpeed;
    float selfRotation;
    float size;
    GLuint texture;
};

std::vector<Planet> planets;

// Structure pour les transformations des objets
struct ObjectTransform {
    float position[3] = {0.0f, 0.0f, 0.0f};
    float rotation[3] = {0.0f, 0.0f, 0.0f};
    float scale[3] = {1.0f, 1.0f, 1.0f};
};

// Map globale pour stocker les transformations des objets
std::map<Mesh*, ObjectTransform> objectTransforms;

// Déclarations des prototypes de fonctions
void createPlanets();
void loadPlanetTextures();
void updatePlanets();
void initializeSolarSystem();
void createSkybox();
void AdjustInitialCamera();

// Déclarer les prototypes des fonctions utilisées dans RenderImGuiObjectControls
std::string OpenFileDialog(const char* filter = "OBJ files\0*.obj\0All files\0*.*\0");
bool LoadModelWithTexture(const std::string& modelPath);

// Fonction de callback pour la souris
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!camera_enabled || !cursor_locked) {
        lastX = xpos;
        lastY = ypos;
        return;
    }

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
        return;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    xoffset *= camera.sensitivity;
    yoffset *= camera.sensitivity;

    camera.yaw += xoffset;
    camera.pitch += yoffset;

    if (camera.pitch > 89.0f) camera.pitch = 89.0f;
    if (camera.pitch < -89.0f) camera.pitch = -89.0f;

    float front[3];
    front[0] = cos(camera.yaw * 3.14159f/180.0f) * cos(camera.pitch * 3.14159f/180.0f);
    front[1] = sin(camera.pitch * 3.14159f/180.0f);
    front[2] = sin(camera.yaw * 3.14159f/180.0f) * cos(camera.pitch * 3.14159f/180.0f);
    
    // Normaliser le vecteur front
    float length = sqrt(front[0]*front[0] + front[1]*front[1] + front[2]*front[2]);
    camera.front[0] = front[0]/length;
    camera.front[1] = front[1]/length;
    camera.front[2] = front[2]/length;
}

// Fonction de gestion des entrées
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
        cursor_locked = !cursor_locked;
        camera_enabled = !camera_enabled;
        glfwSetInputMode(window, GLFW_CURSOR, 
            cursor_locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        glfwWaitEvents();
    }

    // Ne traiter les inputs de caméra que si le curseur est verrouillé ET la caméra est activée
    if (!cursor_locked || !camera_enabled || ImGui::GetIO().WantCaptureMouse) {
        return;
    }

    float cameraSpeed = camera.speed;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.position[0] += cameraSpeed * camera.front[0];
        camera.position[1] += cameraSpeed * camera.front[1];
        camera.position[2] += cameraSpeed * camera.front[2];
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.position[0] -= cameraSpeed * camera.front[0];
        camera.position[1] -= cameraSpeed * camera.front[1];
        camera.position[2] -= cameraSpeed * camera.front[2];
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        // Cross product de front et up pour avoir le vecteur right
        float right[3];
        right[0] = camera.front[1]*camera.up[2] - camera.front[2]*camera.up[1];
        right[1] = camera.front[2]*camera.up[0] - camera.front[0]*camera.up[2];
        right[2] = camera.front[0]*camera.up[1] - camera.front[1]*camera.up[0];
        float length = sqrt(right[0]*right[0] + right[1]*right[1] + right[2]*right[2]);
        camera.position[0] -= cameraSpeed * right[0]/length;
        camera.position[1] -= cameraSpeed * right[1]/length;
        camera.position[2] -= cameraSpeed * right[2]/length;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        float right[3];
        right[0] = camera.front[1]*camera.up[2] - camera.front[2]*camera.up[1];
        right[1] = camera.front[2]*camera.up[0] - camera.front[0]*camera.up[2];
        right[2] = camera.front[0]*camera.up[1] - camera.front[1]*camera.up[0];
        float length = sqrt(right[0]*right[0] + right[1]*right[1] + right[2]*right[2]);
        camera.position[0] += cameraSpeed * right[0]/length;
        camera.position[1] += cameraSpeed * right[1]/length;
        camera.position[2] += cameraSpeed * right[2]/length;
    }
}

std::vector<Mesh*> sceneObjects;
Mesh* sun;

const char* tab[]={
	"models/mercury.png",
	"models/venus.png",
	"models/earth.png",
	"models/mars.png",
	"models/jupiter.png",
	"models/saturn.png",
	"models/uranus.png",
	"models/neptune.png"
};

void loadPlanetTextures() {
    for(size_t i = 0; i < planets.size(); i++) {
        Planet& planet = planets[i];
        if (planet.mesh) {
            // Configuration du matériau
            Material planetMaterial;
            planetMaterial.diffuse[0] = planetMaterial.diffuse[1] = planetMaterial.diffuse[2] = 1.0f;
            planetMaterial.specular[0] = planetMaterial.specular[1] = planetMaterial.specular[2] = 0.5f;
            planetMaterial.shininess = 32.0f;
            planetMaterial.isEmissive = false;
            planet.mesh->setMaterial(planetMaterial);
            
            // Charger la texture
            if (planet.mesh->loadTexture(tab[i])) {
                std::cout << "Texture earth.png chargée avec succès pour la planète " << i << std::endl;
                planet.texture = planet.mesh->getMaterial().diffuseMap;
            } else {
                std::cerr << "Erreur : impossible de charger earth.png pour la planète " << i << std::endl;
            }
        }
    }
}

bool Initialise()
{
    // Configurer GLFW pour permettre le redimensionnement
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    g_Window = glfwCreateWindow(width, height, "OpenGL Window", nullptr, nullptr);
    if (!g_Window) {
        std::cerr << "Erreur : Impossible de créer la fenêtre GLFW" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(g_Window);

    // Ajouter les callbacks pour la souris
    glfwSetCursorPosCallback(g_Window, mouse_callback);
    glfwSetInputMode(g_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Configurer le callback de redimensionnement
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

	g_BasicShader.LoadVertexShader("Basic.vs");
	g_BasicShader.LoadFragmentShader("Basic.fs");
	g_BasicShader.Create();

	// Création du soleil et des planètes
	initializeSolarSystem();

	// Ajuster la caméra pour mieux voir le système solaire
	AdjustInitialCamera();

	// Configuration d'ImGui avec une plus grande taille de police
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.FontGlobalScale = 1.5f; // Augmente la taille globale des éléments ImGui
    
    // Style personnalisé pour ImGui
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(15, 15);
    style.FramePadding = ImVec2(5, 5);
    style.ItemSpacing = ImVec2(12, 8);
    style.ItemInnerSpacing = ImVec2(8, 6);
    style.WindowRounding = 12.0f;
    style.FrameRounding = 5.0f;
    style.PopupRounding = 10.0f;
    style.ScrollbarRounding = 10.0f;
    style.GrabRounding = 5.0f;
    
    // Augmenter la taille des widgets
    style.ScrollbarSize = 18;
    style.GrabMinSize = 20;
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);  // Centre les titres des fenêtres

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(g_Window, true);
	ImGui_ImplOpenGL3_Init("#version 130");

	// Création de la skybox
	createSkybox();

    // Active le framebuffer sRGB pour tout le projet (avant toute création d'objet OpenGL)
    glEnable(GL_FRAMEBUFFER_SRGB);

	return true;
}

void createSkybox() {
    float skyboxVertices[] = {
        -1000.0f,  1000.0f, -1000.0f,
        -1000.0f, -1000.0f, -1000.0f,
         1000.0f, -1000.0f, -1000.0f,
         1000.0f, -1000.0f, -1000.0f,
         1000.0f,  1000.0f, -1000.0f,
        -1000.0f,  1000.0f, -1000.0f,

        -1000.0f, -1000.0f,  1000.0f,
        -1000.0f, -1000.0f, -1000.0f,
        -1000.0f,  1000.0f, -1000.0f,
        -1000.0f,  1000.0f, -1000.0f,
        -1000.0f,  1000.0f,  1000.0f,
        -1000.0f, -1000.0f,  1000.0f,

         1000.0f, -1000.0f, -1000.0f,
         1000.0f, -1000.0f,  1000.0f,
         1000.0f,  1000.0f,  1000.0f,
         1000.0f,  1000.0f,  1000.0f,
         1000.0f,  1000.0f, -1000.0f,
         1000.0f, -1000.0f, -1000.0f,

        -1000.0f, -1000.0f,  1000.0f,
        -1000.0f,  1000.0f,  1000.0f,
         1000.0f,  1000.0f,  1000.0f,
         1000.0f,  1000.0f,  1000.0f,
         1000.0f, -1000.0f,  1000.0f,
        -1000.0f, -1000.0f,  1000.0f,

        -1000.0f,  1000.0f, -1000.0f,
         1000.0f,  1000.0f, -1000.0f,
         1000.0f,  1000.0f,  1000.0f,
         1000.0f,  1000.0f,  1000.0f,
        -1000.0f,  1000.0f,  1000.0f,
        -1000.0f,  1000.0f, -1000.0f,

        -1000.0f, -1000.0f, -1000.0f,
        -1000.0f, -1000.0f,  1000.0f,
         1000.0f, -1000.0f, -1000.0f,
         1000.0f, -1000.0f, -1000.0f,
        -1000.0f, -1000.0f,  1000.0f,
         1000.0f, -1000.0f,  1000.0f
    };

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // Charger la texture de l'espace
    int spaceTexWidth, spaceTexHeight, spaceTexChannels;
    unsigned char* spaceData = stbi_load("space.png", &spaceTexWidth, &spaceTexHeight, &spaceTexChannels, STBI_rgb_alpha);
    if (spaceData) {
        glGenTextures(1, &skyboxTexture);
        glBindTexture(GL_TEXTURE_2D, skyboxTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8 , spaceTexWidth, spaceTexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, spaceData);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(spaceData);
    }

    // Charger les shaders du skybox
    g_SkyboxShader.LoadVertexShader("Skybox.vs");
    g_SkyboxShader.LoadFragmentShader("Skybox.fs");
    g_SkyboxShader.Create();
}

void initializeSolarSystem() {
    // Création du soleil
    sun = new Mesh();
    sun->createSphere(1.0f, 32, 32);

    // Position et échelle du soleil au centre
    const float sunInitialScale = 8.0f;
    const float sunInitialPos[3] = {0.0f, 0.0f, 0.0f};  // Centre de la scène

    sun->setScale(sunInitialScale, sunInitialScale, sunInitialScale);
    sun->setPosition(sunInitialPos[0], sunInitialPos[1], sunInitialPos[2]);

    Material sunMaterial;
    sunMaterial.diffuse[0] = sunMaterial.diffuse[1] = sunMaterial.diffuse[2] = 1.0f;
    sunMaterial.specular[0] = sunMaterial.specular[1] = sunMaterial.specular[2] = 0.0f;
    sunMaterial.shininess = 0.0f;
    sunMaterial.isEmissive = true;
    sun->setMaterial(sunMaterial);
    sun->loadTexture("models/sun.png");

    // Initialiser les transformations du soleil
    ObjectTransform& sunTransform = objectTransforms[sun];
    memcpy(sunTransform.position, sunInitialPos, sizeof(float) * 3);
    sunTransform.scale[0] = sunTransform.scale[1] = sunTransform.scale[2] = sunInitialScale;
    sunTransform.rotation[0] = sunTransform.rotation[1] = sunTransform.rotation[2] = 0.0f;

    sceneObjects.push_back(sun);
    createPlanets();
    loadPlanetTextures();
}

void createPlanets() {
    // Paramètres des planètes: rayon orbital, vitesse orbitale, taille
    float planetData[][4] = {
        {15.0f, 0.8f, 0.8f},    // Mercure - plus proche, plus rapide
        {20.0f, 0.6f, 1.2f},    // Vénus
        {28.0f, 0.4f, 1.5f},    // Terre
        {35.0f, 0.3f, 1.2f},    // Mars
        {50.0f, 0.15f, 4.0f},   // Jupiter - plus gros
        {65.0f, 0.12f, 3.5f},   // Saturne
        {80.0f, 0.08f, 2.5f},   // Uranus
    };

    float time = glfwGetTime();

    for(int i = 0; i < 7; i++) {
        Planet planet;
        planet.mesh = new Mesh();
        planet.mesh->createSphere(1.0f, 32, 32);
        
        // Initialisation des paramètres de la planète
        planet.orbitRadius = planetData[i][0];
        planet.rotationSpeed = planetData[i][1];
        planet.size = planetData[i][2];
        planet.selfRotation = 0.0f;

        // Calculer la position initiale sur l'orbite
        float angle = time * planet.rotationSpeed;
        float x = cos(angle) * planet.orbitRadius;
        float z = sin(angle) * planet.orbitRadius;

        // Appliquer les transformations initiales
        planet.mesh->setScale(planet.size, planet.size, planet.size);
        planet.mesh->setPosition(x, 0.0f, z);
        
        // Configuration du matériau
        Material planetMaterial;
        planetMaterial.diffuse[0] = planetMaterial.diffuse[1] = planetMaterial.diffuse[2] = 1.0f;
        planetMaterial.specular[0] = planetMaterial.specular[1] = planetMaterial.specular[2] = 0.5f;
        planetMaterial.shininess = 32.0f;
        planetMaterial.isEmissive = false;
        planet.mesh->setMaterial(planetMaterial);

        // Initialiser les transformations stockées
        ObjectTransform& planetTransform = objectTransforms[planet.mesh];
        planetTransform.position[0] = x;
        planetTransform.position[1] = 0.0f;
        planetTransform.position[2] = z;
        planetTransform.scale[0] = planetTransform.scale[1] = planetTransform.scale[2] = planet.size;
        planetTransform.rotation[0] = planetTransform.rotation[1] = planetTransform.rotation[2] = 0.0f;

        planets.push_back(planet);
        sceneObjects.push_back(planet.mesh);
    }
}

void updatePlanets() {
    float time = glfwGetTime();
    const float* sunPos = sun->getPosition();
    
    // Mettre à jour la position des planètes
    for(size_t i = 0; i < planets.size(); i++) {
        Planet& planet = planets[i];
        
        // Calcul de la position orbitale
        float angle = time * planet.rotationSpeed;
        float x = cos(angle) * planet.orbitRadius;
        float z = sin(angle) * planet.orbitRadius;
        
        // Créer la matrice de transformation
        Mat4 translation = Mat4::translate(x, 0.0f, z);
        Mat4 rotation = Mat4::rotate(planet.selfRotation, 0.0f, 1.0f, 0.0f);
        Mat4 scale = Mat4::scale(planet.size, planet.size, planet.size);
        
        // Combiner les transformations
        Mat4 transform = translation * rotation * scale;
        
        // Appliquer la transformation
        planet.mesh->setTransform(transform);
        
        // Mettre à jour les transformations stockées
        ObjectTransform& objectTransform = objectTransforms[planet.mesh];
        objectTransform.position[0] = x;
        objectTransform.position[1] = 0.0f;
        objectTransform.position[2] = z;
        objectTransform.scale[0] = objectTransform.scale[1] = objectTransform.scale[2] = planet.size;
        
        // Mettre à jour la rotation
        planet.selfRotation += elapsed_time * 0.5f;
    }
}

void AdjustInitialCamera() {
    camera.position[0] = 0.0f;
    camera.position[1] = 50.0f;  // Plus haut pour voir le système
    camera.position[2] = 100.0f; // Plus loin pour tout voir
    camera.pitch = -30.0f;       // Regarder vers le bas
    camera.yaw = -90.0f;
}

void Terminate()
{
	g_BasicShader.Destroy();

	// Nettoyer les objets de la scène
	for(Mesh* obj : sceneObjects) {
		delete obj;
	}
	sceneObjects.clear();

	// Cleanup ImGui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// Cleanup Skybox
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);
	glDeleteTextures(1, &skyboxTexture);

	// Nettoyer les planètes
	for(Planet& planet : planets) {
		if(planet.texture != 0) {
			glDeleteTextures(1, &planet.texture);
		}
	}
}

void RenderImGuiObjectControls() {
    if (!show_debug_window) return;

    if (ImGui::CollapsingHeader("Scene Objects")) {
        // Le Soleil
        if (ImGui::TreeNode("Sun")) {
            ObjectTransform& transform = objectTransforms[sun];
            
            if (ImGui::DragFloat3("Position", transform.position, 0.1f)) {
                sun->setPosition(transform.position[0], transform.position[1], transform.position[2]);
            }
            
            if (ImGui::DragFloat3("Scale", transform.scale, 0.1f, 0.1f, 100.0f)) {
                sun->setScale(transform.scale[0], transform.scale[1], transform.scale[2]);
            }
            
            if (ImGui::DragFloat3("Rotation", transform.rotation, 0.1f)) {
                Mat4 rotMatrix = Mat4::rotate(transform.rotation[0], 1, 0, 0) *
                               Mat4::rotate(transform.rotation[1], 0, 1, 0) *
                               Mat4::rotate(transform.rotation[2], 0, 0, 1);
                sun->setRotation(rotMatrix);
            }
            
            ImGui::TreePop();
        }
        
        // Les Planètes
        if (ImGui::TreeNode("Planets")) {
            for (size_t i = 0; i < planets.size(); i++) {
                char label[32];
                sprintf(label, "Planet %zu", i);
                
                if (ImGui::TreeNode(label)) {
                    Planet& planet = planets[i];
                    ObjectTransform& transform = objectTransforms[planet.mesh];
                    
                    if (ImGui::DragFloat("Orbit Radius", &planet.orbitRadius, 0.1f, 1.0f, 200.0f)) {
                        // La mise à jour de la position sera faite dans updatePlanets
                    }
                    
                    if (ImGui::DragFloat("Size", &planet.size, 0.1f, 0.1f, 100.0f)) {
                        transform.scale[0] = transform.scale[1] = transform.scale[2] = planet.size;
                    }
                    
                    if (ImGui::DragFloat("Orbital Speed", &planet.rotationSpeed, 0.01f, 0.0f, 2.0f)) {
                        // La vitesse de rotation sera utilisée dans updatePlanets
                    }
                    
                    ImGui::Text("Current Position: %.2f, %.2f, %.2f",
                        transform.position[0], transform.position[1], transform.position[2]);
                    
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }

        // Objets chargés dynamiquement
        if (ImGui::TreeNode("Custom Objects")) {
            for (size_t i = 0; i < sceneObjects.size(); i++) {
                // Skip sun and planets
                if (sceneObjects[i] == sun || std::find_if(planets.begin(), planets.end(),
                    [&](const Planet& p) { return p.mesh == sceneObjects[i]; }) != planets.end()) {
                    continue;
                }

                char label[32];
                sprintf(label, "Object %zu", i);
                
                if (ImGui::TreeNode(label)) {
                    ObjectTransform& transform = objectTransforms[sceneObjects[i]];
                    
                    if (ImGui::DragFloat3("Position", transform.position, 0.1f)) {
                        sceneObjects[i]->setPosition(transform.position[0], transform.position[1], transform.position[2]);
                    }
                    
                    if (ImGui::DragFloat3("Scale", transform.scale, 0.1f, 0.1f, 100.0f)) {
                        sceneObjects[i]->setScale(transform.scale[0], transform.scale[1], transform.scale[2]);
                    }
                    
                    if (ImGui::DragFloat3("Rotation", transform.rotation, 0.1f)) {
                        Mat4 rotMatrix = Mat4::rotate(transform.rotation[0], 1, 0, 0) *
                                       Mat4::rotate(transform.rotation[1], 0, 1, 0) *
                                       Mat4::rotate(transform.rotation[2], 0, 0, 1);
                        sceneObjects[i]->setRotation(rotMatrix);
                    }

                    // Option pour supprimer l'objet
                    if (ImGui::Button("Delete Object")) {
                        objectTransforms.erase(sceneObjects[i]);
                        delete sceneObjects[i];
                        sceneObjects.erase(sceneObjects.begin() + i);
                        ImGui::TreePop();
                        break;
                    }
                    
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
    }

    if (ImGui::CollapsingHeader("Model Loader")) {
        if (ImGui::Button("Load 3D Model")) {
            std::string filepath = OpenFileDialog();
            if (!filepath.empty()) {
                if (!LoadModelWithTexture(filepath)) {
                    ImGui::OpenPopup("Error Loading Model");
                }
            }
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Load Texture")) {
            if (selected_object >= 0 && selected_object < sceneObjects.size()) {
                std::string texturePath = OpenFileDialog("Image files\0*.png;*.jpg;*.jpeg\0All files\0*.*\0");
                if (!texturePath.empty()) {
                    sceneObjects[selected_object]->loadTexture(texturePath.c_str());
                }
            }
        }

        // Liste des objets chargés
        ImGui::Text("Loaded Objects:");
        for (size_t i = 0; i < sceneObjects.size(); i++) {
            char label[32];
            sprintf(label, "Object %zu", i);
            bool is_selected = (selected_object == i);
            if (ImGui::Selectable(label, is_selected)) {
                selected_object = i;
            }
        }
        
        if (ImGui::BeginPopupModal("Error Loading Model", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Failed to load the model!");
            if (ImGui::Button("OK")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}

// Déplacer les implémentations des fonctions OpenFileDialog et LoadModelWithTexture ici,
// avant leur utilisation dans RenderImGuiObjectControls
std::string OpenFileDialog(const char* filter) {
    char filename[MAX_PATH];
    filename[0] = '\0';

    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "";

    if (GetOpenFileNameA(&ofn))
        return filename;
    return "";
}

bool LoadModelWithTexture(const std::string& modelPath) {
    Mesh* new_model = new Mesh();
    if (!new_model->loadFromOBJFile(modelPath.c_str())) {
        delete new_model;
        return false;
    }

    // Chercher une texture avec le même nom que le modèle
    std::string basePath = modelPath.substr(0, modelPath.find_last_of("."));
    std::string texturePath = basePath + ".png"; // Essayer d'abord PNG
    
    if (!std::filesystem::exists(texturePath)) {
        texturePath = basePath + ".jpg"; // Essayer ensuite JPG
    }

    if (std::filesystem::exists(texturePath)) {
        new_model->loadTexture(texturePath.c_str());
    }

    new_model->setScale(0.5f, 0.5f, 0.5f);
    sceneObjects.push_back(new_model);
    
    // Initialiser les transformations
    ObjectTransform transform;
    objectTransforms[new_model] = transform;
    
    return true;
}

void Render()
{
	// Calcul du FPS
	float current_time = glfwGetTime();
	elapsed_time = current_time - last_frame_time;
	last_frame_time = current_time;
	fps = 1.0f / elapsed_time;

	// paramétrage de la fenêtre
	glViewport(0, 0, width, height);
	glEnable(GL_DEPTH_TEST);

	// Nettoyer le buffer avec une couleur noire
	glClearColor(0.0f, 0.0f, 0.0f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto basicProgram = g_BasicShader.GetProgram();
	glUseProgram(basicProgram);

	// Create and pass the perspective projection matrix.
	Mat4 projectionMatrix = Mat4::perspective(fov, (float)width / height, cam_near, cam_far);
	GLint loc_projection = glGetUniformLocation(basicProgram, "u_projection");
	glUniformMatrix4fv(loc_projection, 1, GL_FALSE, projectionMatrix.data());

	// Créer la matrice de vue
	float center[3] = {
		camera.position[0] + camera.front[0],
		camera.position[1] + camera.front[1],
		camera.position[2] + camera.front[2]
	};
	Mat4 viewMatrix = Mat4::lookAt(camera.position, center, camera.up);

	// Passer la matrice de vue au shader
	GLint loc_view = glGetUniformLocation(basicProgram, "u_view");
	glUniformMatrix4fv(loc_view, 1, GL_FALSE, viewMatrix.data());

	// Rendre d'abord les objets de la scène
	for(Mesh* obj : sceneObjects) {
        if(obj == sun) {
            Material sunMat = obj->getMaterial();
            sunMat.isEmissive = true;
            obj->setMaterial(sunMat);
        }
        obj->draw(g_BasicShader);
    }

    // Puis rendre la skybox en dernier
	glDepthFunc(GL_LEQUAL);
	glUseProgram(g_SkyboxShader.GetProgram());

	// Enlever la translation de la matrice de vue pour le skybox
	Mat4 skyboxView = viewMatrix;
	float* skyboxData = skyboxView.data();
	skyboxData[12] = 0.0f;
	skyboxData[13] = 0.0f;
	skyboxData[14] = 0.0f;

	GLint skyboxViewLoc = glGetUniformLocation(g_SkyboxShader.GetProgram(), "u_view");
	GLint skyboxProjLoc = glGetUniformLocation(g_SkyboxShader.GetProgram(), "u_projection");
	glUniformMatrix4fv(skyboxViewLoc, 1, GL_FALSE, skyboxView.data());
	glUniformMatrix4fv(skyboxProjLoc, 1, GL_FALSE, projectionMatrix.data());

	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, skyboxTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	
	// Débinder la texture du skybox avant de passer aux planètes
	glBindTexture(GL_TEXTURE_2D, 0);

	// Réinitialiser les états OpenGL pour le rendu normal
	glDepthFunc(GL_LESS);
	glUseProgram(basicProgram);

	// Mise à jour de la position de la lumière pour qu'elle suive le soleil
	const float* sunPos = sun->getPosition();
	GLint loc_lightDir = glGetUniformLocation(basicProgram, "u_light.direction");
	
	// La direction de la lumière est la position du soleil
	glUniform3f(loc_lightDir, sunPos[0], sunPos[1], sunPos[2]);
	
	// Augmenter l'intensité de la lumière du soleil
	float lightDiffuse[3] = {
		light_color[0] * light_intensity * 2.0f,
		light_color[1] * light_intensity * 2.0f,
		light_color[2] * light_intensity * 2.0f
	};

	// Configuration de la lumière - Utiliser la position du soleil comme source de lumière
	GLint loc_lightDiffuse = glGetUniformLocation(basicProgram, "u_light.diffuseColor");
	GLint loc_lightSpecular = glGetUniformLocation(basicProgram, "u_light.specularColor");
	GLint loc_intensity = glGetUniformLocation(basicProgram, "u_intensity");
	
	glUniform1f(loc_intensity, light_intensity);
	glUniform3fv(loc_lightDiffuse, 1, lightDiffuse);
	glUniform3fv(loc_lightSpecular, 1, lightDiffuse);

	// Configuration du matériau
	float matDiffuse[3] = {0.8f, 0.8f, 0.8f};
	float matSpecular[3] = {1.0f, 1.0f, 1.0f};
	float matShininess = 20.0f;
	
	GLint loc_matDiffuse = glGetUniformLocation(basicProgram, "u_material.diffuseColor");
	GLint loc_matSpecular = glGetUniformLocation(basicProgram, "u_material.specularColor");
	GLint loc_matShininess = glGetUniformLocation(basicProgram, "u_material.shininess");
	
	glUniform3fv(loc_matDiffuse, 1, matDiffuse);
	glUniform3fv(loc_matSpecular, 1, matSpecular);
	glUniform1f(loc_matShininess, matShininess);

	// Position de la caméra
	GLint loc_viewPos = glGetUniformLocation(basicProgram, "u_viewPos");
	glUniform3fv(loc_viewPos, 1, camera.position);

	// Gestion des objets émissifs
	GLint loc_isEmissive = glGetUniformLocation(basicProgram, "u_material.isEmissive");
	glUniform1i(loc_isEmissive, 0);

	// Mise à jour des planètes
	updatePlanets();

	// Modifier la manière dont les objets sont rendus
	for(Mesh* obj : sceneObjects) {
        if(obj == sun) {
            Material sunMat = obj->getMaterial();
            sunMat.isEmissive = true;
            obj->setMaterial(sunMat);
			
        }
        
        // Chaque objet va gérer sa propre texture dans sa méthode draw()
        obj->draw(g_BasicShader);
        
        // Débinder la texture après chaque objet
        glBindTexture(GL_TEXTURE_2D, 0);
    }

	// Début du rendu ImGui
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Ajuster la taille des fenêtres ImGui en fonction de la taille de la fenêtre
	ImGui::SetNextWindowSize(ImVec2(width * 0.3f, height * 0.7f), ImGuiCond_FirstUseEver);
	if (show_debug_window) {
		ImGui::Begin("Debug", &show_debug_window, ImGuiWindowFlags_NoCollapse);
		ImGui::Text("FPS: %.1f", fps);
		ImGui::Text("Camera Position: (%.1f, %.1f, %.1f)", 
			camera.position[0], camera.position[1], camera.position[2]);
		ImGui::Text("Camera Direction: (%.1f, %.1f, %.1f)", 
			camera.front[0], camera.front[1], camera.front[2]);

		if (ImGui::Checkbox("Wireframe Mode", &wireframe_mode)) {
			glPolygonMode(GL_FRONT_AND_BACK, wireframe_mode ? GL_LINE : GL_FILL);
		}

		// Contrôle des paramètres de lumière
		if (ImGui::CollapsingHeader("Light Settings")) {
			ImGui::ColorEdit3("Light Color", light_color);
			ImGui::SliderFloat("Light Intensity", &light_intensity, 0.0f, 10.0f);

			// Création d'une variable temporaire pour la position
			float lightPos[3];
			memcpy(lightPos, sun->getPosition(), 3 * sizeof(float));
			if (ImGui::SliderFloat3("Light Position", lightPos, -50.0f, 50.0f)) {
				sun->setPosition(lightPos[0], lightPos[1], lightPos[2]);
			}
		}


		// Ajout de debug pour la texture dans l'interface ImGui
		if (ImGui::CollapsingHeader("Texture Debug")) {
			ImGui::Text("Texture ID: %d", skyboxTexture);
			
			// Afficher les IDs des textures des planètes
			for (size_t i = 0; i < planets.size(); i++) {
				ImGui::Text("Planet %zu Texture ID: %d", i, planets[i].mesh->getMaterial().diffuseMap);
			}
		}

		// Contrôles d'éclairage simplifiés
		static float ambientStrength = 0.3f;
		if (ImGui::SliderFloat("Ambient Strength", &ambientStrength, 0.0f, 1.0f)) {
			GLint loc_ambient = glGetUniformLocation(basicProgram, "u_ambient_strength");
			glUniform1f(loc_ambient, ambientStrength);
		}

		RenderImGuiObjectControls();

		ImGui::End();
	}

	if (show_settings) {
		ImGui::SetNextWindowSize(ImVec2(width * 0.25f, height * 0.3f), ImGuiCond_FirstUseEver);
		ImGui::Begin("Light Settings", &show_settings, ImGuiWindowFlags_NoCollapse);
		ImGui::ColorEdit3("Light Color", light_color);
		ImGui::SliderFloat("Light Intensity", &light_intensity, 0.0f, 10.0f);
		ImGui::End();
	}

	// Juste avant ImGui, désactive sRGB pour éviter la double correction gamma sur l'UI
	glDisable(GL_FRAMEBUFFER_SRGB);

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Réactive sRGB pour la prochaine frame (optionnel, sécurité)
    glEnable(GL_FRAMEBUFFER_SRGB);
}

int main()
{
    // Initialisation de GLFW
    if (!glfwInit()) {
        std::cerr << "Erreur : Impossible d'initialiser GLFW" << std::endl;
        return -1;
    }

    // Initialisation du shader et de la fenêtre
    if (!Initialise()) {
        std::cerr << "Erreur : Impossible d'initialiser les shaders" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Initialisation du temps
    last_frame_time = glfwGetTime();

    // Boucle de rendu
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