#include "../include/Skybox.h"
#include "../include/UBOManager.h"
#include <stb/stb_image.h>
#include <iostream>
#include <filesystem>
#include <vector>
#include <windows.h>

Skybox::Skybox() : m_VAO(0), m_VBO(0), m_TextureID(0) {
}

Skybox::~Skybox() {
    Cleanup();
}

bool Skybox::Initialize(const char* texturePath) {
    // Charger les shaders dédiés au Skybox
    if (!m_Shader.LoadVertexShader("assets/shaders/Skybox.vs") ||
        !m_Shader.LoadFragmentShader("assets/shaders/Skybox.fs") ||
        !m_Shader.Create()) {
        std::cerr << "Failed to load Skybox shaders" << std::endl;
        return false;
    }

    if (!CreateBuffers()) {
        std::cerr << "Failed to create skybox buffers" << std::endl;
        return false;
    }

    // Charger le cubemap depuis le dossier assets/skybox
    if (!LoadCubeMap()) {
        std::cerr << "Failed to load skybox cubemap" << std::endl;
        return false;
    }

    return true;
}

bool Skybox::CreateBuffers() {
    // Créer un cube simple pour le skybox
    float skyboxVertices[] = {
        // Front face
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        // Left face
        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        // Right face
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        // Back face
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        // Top face
        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        // Bottom face
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    return true;
}

bool Skybox::LoadCubeMap() {
    // Définir les noms des fichiers du cubemap
    std::vector<std::string> faceFiles = {
        "right.png",   // +X
        "left.png",    // -X  
        "top.png",     // +Y
        "bottom.png",  // -Y
        "front.png",   // +Z
        "back.png"     // -Z
    };

    // Construire le chemin vers le dossier assets/skybox
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::filesystem::path basePath = std::filesystem::path(exePath).parent_path();
    std::filesystem::path skyboxDir = basePath / "assets" / "skybox";

    std::cout << "Looking for skybox files in: " << skyboxDir << std::endl;

    // Vérifier que le dossier existe
    if (!std::filesystem::exists(skyboxDir)) {
        std::cerr << "Skybox directory not found: " << skyboxDir << std::endl;
        return CreateProceduralCubeMap(); // Fallback sur un cubemap procédural
    }

    // Construire les chemins complets
    std::vector<std::string> fullPaths;
    for (const auto& filename : faceFiles) {
        std::filesystem::path fullPath = skyboxDir / filename;
        if (std::filesystem::exists(fullPath)) {
            fullPaths.push_back(fullPath.string());
            std::cout << "Found: " << fullPath << std::endl;
        } else {
            std::cerr << "Missing skybox file: " << fullPath << std::endl;
            return CreateProceduralCubeMap(); // Fallback si un fichier manque
        }
    }

    // Charger le cubemap
    glGenTextures(1, &m_TextureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureID);

    for (int i = 0; i < 6; i++) {
        int width, height, channels;
        unsigned char* data = stbi_load(fullPaths[i].c_str(), &width, &height, &channels, 0);
        
        if (data) {
            GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, width, height, 0,
                         format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
            std::cout << "Loaded cubemap face " << i << ": " << fullPaths[i] << std::endl;
        } else {
            std::cerr << "Failed to load cubemap texture: " << fullPaths[i] << std::endl;
            return CreateProceduralCubeMap();
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    std::cout << "Skybox cubemap loaded successfully!" << std::endl;
    return true;
}

bool Skybox::LoadCubeMap(std::string directory) {
    // Définir les noms des fichiers du cubemap
    std::vector<std::string> faceFiles = {
        "right.png",   // +X
        "left.png",    // -X  
        "top.png",     // +Y
        "bottom.png",  // -Y
        "front.png",   // +Z
        "back.png"     // -Z
    };

    // Construire le chemin vers le dossier assets/skybox
    char exePath[MAX_PATH];
    std::filesystem::path skyboxDir = directory;

    std::cout << "Looking for skybox files in: " << skyboxDir << std::endl;

    // Vérifier que le dossier existe
    if (!std::filesystem::exists(skyboxDir)) {
        std::cerr << "Skybox directory not found: " << skyboxDir << std::endl;
        return CreateProceduralCubeMap(); // Fallback sur un cubemap procédural
    }

    // Construire les chemins complets
    std::vector<std::string> fullPaths;
    for (const auto& filename : faceFiles) {
        std::filesystem::path fullPath = skyboxDir / filename;
        if (std::filesystem::exists(fullPath)) {
            fullPaths.push_back(fullPath.string());
            std::cout << "Found: " << fullPath << std::endl;
        } else {
            std::cerr << "Missing skybox file: " << fullPath << std::endl;
            return CreateProceduralCubeMap(); // Fallback si un fichier manque
        }
    }

    // Charger le cubemap
    glGenTextures(1, &m_TextureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureID);

    for (int i = 0; i < 6; i++) {
        int width, height, channels;
        unsigned char* data = stbi_load(fullPaths[i].c_str(), &width, &height, &channels, 0);
        
        if (data) {
            GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, width, height, 0,
                         format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
            std::cout << "Loaded cubemap face " << i << ": " << fullPaths[i] << std::endl;
        } else {
            std::cerr << "Failed to load cubemap texture: " << fullPaths[i] << std::endl;
            return CreateProceduralCubeMap();
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    std::cout << "Skybox cubemap loaded successfully!" << std::endl;
    return true;
}

bool Skybox::CreateProceduralCubeMap() {
    std::cout << "Creating procedural cubemap for skybox..." << std::endl;
    
    glGenTextures(1, &m_TextureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureID);
    
    // Couleurs pour chaque face du skybox
    unsigned char colors[6][3] = {
        {135, 206, 235},  // +X (droite) - bleu ciel
        {135, 206, 235},  // -X (gauche) - bleu ciel
        {173, 216, 230},  // +Y (haut) - bleu clair
        {34, 139, 34},    // -Y (bas) - vert forêt
        {135, 206, 235},  // +Z (fond) - bleu ciel
        {135, 206, 235}   // -Z (avant) - bleu ciel
    };
    
    const int size = 512;
    unsigned char* data = new unsigned char[size * size * 3];
    
    for (int i = 0; i < 6; i++) {
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                int index = (y * size + x) * 3;
                float fx = (float)x / size;
                float fy = (float)y / size;
                
                // Créer un gradient pour un effet plus naturel
                float gradient = 1.0f;
                if (i == 2) { // Face du haut - dégradé du bleu au blanc
                    gradient = 0.7f + 0.3f * fy;
                } else if (i == 3) { // Face du bas - plus sombre
                    gradient = 0.5f + 0.3f * (1.0f - fy);
                }
                
                data[index + 0] = (unsigned char)(colors[i][0] * gradient);
                data[index + 1] = (unsigned char)(colors[i][1] * gradient);
                data[index + 2] = (unsigned char)(colors[i][2] * gradient);
            }
        }
        
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, 
                     size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    
    delete[] data;
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    std::cout << "Procedural skybox cubemap created successfully!" << std::endl;
    return true;
}

void Skybox::Draw(const Mat4& viewMatrix, const Mat4& projectionMatrix) {
    // Sauvegarder les états OpenGL
    GLboolean depthWriteEnabled;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWriteEnabled);
    GLboolean cullFaceEnabled = glIsEnabled(GL_CULL_FACE);
    GLenum depthFunc;
    glGetIntegerv(GL_DEPTH_FUNC, (GLint*)&depthFunc);
    
    // Configurer les états pour le skybox
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);  // Important pour que le skybox passe le test de profondeur
    glDisable(GL_CULL_FACE);
    
    GLuint program = m_Shader.GetProgram();
    glUseProgram(program);

    // Configurer les matrices - enlever la translation de la vue
    Mat4 skyboxView = viewMatrix;
    float* viewData = const_cast<float*>(skyboxView.data());
    viewData[12] = 0.0f; // tx
    viewData[13] = 0.0f; // ty
    viewData[14] = 0.0f; // tz

    GLint loc_proj = glGetUniformLocation(program, "u_projection");
    if (loc_proj >= 0) glUniformMatrix4fv(loc_proj, 1, GL_FALSE, projectionMatrix.data());
    
    GLint loc_view = glGetUniformLocation(program, "u_view");
    if (loc_view >= 0) glUniformMatrix4fv(loc_view, 1, GL_FALSE, skyboxView.data());

    // Lier le cubemap
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureID);
    GLint loc_skybox = glGetUniformLocation(program, "u_skybox");
    if (loc_skybox >= 0) glUniform1i(loc_skybox, 0);

    // Rendu
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Restaurer les états OpenGL
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    if (depthWriteEnabled) glDepthMask(GL_TRUE);
    glDepthFunc(depthFunc);
    if (cullFaceEnabled) glEnable(GL_CULL_FACE);
}

void Skybox::Cleanup() {
    if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
    if (m_VBO) glDeleteBuffers(1, &m_VBO);
    if (m_TextureID) glDeleteTextures(1, &m_TextureID);
    m_Shader.Destroy();
    
    m_VAO = m_VBO = m_TextureID = 0;
}
