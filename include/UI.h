#pragma once

// Même chose pour tous les fichiers .h - s'assurer que GLEW est inclus en premier
#include <GL/glew.h>
#include <GLFW/glfw3.h>
// Ne pas inclure GL/gl.h

#include <windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <vector>
#include <memory>
#include "Mesh.h"
#include "GLShader.h"
#include "Planet.h"  // Include the Planet class instead of defining struct Planet

class UI {
public:
    UI(GLFWwindow* window, int width, int height);
    virtual ~UI();

    bool Initialize();
    void RenderUI(float fps, const float* cameraPos, const float* cameraFront);
    void SetSceneObjects(const std::vector<Mesh*>& objects, Mesh* sun, const std::vector<Planet>& planets);
    void SetShaders(GLShader* basic, GLShader* color, GLShader* envmap); // Changed from references to pointers
    void SetLightParameters(float* lightColor, float* lightIntensity);
    bool IsWireframeMode() const { return m_WireframeMode; }

private:
    void ShowMainWindow(float fps, const float* cameraPos, const float* cameraFront);
    void ShowObjectControls();
    void ShowShaderSettings();
    void ShowSceneManagerWindow();
    void ShowSceneControls();
    void ShowNewSceneDialog();
    void ShowLoadModelDialog(); // Added function to show load model dialog

    GLFWwindow* m_Window;
    int m_Width;
    int m_Height;
    bool m_ShowDebugWindow;
    bool m_ShowSettings;
    bool m_WireframeMode = false;
    int m_SelectedObject;
    bool m_ShowNewSceneDialog;
    char m_NewSceneName[256];

    std::vector<Mesh*>* m_SceneObjects;
    Mesh* m_Sun;
    std::vector<Planet>* m_Planets;  // Utiliser un pointeur vers le vecteur de planètes
    
    GLShader* m_BasicShader;
    GLShader* m_ColorShader;
    GLShader* m_EnvMapShader;
    
    float m_LightColor[3] = {1.0f, 1.0f, 1.0f};
    float m_LightIntensity = 1.0f;
    float* m_GlobalLightColor = nullptr;
    float* m_GlobalLightIntensity = nullptr;

    bool m_ShowLoadModelDialog = false; // Added variable to manage load model dialog visibility
};
