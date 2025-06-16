#pragma once

#include <GL/glew.h>
#include <vector>
#include <string>
#include "GLShader.h"
#include "Mat4.h"

class Skybox {
public:
    Skybox();
    ~Skybox();
    
    bool Initialize(const char* texturePath = nullptr);
    void Draw(const Mat4& viewMatrix, const Mat4& projectionMatrix);
    void Cleanup();

private:
    bool CreateBuffers();
    bool LoadCubeMap();
    bool CreateProceduralCubeMap();
    
    GLuint m_VAO;
    GLuint m_VBO;
    GLuint m_TextureID;
    GLShader m_Shader;
};
