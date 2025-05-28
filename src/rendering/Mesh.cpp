#include "Mesh.h"
#include "Mat4.h"
#include <stb/stb_image.h>
#include "tiny_obj_loader.h"
#include <iostream>
#include <cmath>
#include <filesystem>
#include <unordered_map>

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
    if (!data) {
        std::cerr << "Erreur de chargement de la texture: " << filename << std::endl;
        std::cerr << "Raison: " << stbi_failure_reason() << std::endl;
        return false;
    }

    if (material.diffuseMap) {
        glDeleteTextures(1, &material.diffuseMap);
    }

    glGenTextures(1, &material.diffuseMap);
    glBindTexture(GL_TEXTURE_2D, material.diffuseMap);

    // Utilise GL_SRGB8_ALPHA8 pour les textures couleur (sRGB aware)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);  // Unbind la texture
    
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

    // Indiquer au shader si l'objet a une texture
    GLint loc_hasTexture = glGetUniformLocation(program, "u_hasTexture");
    glUniform1i(loc_hasTexture, material.diffuseMap != 0);

    // Activer la texture seulement si elle existe
    glActiveTexture(GL_TEXTURE0);
    if (material.diffuseMap) {
        glBindTexture(GL_TEXTURE_2D, material.diffuseMap);
        GLint loc_texture = glGetUniformLocation(program, "u_texture");
        glUniform1i(loc_texture, 0);
    } else {
        glBindTexture(GL_TEXTURE_2D, 0);  // Unbind toute texture
    }

    // Gestion de l'état émissif
    GLint loc_isEmissive = glGetUniformLocation(program, "u_material.isEmissive");
    glUniform1i(loc_isEmissive, material.isEmissive ? 1 : 0);

    // Choix du modèle d'illumination
    GLint loc_illumModel = glGetUniformLocation(program, "u_material.illuminationModel");
    if (loc_illumModel >= 0) {
        glUniform1i(loc_illumModel, static_cast<int>(material.illuminationModel));
    }

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

void Mesh::updateShaderUniforms() {
    if (!m_CurrentShader) return;

    m_CurrentShader->Use();
    
    // Mise à jour des uniformes du matériau
    m_CurrentShader->SetVec3("u_material.diffuseColor", material.diffuse);
    m_CurrentShader->SetVec3("u_material.specularColor", material.specular);
    m_CurrentShader->SetFloat("u_material.shininess", material.shininess);
    m_CurrentShader->SetBool("u_material.isEmissive", material.isEmissive);

    // Ces uniformes sont communs à tous les shaders
    m_CurrentShader->SetFloat("u_intensity", 1.0f);
}

