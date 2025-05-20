#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>
#include <string>
#include <GL/glew.h>
#include "GLShader.h"

struct Vertex {
    float position[3];
    float normal[3];
    float uv[2];
};

struct Material {
    float diffuse[3] = {0.8f, 0.8f, 0.8f};
    float specular[3] = {1.0f, 1.0f, 1.0f};
    float shininess = 32.0f;
    GLuint diffuseMap = 0;
    bool isEmissive = false;  // Ajout de la propriété isEmissive
};

class Mesh {
public:
    Mesh();
    ~Mesh();
    
    void setPosition(float x, float y, float z);
    void setRotation(float x, float y, float z);
    void setScale(float x, float y, float z);
    void setMaterial(const Material& mat);
    void createSphere(float radius, int sectors, int stacks);  // Ajout de cette méthode
    const float* getPosition() const;
    
    bool loadFromFile(const char* filename);
    bool loadTexture(const char* filename);
    void draw(GLShader& shader);

private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    Material material;
    
    GLuint VAO, VBO, EBO;
    float position[3] = {0.0f, 0.0f, 0.0f};
    float rotation[3] = {0.0f, 0.0f, 0.0f};
    float scale[3] = {1.0f, 1.0f, 1.0f};
    
    void setupMesh();
    void calculateModelMatrix(float* outMatrix);
};
