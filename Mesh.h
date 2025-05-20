#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>
#include <string>
#include <GL/glew.h>
#include "GLShader.h"
#include "Mat4.h"
#include "tiny_obj_loader.h"

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
    bool isEmissive = false;
};

class Mesh {
public:
    Mesh();
    ~Mesh();
    
    void setPosition(float x, float y, float z);
    void setRotation(const Mat4& rotationMatrix); // Nouveau
    void setRotation(float x, float y, float z); // Garde l'ancien pour compatibilité
    void setScale(float x, float y, float z);
    const Material& getMaterial() const; // Nouvelle méthode
    void setMaterial(const Material& mat);
    void createSphere(float radius, int sectors, int stacks);
    const float* getPosition() const;
    void setTransform(const Mat4& transform);
    const Mat4& getTransform() const;
    
    bool loadFromFile(const char* filename);
    bool loadTexture(const char* filename);
    bool loadFromOBJFile(const char* filename);
    void draw(GLShader& shader);

private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    Material material;
    
    GLuint VAO, VBO, EBO;
    float position[3] = {0.0f, 0.0f, 0.0f};
    Mat4 rotation; // Changement ici
    float scale[3] = {1.0f, 1.0f, 1.0f};
    Mat4 m_transform;  // Nouvelle matrice de transformation complète
    
    void setupMesh();
    void calculateModelMatrix(float* outMatrix);
};
