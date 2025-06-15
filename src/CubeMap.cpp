#include "../include/CubeMap.h"
#include <stb/stb_image.h>
#include <iostream>
#include <GL/glew.h>

CubeMap::CubeMap() : m_TextureID(0), m_IsLoaded(false) {
}

CubeMap::~CubeMap() {
    if (m_TextureID) {
        glDeleteTextures(1, &m_TextureID);
    }
}

bool CubeMap::LoadFromImages(const std::vector<std::string>& faces) {
    if (faces.size() != 6) {
        std::cerr << "CubeMap requires exactly 6 faces" << std::endl;
        return false;
    }

    glGenTextures(1, &m_TextureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureID);

    for (int i = 0; i < 6; i++) {
        int width, height, channels;
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &channels, 0);
        
        if (data) {
            GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, width, height, 0,
                         format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cerr << "Failed to load cubemap texture: " << faces[i] << std::endl;
            return false;
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    m_IsLoaded = true;
    return true;
}

bool CubeMap::CreateProcedural() {
    glGenTextures(1, &m_TextureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureID);
    
    unsigned char colors[6][3] = {
        {255, 100, 100},  // +X (droite) - rouge
        {100, 255, 100},  // -X (gauche) - vert
        {100, 100, 255},  // +Y (haut) - bleu
        {255, 255, 100},  // -Y (bas) - jaune
        {255, 100, 255},  // +Z (fond) - magenta
        {100, 255, 255}   // -Z (avant) - cyan
    };
    
    const int size = 256;
    unsigned char* data = new unsigned char[size * size * 3];
    
    for (int i = 0; i < 6; i++) {
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                int index = (y * size + x) * 3;
                float fx = (float)x / size;
                float fy = (float)y / size;
                
                data[index + 0] = (unsigned char)(colors[i][0] * (0.5f + 0.5f * fx));
                data[index + 1] = (unsigned char)(colors[i][1] * (0.5f + 0.5f * fy));
                data[index + 2] = (unsigned char)(colors[i][2] * (0.7f + 0.3f * (fx + fy) * 0.5f));
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
    
    m_IsLoaded = true;
    return true;
}

void CubeMap::Bind(GLuint unit) {
    if (!m_IsLoaded) return;
    
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureID);
}

void CubeMap::Reload() {
    if (m_TextureID) {
        glDeleteTextures(1, &m_TextureID);
        m_TextureID = 0;
        m_IsLoaded = false;
    }
    CreateProcedural();
}

bool CubeMap::LoadFromFiles(const std::string& posX, const std::string& negX,
                           const std::string& posY, const std::string& negY,
                           const std::string& posZ, const std::string& negZ) {
    std::vector<std::string> faces = {posX, negX, posY, negY, posZ, negZ};
    return LoadFromImages(faces);
}