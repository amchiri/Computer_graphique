#include "Mesh.h"
#include "MatrixUtils.h"
#include <stb/stb_image.h>
#include <iostream>
#include <cmath>

Mesh::Mesh() : VAO(0), VBO(0), EBO(0) {
    position[0] = position[1] = position[2] = 0.0f;
    rotation[0] = rotation[1] = rotation[2] = 0.0f;
    scale[0] = scale[1] = scale[2] = 1.0f;
}

Mesh::~Mesh() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
    if (material.diffuseMap) glDeleteTextures(1, &material.diffuseMap);
}

bool Mesh::loadTexture(const char* filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 4);
    if (!data) return false;

    glGenTextures(1, &material.diffuseMap);
    glBindTexture(GL_TEXTURE_2D, material.diffuseMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    stbi_image_free(data);
    return true;
}

void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
}

void Mesh::draw(GLShader& shader) {
    auto program = shader.GetProgram();
    glUseProgram(program);

    // Calculer et appliquer la matrice de modèle
    float modelMatrix[16];
    calculateModelMatrix(modelMatrix);
    GLint loc_transform = glGetUniformLocation(program, "u_transform");
    glUniformMatrix4fv(loc_transform, 1, GL_FALSE, modelMatrix);

    // Appliquer le matériau
    GLint loc_matDiffuse = glGetUniformLocation(program, "u_material.diffuseColor");
    GLint loc_matSpecular = glGetUniformLocation(program, "u_material.specularColor");
    GLint loc_matShininess = glGetUniformLocation(program, "u_material.shininess");
    
    glUniform3fv(loc_matDiffuse, 1, material.diffuse);
    glUniform3fv(loc_matSpecular, 1, material.specular);
    glUniform1f(loc_matShininess, material.shininess);

    // Activer la texture si elle existe
    if (material.diffuseMap) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, material.diffuseMap);
        GLint loc_texture = glGetUniformLocation(program, "u_texture");
        glUniform1i(loc_texture, 0);
    }

    // Gestion de l'état émissif
    GLint loc_isEmissive = glGetUniformLocation(program, "u_material.isEmissive");
    glUniform1i(loc_isEmissive, material.isEmissive ? 1 : 0);

    // Dessiner la géométrie
    if (VAO) {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

void Mesh::setPosition(float x, float y, float z) {
    position[0] = x;
    position[1] = y;
    position[2] = z;
}

void Mesh::setRotation(float x, float y, float z) {
    rotation[0] = x;
    rotation[1] = y;
    rotation[2] = z;
}

void Mesh::setScale(float x, float y, float z) {
    scale[0] = x;
    scale[1] = y;
    scale[2] = z;
}

void Mesh::setMaterial(const Material& mat) {
    material = mat;
}

const float* Mesh::getPosition() const {
    return position;
}

void Mesh::calculateModelMatrix(float* outMatrix) {
    // Créer les matrices de translation, rotation et échelle
    float translationMatrix[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        position[0], position[1], position[2], 1.0f
    };

    float cosX = static_cast<float>(cos(rotation[0]));
    float sinX = static_cast<float>(sin(rotation[0]));
    float rotationMatrixX[16] = {
        1.0f, 0.0f,  0.0f, 0.0f,
        0.0f, cosX, -sinX, 0.0f,
        0.0f, sinX,  cosX, 0.0f,
        0.0f, 0.0f,  0.0f, 1.0f
    };

    float cosY = static_cast<float>(cos(rotation[1]));
    float sinY = static_cast<float>(sin(rotation[1]));
    float rotationMatrixY[16] = {
        cosY, 0.0f, sinY, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        -sinY, 0.0f, cosY, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    float cosZ = static_cast<float>(cos(rotation[2]));
    float sinZ = static_cast<float>(sin(rotation[2]));
    float rotationMatrixZ[16] = {
        cosZ, -sinZ, 0.0f, 0.0f,
        sinZ,  cosZ, 0.0f, 0.0f,
        0.0f,  0.0f, 1.0f, 0.0f,
        0.0f,  0.0f, 0.0f, 1.0f
    };

    float scaleMatrix[16] = {
        scale[0], 0.0f, 0.0f, 0.0f,
        0.0f, scale[1], 0.0f, 0.0f,
        0.0f, 0.0f, scale[2], 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    // Combiner les matrices
    float temp[16];
    multiplyMatrix4x4(rotationMatrixX, scaleMatrix, temp);
    float temp2[16];
    multiplyMatrix4x4(rotationMatrixY, temp, temp2);
    float temp3[16];
    multiplyMatrix4x4(rotationMatrixZ, temp2, temp3);
    multiplyMatrix4x4(translationMatrix, temp3, outMatrix);
}

void Mesh::createSphere(float radius, int sectors, int stacks) {
    vertices.clear();
    indices.clear();

    float sectorStep = static_cast<float>(2.0 * M_PI / sectors);
    float stackStep = static_cast<float>(M_PI / stacks);

    // Générer les vertices
    for(int i = 0; i <= stacks; ++i) {
        float stackAngle = static_cast<float>(M_PI / 2 - i * stackStep);
        float xy = radius * std::cos(stackAngle);
        float z = radius * std::sin(stackAngle);

        for(int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * sectorStep;

            // Position
            float x = xy * std::cos(sectorAngle);
            float y = xy * std::sin(sectorAngle);
            
            // Normal
            float nx = x / radius;
            float ny = y / radius;
            float nz = z / radius;

            // UV
            float u = static_cast<float>(j) / sectors;
            float v = static_cast<float>(i) / stacks;

            Vertex vertex;
            vertex.position[0] = x;
            vertex.position[1] = y;
            vertex.position[2] = z;
            vertex.normal[0] = nx;
            vertex.normal[1] = ny;
            vertex.normal[2] = nz;
            vertex.uv[0] = u;
            vertex.uv[1] = v;
            vertices.push_back(vertex);
        }
    }

    // Générer les indices
    for(int i = 0; i < stacks; ++i) {
        for(int j = 0; j < sectors; ++j) {
            int k1 = i * (sectors + 1) + j;
            int k2 = k1 + 1;
            int k3 = k1 + (sectors + 1);
            int k4 = k3 + 1;

            indices.push_back(k1);
            indices.push_back(k2);
            indices.push_back(k3);

            indices.push_back(k2);
            indices.push_back(k4);
            indices.push_back(k3);
        }
    }

    setupMesh();
}
