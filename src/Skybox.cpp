#include "../include/Skybox.h"
#include "../include/UBOManager.h"
#include <stb/stb_image.h>
#include <iostream>

Skybox::Skybox() : m_VAO(0), m_VBO(0), m_TextureID(0) {
}

Skybox::~Skybox() {
    Cleanup();
}

bool Skybox::Initialize(const char* texturePath) {
    // Charger les shaders
    if (!m_Shader.LoadVertexShader("assets/shaders/Skybox.vs") ||
        !m_Shader.LoadFragmentShader("assets/shaders/Skybox.fs") ||
        !m_Shader.Create()) {
        std::cerr << "Failed to load skybox shaders" << std::endl;
        return false;
    }

    if (!CreateBuffers()) {
        std::cerr << "Failed to create skybox buffers" << std::endl;
        return false;
    }

    if (!LoadTexture(texturePath)) {
        std::cerr << "Failed to load skybox texture" << std::endl;
        return false;
    }

    return true;
}

bool Skybox::CreateBuffers() {
    float skyboxVertices[] = {
        -1000.0f,  1000.0f, -1000.0f,
        -1000.0f, -1000.0f, -1000.0f,
         1000.0f, -1000.0f, -1000.0f,
         1000.0f, -1000.0f, -1000.0f,
         1000.0f,  1000.0f, -1000.0f,
        -1000.0f,  1000.0f, -1000.0f,

        -1000.0f, -1000.0f,  1000.0f,
        -1000.0f, -1000.0f, -1000.0f,
        -1000.0f,  1000.0f, -1000.0f,
        -1000.0f,  1000.0f, -1000.0f,
        -1000.0f,  1000.0f,  1000.0f,
        -1000.0f, -1000.0f,  1000.0f,

         1000.0f, -1000.0f, -1000.0f,
         1000.0f, -1000.0f,  1000.0f,
         1000.0f,  1000.0f,  1000.0f,
         1000.0f,  1000.0f,  1000.0f,
         1000.0f,  1000.0f, -1000.0f,
         1000.0f, -1000.0f, -1000.0f,

        -1000.0f, -1000.0f,  1000.0f,
        -1000.0f,  1000.0f,  1000.0f,
         1000.0f,  1000.0f,  1000.0f,
         1000.0f,  1000.0f,  1000.0f,
         1000.0f, -1000.0f,  1000.0f,
        -1000.0f, -1000.0f,  1000.0f,

        -1000.0f,  1000.0f, -1000.0f,
         1000.0f,  1000.0f, -1000.0f,
         1000.0f,  1000.0f,  1000.0f,
         1000.0f,  1000.0f,  1000.0f,
        -1000.0f,  1000.0f,  1000.0f,
        -1000.0f,  1000.0f, -1000.0f,

        -1000.0f, -1000.0f, -1000.0f,
        -1000.0f, -1000.0f,  1000.0f,
         1000.0f, -1000.0f, -1000.0f,
         1000.0f, -1000.0f, -1000.0f,
        -1000.0f, -1000.0f,  1000.0f,
         1000.0f, -1000.0f,  1000.0f
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

void Skybox::Draw(const Mat4& viewMatrix, const Mat4& projectionMatrix) {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    GLuint program = m_Shader.GetProgram();
    glUseProgram(program);

    // Textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_TextureID);
    GLint loc_texture = glGetUniformLocation(program, "u_texture");
    if (loc_texture >= 0) {
        glUniform1i(loc_texture, 0);
    }

    // Rendu
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Restaurer les états
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

bool Skybox::LoadTexture(const char* texturePath) {
    int width, height, channels;
    unsigned char* data = stbi_load(texturePath, &width, &height, &channels, 0);
    
    if (!data) {
        std::cerr << "Failed to load skybox texture: " << texturePath << std::endl;
        return false;
    }

    glGenTextures(1, &m_TextureID);
    glBindTexture(GL_TEXTURE_2D, m_TextureID);
    
    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    GLenum internalFormat = (channels == 4) ? GL_RGBA8 : GL_RGB8;
    
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    stbi_image_free(data);
    return true;
}

void Skybox::Cleanup() {
    if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
    if (m_VBO) glDeleteBuffers(1, &m_VBO);
    if (m_TextureID) glDeleteTextures(1, &m_TextureID);
    m_Shader.Destroy();
    
    m_VAO = m_VBO = m_TextureID = 0;
}
