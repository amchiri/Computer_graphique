#include "../include/Skybox.h"
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

bool Skybox::LoadTexture(const char* texturePath) {
    int width, height, channels;
    unsigned char* data = stbi_load(texturePath, &width, &height, &channels, STBI_rgb_alpha);
    
    if (!data) {
        std::cerr << "Failed to load skybox texture: " << texturePath << std::endl;
        std::cerr << "Reason: " << stbi_failure_reason() << std::endl;
        return false;
    }

    glGenTextures(1, &m_TextureID);
    glBindTexture(GL_TEXTURE_2D, m_TextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return true;
}

void Skybox::Draw(const Mat4& viewMatrix, const Mat4& projectionMatrix) {
    glDepthFunc(GL_LEQUAL);
    
    GLuint program = m_Shader.GetProgram();
    glUseProgram(program);

    // Supprimer la translation de la matrice de vue
    Mat4 skyboxView = viewMatrix;
    float* skyboxData = skyboxView.data();
    skyboxData[12] = skyboxData[13] = skyboxData[14] = 0.0f;

    GLint viewLoc = glGetUniformLocation(program, "u_view");
    GLint projLoc = glGetUniformLocation(program, "u_projection");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, skyboxView.data());
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projectionMatrix.data());

    glBindVertexArray(m_VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_TextureID);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDepthFunc(GL_LESS);
}

void Skybox::Cleanup() {
    if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
    if (m_VBO) glDeleteBuffers(1, &m_VBO);
    if (m_TextureID) glDeleteTextures(1, &m_TextureID);
    m_Shader.Destroy();
    
    m_VAO = m_VBO = m_TextureID = 0;
}
