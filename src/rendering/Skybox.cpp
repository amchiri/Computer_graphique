#include "Skybox.h"
#include <stb/stb_image.h>
#include <iostream>

Skybox::Skybox() : m_VAO(0), m_VBO(0), m_TextureID(0) {
}

Skybox::~Skybox() {
    Cleanup();
}

bool Skybox::Initialize(const char* texturePath) {
    std::cout << "Initializing Skybox..." << std::endl;
    
    // Charger les shaders
    std::cout << "Loading skybox shaders..." << std::endl;
    if (!m_Shader.LoadVertexShader("src/shaders/Skybox.vs")) {
        std::cerr << "Failed to load skybox vertex shader" << std::endl;
        return false;
    }
    if (!m_Shader.LoadFragmentShader("src/shaders/Skybox.fs")) {
        std::cerr << "Failed to load skybox fragment shader" << std::endl;
        return false;
    }
    if (!m_Shader.Create()) {
        std::cerr << "Failed to create skybox shader program" << std::endl;
        return false;
    }
    std::cout << "Skybox shaders loaded successfully" << std::endl;

    std::cout << "Creating skybox buffers..." << std::endl;
    if (!CreateBuffers()) {
        std::cerr << "Failed to create skybox buffers" << std::endl;
        return false;
    }
    std::cout << "Skybox buffers created successfully" << std::endl;

    std::cout << "Loading skybox texture: " << texturePath << std::endl;
    if (!LoadTexture(texturePath)) {
        std::cerr << "Failed to load skybox texture: " << texturePath << std::endl;
        return false;
    }
    std::cout << "Skybox texture loaded successfully" << std::endl;

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
    // Clear any previous texture
    if (m_TextureID != 0) {
        glDeleteTextures(1, &m_TextureID);
        m_TextureID = 0;
    }

    // Load texture data using stb_image
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(texturePath, &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Failed to load skybox texture data: " << stbi_failure_reason() << std::endl;
        return false;
    }

    // Create and configure the texture
    glGenTextures(1, &m_TextureID);
    glBindTexture(GL_TEXTURE_2D, m_TextureID);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Upload texture data
    GLenum format = channels == 4 ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    
    // Free the loaded data
    stbi_image_free(data);

    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error while loading skybox texture: " << error << std::endl;
        glDeleteTextures(1, &m_TextureID);
        m_TextureID = 0;
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
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