void Mesh::setMaterial(const Material& mat) {
    material = mat;
    updateShaderUniforms();  // Mettre à jour les uniformes immédiatement
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
    Mat4 model = Mat4::identity();
    
    // Appliquer d'abord l'échelle
    model = model * Mat4::scale(scale[0], scale[1], scale[2]);
    
    // Ensuite la rotation
    model = model * rotation;
    
    // Finalement la translation
    model = model * Mat4::translate(position[0], position[1], position[2]);
    
    // Si une transformation personnalisée est définie, l'appliquer
    if (m_transform != Mat4::identity()) {
        model = m_transform;
    }
    
    memcpy(outMatrix, model.data(), 16 * sizeof(float));
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
    std::cout << "\n=== Loading OBJ: " << filename << " ===" << std::endl;
    
    // Reset complet du matériau et de la texture
    material = Material();

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    
    std::string baseDir = std::filesystem::path(filename).parent_path().string();
    std::cout << "Base directory: " << baseDir << std::endl;
    
    bool ret = tinyobj::LoadObj(
        &attrib, &shapes, &materials, &warn, &err,
        filename, baseDir.c_str(),
        true  // triangulate
    );
    
    if (!ret || shapes.empty()) {
        std::cerr << "Failed to load OBJ file: " << filename << std::endl;
        if (!err.empty()) std::cerr << err << std::endl;
        return false;
    }
    
    if (!warn.empty()) {
        std::cout << "OBJ loading warnings: " << warn << std::endl;
    }
    
    vertices.clear();
    indices.clear();
    
    // Map pour éviter les vertices dupliqués
    std::map<std::tuple<int, int, int>, unsigned int> vertexMap;
    unsigned int currentIndex = 0;
    
    // Parcourir tous les matériaux pour trouver celui avec une texture
    bool textureFound = false;
    if (!materials.empty()) {
        for (const auto& mat : materials) {
            if (!mat.diffuse_texname.empty()) {
                std::cout << "Found material with texture: " << mat.diffuse_texname << std::endl;
                
                // Construire le chemin en utilisant std::filesystem
                std::filesystem::path texPath = std::filesystem::path(baseDir) / mat.diffuse_texname;
                std::string texPathStr = texPath.string();
                
                if (std::filesystem::exists(texPath)) {
                    if (loadTexture(texPathStr.c_str())) {
                        textureFound = true;
                        material.diffuse[0] = mat.diffuse[0];
                        material.diffuse[1] = mat.diffuse[1];
                        material.diffuse[2] = mat.diffuse[2];
                        material.specular[0] = mat.specular[0];
                        material.specular[1] = mat.specular[1];
                        material.specular[2] = mat.specular[2];
                        material.shininess = mat.shininess;
                        break;
                    }
                } else {
                    std::vector<std::string> possiblePaths = {
                        (std::filesystem::path(filename).parent_path() / mat.diffuse_texname).string(),
                        mat.diffuse_texname,
                        (std::filesystem::path(baseDir) / std::filesystem::path(mat.diffuse_texname).filename()).string()
                    };
                    
                    for (const auto& path : possiblePaths) {
                        std::cout << "Trying path: " << path << std::endl;
                        if (std::filesystem::exists(path)) {
                            if (loadTexture(path.c_str())) {
                                textureFound = true;
                                break;
                            }
                        }
                    }
                    
                    if (textureFound) break;
                }
            }
        }
        
        // Si aucune texture n'a été trouvée, utiliser le premier matériau
        if (!textureFound && !materials.empty()) {
            const auto& mat = materials[0];
            material.diffuse[0] = mat.diffuse[0];
            material.diffuse[1] = mat.diffuse[1];
            material.diffuse[2] = mat.diffuse[2];
            material.specular[0] = mat.specular[0];
            material.specular[1] = mat.specular[1];
            material.specular[2] = mat.specular[2];
            material.shininess = mat.shininess;
        }
    }

    // Pour chaque forme dans le fichier
    for (size_t s = 0; s < shapes.size(); s++) {
        size_t index_offset = 0;
        
        // Pour chaque face de la forme
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            int fv = shapes[s].mesh.num_face_vertices[f];
            
            // Pour chaque vertex dans la face
            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                
                // Créer une clé unique pour ce vertex basée sur ses indices
                auto key = std::make_tuple(idx.vertex_index, idx.normal_index, idx.texcoord_index);
                
                // Vérifier si ce vertex existe déjà
                auto it = vertexMap.find(key);
                if (it != vertexMap.end()) {
                    // Vertex existe déjà, utiliser son index
                    indices.push_back(it->second);
                } else {
                    // Nouveau vertex, le créer
                    Vertex vertex;
                    
                    // Position
                    if (idx.vertex_index >= 0 && idx.vertex_index < static_cast<int>(attrib.vertices.size() / 3)) {
                        vertex.position[0] = attrib.vertices[3 * idx.vertex_index + 0];
                        vertex.position[1] = attrib.vertices[3 * idx.vertex_index + 1];
                        vertex.position[2] = attrib.vertices[3 * idx.vertex_index + 2];
                    } else {
                        vertex.position[0] = vertex.position[1] = vertex.position[2] = 0.0f;
                    }
                    
                    // Normal
                    if (idx.normal_index >= 0 && idx.normal_index < static_cast<int>(attrib.normals.size() / 3)) {
                        vertex.normal[0] = attrib.normals[3 * idx.normal_index + 0];
                        vertex.normal[1] = attrib.normals[3 * idx.normal_index + 1];
                        vertex.normal[2] = attrib.normals[3 * idx.normal_index + 2];
                    } else {
                        // Si pas de normale, on la calculera plus tard ou on met une valeur par défaut
                        vertex.normal[0] = 0.0f;
                        vertex.normal[1] = 0.0f;
                        vertex.normal[2] = 1.0f;  // Normal pointant vers Z+
                    }
                    
                    // Texture coordinates - CORRECTION IMPORTANTE
                    if (idx.texcoord_index >= 0 && idx.texcoord_index < static_cast<int>(attrib.texcoords.size() / 2)) {
                        vertex.uv[0] = attrib.texcoords[2 * idx.texcoord_index + 0];
                        // INVERSION DE LA COORDONNÉE V POUR CORRIGER L'ORIENTATION
                        vertex.uv[1] = 1.0f - attrib.texcoords[2 * idx.texcoord_index + 1];
                    } else {
                        vertex.uv[0] = vertex.uv[1] = 0.0f;
                    }
                    
                    vertices.push_back(vertex);
                    vertexMap[key] = currentIndex;
                    indices.push_back(currentIndex);
                    currentIndex++;
                }
            }
            index_offset += fv;
        }
    }
    
    // Calculer les normales si elles sont manquantes
    calculateNormalsIfNeeded();
    
    std::cout << "Loaded mesh with " << vertices.size() << " vertices and " 
              << indices.size() / 3 << " triangles" << std::endl;
    
    setupMesh();
    return true;
}

