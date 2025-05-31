#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class CameraController {
public:
    CameraController(GLFWwindow* window);
    ~CameraController();

    void Initialize();  // Nouvelle méthode pour initialiser les callbacks
    void Update(float deltaTime);
    void SetPosition(float x, float y, float z);
    void SetRotation(float pitch, float yaw);
    void SetMovementSpeed(float speed) { m_Speed = speed; }
    void SetSensitivity(float sensitivity) { m_Sensitivity = sensitivity; }
    void SetCursorLocked(bool locked);
    void SetEnabled(bool enabled) { m_Enabled = enabled; }

    const float* GetPosition() const { return m_Position; }
    const float* GetFront() const { return m_Front; }
    const float* GetUp() const { return m_Up; }
    bool IsEnabled() const { return m_Enabled; }
    bool IsCursorLocked() const { return m_CursorLocked; }

    // Callback statique pour GLFW
    static void MouseCallback(GLFWwindow* window, double xpos, double ypos);
    
private:
    void ProcessMouseMovement(double xpos, double ypos);
    void UpdateCameraVectors();
    void HandleKeyboardInput();

    static CameraController* s_Instance; // Pour le callback statique

    GLFWwindow* m_Window;
    float m_Position[3] = {0.0f, 0.0f, 3.0f};
    float m_Front[3] = {0.0f, 0.0f, -1.0f};
    float m_Up[3] = {0.0f, 1.0f, 0.0f};
    float m_Yaw = -90.0f;
    float m_Pitch = 0.0f;
    float m_Speed = 1.0f;
    float m_Sensitivity = 0.1f;
    bool m_Enabled = true;
    bool m_CursorLocked = true;
    bool m_FirstMouse = true;
    float m_LastX = 0.0f;
    float m_LastY = 0.0f;
};
