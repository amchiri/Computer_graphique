#pragma once
#include <GL/glew.h>
#include "GLShader.h"
#include "Mat4.h"

class Skybox {
public:
    Skybox();
    ~Skybox();

    bool Initialize(const char* texturePath);
    void Draw(const Mat4& viewMatrix, const Mat4& projectionMatrix);
    void Cleanup();

private:
    GLuint m_VAO;
    GLuint m_VBO;
    GLuint m_TextureID;
    GLShader m_Shader;

    bool CreateBuffers();
    bool LoadTexture(const char* texturePath);
};