// Fonction helper pour calculer les normales manquantes
void Mesh::calculateNormalsIfNeeded() {
    // Vérifier si on a besoin de calculer les normales
    bool needsNormals = false;
    for (const auto& vertex : vertices) {
        if (vertex.normal[0] == 0.0f && vertex.normal[1] == 0.0f && vertex.normal[2] == 0.0f) {
            needsNormals = true;
            break;
        }
    }
    
    if (!needsNormals) return;
    
    // Réinitialiser toutes les normales
    for (auto& vertex : vertices) {
        vertex.normal[0] = vertex.normal[1] = vertex.normal[2] = 0.0f;
    }
    
    // Calculer les normales par triangle
    for (size_t i = 0; i < indices.size(); i += 3) {
        if (i + 2 >= indices.size()) break;
        
        unsigned int i0 = indices[i];
        unsigned int i1 = indices[i + 1];
        unsigned int i2 = indices[i + 2];
        
        if (i0 >= vertices.size() || i1 >= vertices.size() || i2 >= vertices.size()) continue;
        
        // Calculer les vecteurs des arêtes
        float v1[3] = {
            vertices[i1].position[0] - vertices[i0].position[0],
            vertices[i1].position[1] - vertices[i0].position[1],
            vertices[i1].position[2] - vertices[i0].position[2]
        };
        
        float v2[3] = {
            vertices[i2].position[0] - vertices[i0].position[0],
            vertices[i2].position[1] - vertices[i0].position[1],
            vertices[i2].position[2] - vertices[i0].position[2]
        };
        
        // Produit vectoriel pour obtenir la normale
        float normal[3] = {
            v1[1] * v2[2] - v1[2] * v2[1],
            v1[2] * v2[0] - v1[0] * v2[2],
            v1[0] * v2[1] - v1[1] * v2[0]
        };
        
        // Normaliser
        float length = sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
        if (length > 0.0f) {
            normal[0] /= length;
            normal[1] /= length;
            normal[2] /= length;
        }
        
        // Ajouter aux vertices du triangle
        for (int j = 0; j < 3; j++) {
            vertices[indices[i + j]].normal[0] += normal[0];
            vertices[indices[i + j]].normal[1] += normal[1];
            vertices[indices[i + j]].normal[2] += normal[2];
        }
    }
    
    // Renormaliser les normales finales
    for (auto& vertex : vertices) {
        float length = sqrt(vertex.normal[0] * vertex.normal[0] + 
                           vertex.normal[1] * vertex.normal[1] + 
                           vertex.normal[2] * vertex.normal[2]);
        if (length > 0.0f) {
            vertex.normal[0] /= length;
            vertex.normal[1] /= length;
            vertex.normal[2] /= length;
        }
    }
}