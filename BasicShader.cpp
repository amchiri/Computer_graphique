#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <GL/glew.h> // GLEW doit être inclus en premier
#include <GLFW/glfw3.h>

#include <algorithm>  // Ajouter cette ligne pour std::find_if

#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <filesystem>
#include "GLShader.h"
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
#include "UI.h"
#include "Skybox.h" // Ajouter en haut du fichier
#include "CameraController.h"

// Déclaration globale des variables
std::unique_ptr<UI> g_UI;
std::unique_ptr<Skybox> g_Skybox;
std::unique_ptr<CameraController> g_Camera;
std::vector<Mesh*> sceneObjects;  // Moved to top
Mesh* sun = nullptr;  // Moved to top and initialized

// Variables globales de la skybox
GLuint skyboxVAO, skyboxVBO;
GLuint skyboxTexture;
GLShader g_SkyboxShader;

GLShader g_BasicShader;
GLShader g_ColorShader;
GLShader g_EnvMapShader;
GLFWwindow* g_Window = nullptr; // Variable globale pour la fenêtre
int width = 1200, height = 900; // taille de la fenêtre

float fov = 60 * (3.14159f / 180.0f); // angle d'ouverture de 60°
float cam_far = 1000.0f;  // Augmenter cette valeur (était 100.0f)
float cam_near = 0.1f;

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
float elapsed_time = 0.0f;
float last_frame_time = 0.0f;

// Variables globales ajoutées
bool camera_enabled = true;
char selected_file[256] = "";
char selected_texture[256] = "";
int selected_object = -1;

std::vector<Planet> planets;

// Structure pour les transformations des objets
struct ObjectTransform {
    float position[3] = {0.0f, 0.0f, 0.0f};
    float rotation[3] = {0.0f, 0.0f, 0.0f};
    float scale[3] = {1.0f, 1.0f, 1.0f};
};

// Map globale pour stocker les transformations des objets
std::map<Mesh*, ObjectTransform> objectTransforms;

// Map pour le shader choisi par objet
std::map<Mesh*, GLShader*> objectShaders;

// Déclarations des prototypes de fonctions
void createPlanets();
void loadPlanetTextures();
void updatePlanets();
void initializeSolarSystem();
void createSkybox();
void AdjustInitialCamera();
void createDemoObjects();

// Déclarer les prototypes des fonctions utilisées dans RenderImGuiObjectControls
std::string OpenFileDialog(const char* filter = "OBJ files\0*.obj\0All files\0*.*\0");
bool LoadModelWithTexture(const std::string& modelPath);

// Fonction de callback pour la souris
// Supprimer la fonction mouse_callback et la remplacer par la gestion dans CameraController

// Supprimer la fonction processInput et la remplacer par :
void processInput(GLFWwindow* window) {
    if (g_Camera) {
        g_Camera->Update(elapsed_time);
    }
}

// Dans la fonction Render(), remplacer les références à camera par g_Camera :
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

	// Créer les matrices communes
	Mat4 projectionMatrix = Mat4::perspective(fov, (float)width / height, cam_near, cam_far);
	
	// Get camera data safely
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

	// Rendu des objets avec leur shader sélectionné
	for (Mesh* obj : sceneObjects) {
		// Déterminer quel shader utiliser
		GLShader* shader = obj->getCurrentShader();
		if (!shader) {
			shader = &g_BasicShader; // Shader par défaut
			obj->setCurrentShader(shader);
		}

		GLuint program = shader->GetProgram();
		glUseProgram(program);

		// Passer les matrices communes à tous les shaders
		GLint loc_proj = glGetUniformLocation(program, "u_projection");
		if (loc_proj >= 0) glUniformMatrix4fv(loc_proj, 1, GL_FALSE, projectionMatrix.data());
		
		GLint loc_view = glGetUniformLocation(program, "u_view");
		if (loc_view >= 0) glUniformMatrix4fv(loc_view, 1, GL_FALSE, viewMatrix.data());

		// Passer la matrice de transformation de l'objet
		float modelMatrix[16];
		obj->calculateModelMatrix(modelMatrix);
		GLint loc_transform = glGetUniformLocation(program, "u_transform");
		if (loc_transform >= 0) glUniformMatrix4fv(loc_transform, 1, GL_FALSE, modelMatrix);

		// Configuration spécifique selon le shader
		if (shader == &g_ColorShader) {
			// Pour le shader couleur : passer uniquement la couleur
			GLint loc_color = glGetUniformLocation(program, "u_color");
			if (loc_color >= 0) {
				const Material& mat = obj->getMaterial();
				glUniform3fv(loc_color, 1, mat.diffuse);
			}
		}
		else if (shader == &g_BasicShader) {
			// Pour le shader de base : configuration complète de l'éclairage
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

			// Configuration du matériau
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

			// Gestion des objets émissifs
			GLint loc_isEmissive = glGetUniformLocation(program, "u_material.isEmissive");
			if (loc_isEmissive >= 0) {
				bool isEmissive = (obj == sun);
				glUniform1i(loc_isEmissive, isEmissive ? 1 : 0);
			}
		}
		else if (shader == &g_EnvMapShader) {
			// Pour le shader environment mapping
            GLint loc_viewPos = glGetUniformLocation(program, "u_viewPos");
            if (loc_viewPos >= 0) glUniform3fv(loc_viewPos, 1, g_Camera->GetPosition());
            
            // Si vous avez une cubemap, l'activer ici
            // GLint loc_envmap = glGetUniformLocation(program, "u_envmap");
            // if (loc_envmap >= 0) glUniform1i(loc_envmap, 0);
        }

		// Dessiner l'objet
		obj->draw(*shader);
	}

	// Rendu de la skybox en dernier
	if (g_Skybox) {
        g_Skybox->Draw(viewMatrix, projectionMatrix);
    }

	// Remplacer tout le code ImGui par:
    glDisable(GL_FRAMEBUFFER_SRGB);
    g_UI->RenderUI(fps, g_Camera->GetPosition(), g_Camera->GetFront());
    glEnable(GL_FRAMEBUFFER_SRGB);
}

