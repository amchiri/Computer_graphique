#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>

#ifdef WIN32
#include <GL/wglew.h>
#endif

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
#ifdef WIN32
	wglSwapIntervalEXT(1);
#endif
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