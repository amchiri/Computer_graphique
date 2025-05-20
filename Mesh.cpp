#include "Mesh.h"
#include "MatrixUtils.h"
#include <stb/stb_image.h>
#include "tiny_obj_loader.h"
#include <iostream>
#include <cmath>

Mesh::Mesh() : VAO(0), VBO(0), EBO(0) {
    position[0] = position[1] = position[2] = 0.0f;
    rotation = Mat4::identity();
    m_transform = Mat4::identity(); // Initialisation de m_transform
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

void Mesh::setRotation(const Mat4& rotationMatrix) {
    rotation = rotationMatrix;
}

void Mesh::setRotation(float x, float y, float z) {
    Mat4 rotX = Mat4::rotate(x, 1.0f, 0.0f, 0.0f);
    Mat4 rotY = Mat4::rotate(y, 0.0f, 1.0f, 0.0f);
    Mat4 rotZ = Mat4::rotate(z, 0.0f, 0.0f, 1.0f);
    rotation = rotX * rotY * rotZ;
}

void Mesh::setScale(float x, float y, float z) {
    scale[0] = x;
    scale[1] = y;
    scale[2] = z;
}

void Mesh::setMaterial(const Material& mat) {
    material = mat;
}

const Material& Mesh::getMaterial() const {
    return material;
}

const float* Mesh::getPosition() const {
    return position;
}

void Mesh::setTransform(const Mat4& transform) {
    m_transform = transform;
}

const Mat4& Mesh::getTransform() const {
    return m_transform;
}

void Mesh::calculateModelMatrix(float* outMatrix) {
    // Utiliser directement la matrice de transformation
    memcpy(outMatrix, m_transform.data(), 16 * sizeof(float));
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

bool Mesh::loadFromOBJFile(const char* filename) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename);

    if (!warn.empty()) {
        std::cout << "Warning: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << "Error: " << err << std::endl;
    }

    if (!ret) {
        return false;
    }

    // Clear existing data
    vertices.clear();
    indices.clear();

    // Pour chaque shape dans le fichier
    for (size_t s = 0; s < shapes.size(); s++) {
        size_t index_offset = 0;
        // Pour chaque face dans la shape
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            // Pour chaque vertex dans la face
            for (size_t v = 0; v < 3; v++) {
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                
                Vertex vertex;
                // Position
                vertex.position[0] = attrib.vertices[3 * idx.vertex_index + 0];
                vertex.position[1] = attrib.vertices[3 * idx.vertex_index + 1];
                vertex.position[2] = attrib.vertices[3 * idx.vertex_index + 2];
                
                // Normal
                if (idx.normal_index >= 0) {
                    vertex.normal[0] = attrib.normals[3 * idx.normal_index + 0];
                    vertex.normal[1] = attrib.normals[3 * idx.normal_index + 1];
                    vertex.normal[2] = attrib.normals[3 * idx.normal_index + 2];
                }
                
                // UV
                if (idx.texcoord_index >= 0) {
                    vertex.uv[0] = attrib.texcoords[2 * idx.texcoord_index + 0];
                    vertex.uv[1] = attrib.texcoords[2 * idx.texcoord_index + 1];
                }
                
                vertices.push_back(vertex);
                indices.push_back(indices.size());
            }
            index_offset += 3;
        }
    }

    setupMesh();
    return true;
}