void createDemoObjects() {
    // Cube couleur simple
    Mesh* colorCube = new Mesh();
    colorCube->createSphere(1.0f, 32, 32);  // ou créer un cube si vous avez la fonction
    Material matColor;
    matColor.diffuse[0] = 0.2f; matColor.diffuse[1] = 0.8f; matColor.diffuse[2] = 0.2f;
    matColor.specular[0] = matColor.specular[1] = matColor.specular[2] = 0.0f;
    matColor.shininess = 1.0f;
    matColor.isEmissive = false;
    colorCube->setMaterial(matColor);
    colorCube->setPosition(-20, 5, 0);
    sceneObjects.push_back(colorCube);
    objectShaders[colorCube] = &g_ColorShader;

    // Cube texturé
    Mesh* texCube = new Mesh();
    texCube->createSphere(1.0f, 32, 32);  // ou créer un cube
    texCube->loadTexture("models/earth.png");
    Material matTex;
    matTex.diffuse[0] = matTex.diffuse[1] = matTex.diffuse[2] = 1.0f;
    matTex.specular[0] = matTex.specular[1] = matTex.specular[2] = 0.5f;
    matTex.shininess = 16.0f;
    matTex.isEmissive = false;
    texCube->setMaterial(matTex);
    texCube->setPosition(0, 5, 0);
    sceneObjects.push_back(texCube);
    objectShaders[texCube] = &g_BasicShader;

    // Cube envmap
    Mesh* envCube = new Mesh();
    envCube->createSphere(1.0f, 32, 32);  // ou créer un cube
    Material matEnv;
    matEnv.diffuse[0] = matEnv.diffuse[1] = matEnv.diffuse[2] = 1.0f;
    matEnv.specular[0] = matEnv.specular[1] = matEnv.specular[2] = 1.0f;
    matEnv.shininess = 64.0f;
    matEnv.isEmissive = false;
    envCube->setMaterial(matEnv);
    envCube->setPosition(20, 5, 0);
    sceneObjects.push_back(envCube);
    objectShaders[envCube] = &g_EnvMapShader;
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

    // Initialize camera first
    g_Camera = std::make_unique<CameraController>(g_Window);
    if (!g_Camera) {
        std::cerr << "Failed to create camera controller" << std::endl;
        return false;
    }

    // Set initial camera position and settings
    g_Camera->SetPosition(0.0f, 50.0f, 100.0f);
    g_Camera->SetMovementSpeed(0.05f);
    g_Camera->SetSensitivity(0.1f);
    g_Camera->SetRotation(-30.0f, -90.0f);
    g_Camera->Initialize(); // Nouveau : initialise les callbacks de la souris

    // Supprimer ces lignes car maintenant gérées par la caméra
    // glfwSetCursorPosCallback(g_Window, mouse_callback);
    // glfwSetInputMode(g_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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

    // Ajoute l'initialisation des shaders supplémentaires
    g_ColorShader.LoadVertexShader("Color.vs");
    g_ColorShader.LoadFragmentShader("Color.fs");
    g_ColorShader.Create();

    g_EnvMapShader.LoadVertexShader("EnvMap.vs");
    g_EnvMapShader.LoadFragmentShader("EnvMap.fs");
    g_EnvMapShader.Create();

    // Création de la skybox avant l'UI
    createSkybox();  // Déplacer cette ligne ici

	// Création du soleil et des planètes
	initializeSolarSystem();
    createDemoObjects();

    // Create the UI instance and initialize it
    g_UI = std::make_unique<UI>(g_Window, width, height);
    g_UI->Initialize();
    g_UI->SetSceneObjects(sceneObjects, sun, planets);
    g_UI->SetShaders(&g_BasicShader, &g_ColorShader, &g_EnvMapShader);

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
    if (g_Camera) {
        g_Camera->SetPosition(0.0f, 50.0f, 100.0f);
        g_Camera->SetRotation(-30.0f, -90.0f); // Add this method to CameraController
    }
}

void Terminate()
{
    g_BasicShader.Destroy();

    // Nettoyer les objets de la scène
    for(Mesh* obj : sceneObjects) {
        delete obj;
    }
    sceneObjects.clear();

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

    g_UI.reset();
    g_Skybox.reset(); // Remplacer le nettoyage de la skybox par
    g_Camera.reset(); // Ajouté ici pour libérer la caméra
}

void loadPlanetTextures() {

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
                std::cout << "Texture loaded successfully for planet " << i << std::endl;
                planet.texture = planet.mesh->getMaterial().diffuseMap;
            } else {
                std::cerr << "Error: Could not load texture for planet " << i << std::endl;
            }
        }
    }
}

// Add main() declaration before implementation
int main(int argc, char* argv[]);

// Add WinMain for Windows builds
#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    return main(__argc, __argv);
}
#endif

// Change main implementation to include parameters
int main(int argc, char* argv[]) {
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
        g_Camera->Update(elapsed_time);  // Cette fonction gère maintenant tout l'input
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