#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>

#include <iostream>
#include <vector>
#include <cmath>
#include "GLShader.h"
#include "MatrixUtils.h"
#include "dragonData.h"

struct Vertex {
	float position[3]; // x, y, z
	float normal[3]; // nx, ny, ny
	float uv[2]; // u, v

	// Constructeur avec valeurs individuelles
    Vertex(float px, float py, float pz,
		float nx, float ny, float nz,
		float u, float v) {
	 position[0] = px; position[1] = py; position[2] = pz;
	 normal[0] = nx; normal[1] = ny; normal[2] = nz;
	 uv[0] = u; uv[1] = v;
	}

	// Constructeur avec tableaux
	Vertex(const float pos[3], const float norm[3], const float tex[2]) {
		for (int i = 0; i < 3; ++i) {
			position[i] = pos[i];
			normal[i] = norm[i];
		}
		for (int i = 0; i < 2; ++i) {
			uv[i] = tex[i];
		}
	}

};

// VBO : buffer les valeurs des paramètres des vertexes
// IBO : buffer les vertexes pour ne pas redéssiner plusieurs fois les mêmes
// VAO : buffer les paramètres des vertexes (le mapping)
GLuint VBO, IBO, VAO;
GLuint textureID;
GLShader g_BasicShader;
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
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

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

bool Initialise()
{
	g_BasicShader.LoadVertexShader("Basic.vs");
	g_BasicShader.LoadFragmentShader("Basic.fs");
	g_BasicShader.Create();


	//Texture
	int texWidth, texHeight, texChannels;
	unsigned char* data = stbi_load("dragon.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	if (data) {
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// Configuration basique
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Upload des données dans OpenGL
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);
		glBindTexture(GL_TEXTURE_2D, 0); // unbind proprement
	} else {
		std::cerr << "Erreur : impossible de charger la texture PNG\n";
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

// cette fonction est spécifique à Windows et permet d'activer (1) ou non (0)
// la synchronization vertical. Elle necessite l'include wglew.h

	return true;
}
void Terminate()
{
	g_BasicShader.Destroy();
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &IBO);
	glDeleteVertexArrays(1, &VAO);
}

void Render()
{
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

	//conf lumière
	float lightDir[3] = {0.0f, -0.75f, 1.0f};
    float lightDiffuse[3] = {1.0f, 1.0f, 1.0f};
    float lightSpecular[3] = {1.0f, 0.0f, 0.0f};
    
    GLint loc_lightDir = glGetUniformLocation(basicProgram, "u_light.direction");
    GLint loc_lightDiffuse = glGetUniformLocation(basicProgram, "u_light.diffuseColor");
    GLint loc_lightSpecular = glGetUniformLocation(basicProgram, "u_light.specularColor");
    
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
	float viewPos[3] = {0.0f, 0.0f, 0.0f};
	GLint loc_viewPos = glGetUniformLocation(basicProgram, "u_viewPos");
	glUniform3fv(loc_viewPos, 1, viewPos);


	// Récupération du buffer VAO
	glBindVertexArray(VAO);

	// dessin
	glDrawElements(GL_TRIANGLES,  _countof(DragonIndices), GL_UNSIGNED_SHORT, 0);
}

int main()
{
	// Initialisation de GLFW
	if (!glfwInit())
	{
		std::cerr << "Erreur : Impossible d'initialiser GLFW" << std::endl;
		return -1;
	}

	// Cr�ation de la fenetre OpenGL
	GLFWwindow *window = glfwCreateWindow(width, height, "OpenGL Window", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "Erreur : Impossible de cr�er la fen�tre GLFW" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Ajouter les callbacks pour la souris
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Initialisation de GLEW
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Erreur : Impossible d'initialiser GLEW" << std::endl;
		return -1;
	}

	// Initialisation du shader
	if (!Initialise())
	{
		std::cerr << "Erreur : Impossible d'initialiser les shaders" << std::endl;
		return -1;
	}

	// Boucle de rendu
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);
		Render();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Nettoyage
	Terminate();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}