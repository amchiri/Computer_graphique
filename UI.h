#pragma once

// Même chose pour tous les fichiers .h - s'assurer que GLEW est inclus en premier
#include <GL/glew.h>
#include <GLFW/glfw3.h>
// Ne pas inclure GL/gl.h

#include <vector>
#include <memory>
#include "Mesh.h"
#include "GLShader.h"

struct Planet {
    Mesh* mesh = nullptr;
    float orbitRadius;
    float rotationSpeed;
    float size;
    float selfRotation;
    GLuint texture;
};

class UI {
public:
    UI(GLFWwindow* window, int width, int height);
    virtual ~UI();

    bool Initialize();
    void RenderUI(float fps, const float* cameraPos, const float* cameraFront);
    void SetSceneObjects(std::vector<Mesh*>& objects, Mesh* sun, std::vector<Planet>& planets);
    void SetShaders(GLShader* basic, GLShader* color, GLShader* envmap);
    void SetLightParameters(float* lightColor, float* lightIntensity);

private:
    void ShowMainWindow(float fps, const float* cameraPos, const float* cameraFront);
    void ShowLightSettings();
    void ShowObjectControls();
    void ShowShaderSettings();
    
    GLFWwindow* m_Window;
    int m_Width;
    int m_Height;
    bool m_ShowDebugWindow;
    bool m_ShowSettings;
    bool m_WireframeMode;
    int m_SelectedObject;
    
    std::vector<Mesh*>* m_SceneObjects;
    Mesh* m_Sun;
    std::vector<Planet>* m_Planets;
    
    GLShader* m_BasicShader;
    GLShader* m_ColorShader;
    GLShader* m_EnvMapShader;
    
    float m_LightColor[3] = {1.0f, 1.0f, 1.0f};
    float m_LightIntensity = 1.0f;
    float* m_GlobalLightColor = nullptr;
    float* m_GlobalLightIntensity = nullptr;
};
