#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>

#include <iostream>
#include <vector>
#include <cmath>
#include <filesystem>
#include "GLShader.h"
#include "MatrixUtils.h"
#include "dragonData.h"
#include "Mesh.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"

// VBO : buffer les valeurs des paramètres des vertexes
// IBO : buffer les vertexes pour ne pas redéssiner plusieurs fois les mêmes
// VAO : buffer les paramètres des vertexes (le mapping)
GLuint VBO, IBO, VAO;
GLuint textureID;
GLShader g_BasicShader;
GLFWwindow* g_Window = nullptr; // Variable globale pour la fenêtre
int width = 1200, height = 900; // taille de la fenêtre

// matrice de scale *2
float scale[16] = {
	0.5f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.5f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.5f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f};

// Matrice de rotation 45° sur l'axe Z
float radians = 0 * (3.14159f / 180.0f);
float rotation[16] = {
	cos(radians), -sin(radians), 0.0f, 0.0f,
	sin(radians), cos(radians), 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f};

// matrice de translation sur l'axe X
float translation[16] = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, -10.0f, 1.0f};

float fov = 60 * (3.14159f / 180.0f); // angle d'ouverture de 60°
float cam_far = 100.0f;
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

// Fonction de callback pour la souris
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
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
    // Modifier la gestion de la touche Escape
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
        cursor_locked = !cursor_locked;
        glfwSetInputMode(window, GLFW_CURSOR, 
            cursor_locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        // Attendre un peu pour éviter le rebond
        glfwWaitEvents();
    }

    // Ne traiter les inputs de caméra que si le curseur est verrouillé
    if (!cursor_locked || ImGui::GetIO().WantCaptureMouse) {
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

bool Initialise()
{
    // Déplacer la création de la fenêtre depuis main()
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

    // Initialisation de GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Erreur : Impossible d'initialiser GLEW" << std::endl;
        return false;
    }

	g_BasicShader.LoadVertexShader("Basic.vs");
	g_BasicShader.LoadFragmentShader("Basic.fs");
	g_BasicShader.Create();


	//Texture
	int texWidth, texHeight, texChannels;
	unsigned char* data = stbi_load("dragon.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	if (data) {
		std::cout << "Texture chargée avec succès : " << texWidth << "x" << texHeight 
				  << " (" << texChannels << " canaux)" << std::endl;
				  
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// Configuration basique
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Upload des données dans OpenGL
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);
		glBindTexture(GL_TEXTURE_2D, 0); // unbind proprement
	} else {
		std::cerr << "Erreur : impossible de charger la texture PNG\n";
		std::cerr << "Erreur stbi : " << stbi_failure_reason() << std::endl;
		std::cerr << "Chemin tenté : " << std::filesystem::absolute("dragon.png").string() << std::endl;
	}

	// création du VAO
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// VBO
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(DragonVertices), DragonVertices, GL_STATIC_DRAW);
	//glBindBuffer(GL_ARRAY_BUFFER, 0); // reinitialisation les etats a la fin pour eviter les effets de bord

	// IBO
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(DragonIndices), DragonIndices, GL_STATIC_DRAW);

	// Paramètrage de la forme
	auto basicProgram = g_BasicShader.GetProgram();
	glUseProgram(basicProgram);

	int loc_position = glGetAttribLocation(basicProgram, "a_position");
	int loc_couleur = glGetAttribLocation(basicProgram, "a_normal");
	int loc_uv = glGetAttribLocation(basicProgram, "a_uv");

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(loc_position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));
	glEnableVertexAttribArray(loc_position);
	glVertexAttribPointer(loc_couleur, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));
	glEnableVertexAttribArray(loc_couleur);
	glVertexAttribPointer(loc_uv, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(loc_uv);

	// Création du soleil
	sun = new Mesh();
	sun->createSphere(1.0f, 32, 32);
	sun->setScale(5.0f, 5.0f, 5.0f);  // Soleil plus grand
	sun->setPosition(0.0f, 10.0f, -15.0f);  // Position ajustée pour être visible

	// Définir le matériau du soleil pour qu'il soit émissif
	Material sunMaterial;
	sunMaterial.diffuse[0] = 10.0f;    // Jaune plus intense
	sunMaterial.diffuse[1] = 8.0f;     
	sunMaterial.diffuse[2] = 3.0f;     // Ajout d'un peu de rouge pour un effet plus chaud
	sunMaterial.specular[0] = 0.0f;    
	sunMaterial.specular[1] = 0.0f;
	sunMaterial.specular[2] = 0.0f;
	sunMaterial.shininess = 0.0f;
	sunMaterial.isEmissive = true;
	sun->setMaterial(sunMaterial);
	
	sceneObjects.push_back(sun);

	// Remplacer la création manuelle du dragon par le chargement du fichier OBJ
	// Mesh* dragon = new Mesh();
	// if (!dragon->loadFromOBJFile("models/dragon.obj")) {
	// 	std::cerr << "Erreur : impossible de charger le modèle dragon.obj\n";
	// 	delete dragon;
	// 	return false;
	// }
	// dragon->setScale(0.5f, 0.5f, 0.5f);
	// dragon->setPosition(0.0f, 0.0f, 0.0f);
	// sceneObjects.push_back(dragon);

	// Initialisation d'ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(g_Window, true);
	ImGui_ImplOpenGL3_Init("#version 130");

	return true;
}
void Terminate()
{
	g_BasicShader.Destroy();
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &IBO);
	glDeleteVertexArrays(1, &VAO);

	// Nettoyer les objets de la scène
	for(Mesh* obj : sceneObjects) {
		delete obj;
	}
	sceneObjects.clear();

	// Cleanup ImGui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
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

	// clean des couleurs
	glClearColor(0.5f, 0.5f, 0.5f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto basicProgram = g_BasicShader.GetProgram();
	glUseProgram(basicProgram);

	int loc_texture = glGetUniformLocation(basicProgram, "u_texture");
	glUniform1i(loc_texture, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Debug de la texture
	GLint boundTexture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTexture);
	if (boundTexture != textureID) {
		std::cout << "Warning: Texture non liée correctement" << std::endl;
	}

	// Combine matrices into one transformation matrix
	float transform[16];
	combineTRS(translation, rotation, scale, transform);
	GLint loc_transform = glGetUniformLocation(basicProgram, "u_transform");
	glUniformMatrix4fv(loc_transform, 1, GL_FALSE, transform);

	// Create and pass the perspective projection matrix.
	float projection[16];
	createPerspectiveMatrix(fov, (float)width / height, cam_near, cam_far, projection);
	GLint loc_projection = glGetUniformLocation(basicProgram, "u_projection");
	glUniformMatrix4fv(loc_projection, 1, GL_FALSE, projection);

	// Créer la matrice de vue
	float view[16];
	float center[3] = {
		camera.position[0] + camera.front[0],
		camera.position[1] + camera.front[1],
		camera.position[2] + camera.front[2]
	};
	createLookAtMatrix(camera.position, center, camera.up, view);

	// Passer la matrice de vue au shader
	GLint loc_view = glGetUniformLocation(basicProgram, "u_view");
	glUniformMatrix4fv(loc_view, 1, GL_FALSE, view);

	// Configuration de la lumière
	const float* sunPos = sun->getPosition();
	float lightDir[3] = {
		sunPos[0],
		sunPos[1],
		sunPos[2]
	};
	float lightDiffuse[3] = {
		light_color[0] * light_intensity,
		light_color[1] * light_intensity,
		light_color[2] * light_intensity
	};
	float lightSpecular[3] = {
		light_color[0] * light_intensity * 0.5f,
		light_color[1] * light_intensity * 0.5f,
		light_color[2] * light_intensity * 0.5f
	};
    
    GLint loc_lightDir = glGetUniformLocation(basicProgram, "u_light.direction");
    GLint loc_lightDiffuse = glGetUniformLocation(basicProgram, "u_light.diffuseColor");
    GLint loc_lightSpecular = glGetUniformLocation(basicProgram, "u_light.specularColor");
    GLint loc_intensity = glGetUniformLocation(basicProgram, "u_intensity");
    
    glUniform1f(loc_intensity, light_intensity);
    glUniform3fv(loc_lightDir, 1, lightDir);
    glUniform3fv(loc_lightDiffuse, 1, lightDiffuse);
    glUniform3fv(loc_lightSpecular, 1, lightSpecular);

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

	//camera position
	GLint loc_viewPos = glGetUniformLocation(basicProgram, "u_viewPos");
	glUniform3fv(loc_viewPos, 1, camera.position); // Utiliser la vraie position de la caméra

	// Ajout pour les objets normaux
	GLint loc_isEmissive = glGetUniformLocation(basicProgram, "u_material.isEmissive");
	glUniform1i(loc_isEmissive, 0);  // Pour les objets normaux

	// Récupération du buffer VAO
	glBindVertexArray(VAO);

	// dessin
	glDrawElements(GL_TRIANGLES,  _countof(DragonIndices), GL_UNSIGNED_SHORT, 0);

	// Dessiner tous les objets de la scène
	for(Mesh* obj : sceneObjects) {
		obj->draw(g_BasicShader);
	}

	// Début du rendu ImGui
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Fenêtre de debug
	if (show_debug_window) {
		ImGui::Begin("Debug", &show_debug_window);
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
			
			// Utilisation de la variable temporaire
			if (ImGui::SliderFloat3("Light Position", lightPos, -50.0f, 50.0f)) {
				sun->setPosition(lightPos[0], lightPos[1], lightPos[2]);
			}
			
			ImGui::ColorEdit3("Ambient Color", matDiffuse);
			ImGui::SliderFloat("Shininess", &matShininess, 1.0f, 256.0f);
		}

		// Chargement de modèles OBJ
		if (ImGui::CollapsingHeader("Model Loader")) {
			ImGui::InputText("Model Path", obj_path, sizeof(obj_path));
			if (ImGui::Button("Load Model")) {
				Mesh* new_model = new Mesh();
				if (new_model->loadFromOBJFile(obj_path)) {
					new_model->setScale(0.5f, 0.5f, 0.5f);
					sceneObjects.push_back(new_model);
				} else {
					delete new_model;
					ImGui::OpenPopup("Error");
				}
			}
			if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
				ImGui::Text("Failed to load model!");
				if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}
		}

		// Liste des objets dans la scène
		if (ImGui::CollapsingHeader("Scene Objects")) {
			for (size_t i = 0; i < sceneObjects.size(); i++) {
				char label[32];
				sprintf(label, "Object %zu", i);
				if (ImGui::TreeNode(label)) {
					float* pos = (float*)sceneObjects[i]->getPosition();
					ImGui::SliderFloat3("Position", pos, -50.0f, 50.0f);
					ImGui::TreePop();
				}
			}
		}

		// Ajout de debug pour la texture dans l'interface ImGui
		if (ImGui::CollapsingHeader("Texture Debug")) {
			ImGui::Text("Texture ID: %d", textureID);
			ImGui::Text("Texture Binding: %d", boundTexture);
			
			// Contrôles d'éclairage simplifiés
			static float ambientStrength = 0.3f;
			if (ImGui::SliderFloat("Ambient Strength", &ambientStrength, 0.0f, 1.0f)) {
				GLint loc_ambient = glGetUniformLocation(basicProgram, "u_ambient_strength");
				glUniform1f(loc_ambient, ambientStrength);
			}
			
			// Afficher les coordonnées UV du centre
			ImGui::Text("UV Test: %.2f, %.2f", 0.5f, 0.5f);
		}

		ImGui::End();
	}

	if (show_settings) {
		ImGui::Begin("Light Settings", &show_settings);
		ImGui::ColorEdit3("Light Color", light_color);
		ImGui::SliderFloat("Light Intensity", &light_intensity, 0.0f, 10.0f);
		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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