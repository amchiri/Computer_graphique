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
    float ambient[3] = {0.2f, 0.2f, 0.2f};
    float shininess = 32.0f;
    GLuint diffuseMap = 0;
    bool isEmissive = false;
    float emissiveIntensity = 1.0f;
    float lightColor[3] = {1.0f, 1.0f, 1.0f};
    float specularStrength = 0.5f;
    bool useTextureInColorShader = false;
    bool useTextureInEnvMapShader = false;
    bool useTextureInBasicShader = true;
    bool ignoreObjectMaterialInEnvMap = false; // Nouvelle propriété pour ignorer le matériau

    enum class IlluminationModel {
        LAMBERT = 0,
        PHONG = 1,
        BLINN_PHONG = 2
    } illuminationModel = IlluminationModel::BLINN_PHONG;
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

    // Ces méthodes doivent être publiques (et une seule déclaration !)
    void calculateModelMatrix(float* outMatrix);
    bool loadTexture(const char* filename);
    void removeTexture(); // Nouvelle méthode pour supprimer la texture
    void unbindTexture();
    void bindTexture();

    // Ajoute ces deux méthodes publiques :
    bool loadFromOBJFile(const char* filename);
    void draw(GLShader& shader);

    const float* getScale() const { return scale; }
    void setCurrentShader(GLShader* shader) {
        m_CurrentShader = shader;
        updateShaderUniforms();  // Nouvelle méthode
    }
    GLShader* getCurrentShader() const { return m_CurrentShader; }

private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    Material material;
    bool textureEnabled = true;
    
    GLuint VAO, VBO, EBO;
    float position[3] = {0.0f, 0.0f, 0.0f};
    Mat4 rotation; // Changement ici
    float scale[3] = {1.0f, 1.0f, 1.0f};
    Mat4 m_transform;  // Nouvelle matrice de transformation complète
    GLShader* m_CurrentShader = nullptr;
    
    void setupMesh();
    void updateShaderUniforms();  // Nouvelle méthode pour mettre à jour les uniformes
    void calculateNormalsIfNeeded();  // Ajout de la déclaration de la fonction
};
