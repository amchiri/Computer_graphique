#include "../include/CameraController.h"
#include "../imgui/imgui.h"
#include <cmath>

CameraController* CameraController::s_Instance = nullptr;

CameraController::CameraController(GLFWwindow* window)
    : m_Window(window)
{
    s_Instance = this; // Pour le callback statique

    // Obtenir la taille de la fenêtre pour initialiser lastX et lastY
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    m_LastX = width / 2.0f;
    m_LastY = height / 2.0f;

    SetCursorLocked(true);
}

CameraController::~CameraController() {
    // Restaurer le curseur normal à la destruction
    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void CameraController::Initialize() {
    glfwSetCursorPosCallback(m_Window, MouseCallback);
    SetCursorLocked(true);
}

void CameraController::MouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (s_Instance) {
        s_Instance->ProcessMouseMovement(xpos, ypos);
    }
}

void CameraController::Update(float deltaTime) {
    static bool previousO = false;
    bool currentO = glfwGetKey(m_Window, GLFW_KEY_O) == GLFW_PRESS;
    if (currentO && !previousO) {
        SetCursorLocked(!m_CursorLocked);
    }
    previousO = currentO;

    if (!m_Enabled || !m_CursorLocked || ImGui::GetIO().WantCaptureMouse) {
        return;
    }

    HandleKeyboardInput();
}

void CameraController::HandleKeyboardInput() {
    float cameraSpeed = m_Speed;

    // Mouvement avant/arrière
    if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS) {
        m_Position[0] += cameraSpeed * m_Front[0];
        m_Position[1] += cameraSpeed * m_Front[1];
        m_Position[2] += cameraSpeed * m_Front[2];
    }
    if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS) {
        m_Position[0] -= cameraSpeed * m_Front[0];
        m_Position[1] -= cameraSpeed * m_Front[1];
        m_Position[2] -= cameraSpeed * m_Front[2];
    }

    // Mouvement gauche/droite
    if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS || 
        glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS) {
        // Calculer le vecteur droit
        float right[3];
        right[0] = m_Front[1] * m_Up[2] - m_Front[2] * m_Up[1];
        right[1] = m_Front[2] * m_Up[0] - m_Front[0] * m_Up[2];
        right[2] = m_Front[0] * m_Up[1] - m_Front[1] * m_Up[0];
        
        // Normaliser
        float length = sqrt(right[0]*right[0] + right[1]*right[1] + right[2]*right[2]);
        right[0] /= length;
        right[1] /= length;
        right[2] /= length;

        if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS) {
            m_Position[0] -= cameraSpeed * right[0];
            m_Position[1] -= cameraSpeed * right[1];
            m_Position[2] -= cameraSpeed * right[2];
        }
        if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS) {
            m_Position[0] += cameraSpeed * right[0];
            m_Position[1] += cameraSpeed * right[1];
            m_Position[2] += cameraSpeed * right[2];
        }
    }
}

void CameraController::ProcessMouseMovement(double xpos, double ypos) {
    // Ajouter ces vérifications au début
    if (!m_Enabled || !m_CursorLocked || ImGui::GetIO().WantCaptureMouse) {
        m_LastX = xpos;
        m_LastY = ypos;
        return;
    }

    if (m_FirstMouse) {
        m_LastX = xpos;
        m_LastY = ypos;
        m_FirstMouse = false;
        return;
    }

    float xoffset = xpos - m_LastX;
    float yoffset = m_LastY - ypos; // Inversé car y va de bas en haut
    m_LastX = xpos;
    m_LastY = ypos;

    xoffset *= m_Sensitivity;
    yoffset *= m_Sensitivity;

    m_Yaw += xoffset;
    m_Pitch += yoffset;

    // Contraindre le pitch pour éviter le retournement de la caméra
    if (m_Pitch > 89.0f) m_Pitch = 89.0f;
    if (m_Pitch < -89.0f) m_Pitch = -89.0f;

    UpdateCameraVectors();
}

void CameraController::SetPosition(float x, float y, float z) {
    m_Position[0] = x;
    m_Position[1] = y;
    m_Position[2] = z;
}

void CameraController::SetCursorLocked(bool locked) {
    m_CursorLocked = locked;
    m_Enabled = locked; // Synchroniser l'état enabled avec le lock
    
    if (locked) {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        // Réinitialiser la position de la souris au centre
        int width, height;
        glfwGetWindowSize(m_Window, &width, &height);
        glfwSetCursorPos(m_Window, width/2.0f, height/2.0f);
    } else {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    m_FirstMouse = true;
}

void CameraController::UpdateCameraVectors() {
    // Convert degrees to radians
    const float PI = 3.14159265359f;
    float yawRad = m_Yaw * PI / 180.0f;
    float pitchRad = m_Pitch * PI / 180.0f;

    // Calculer le nouveau Front vector
    float newX = cos(yawRad) * cos(pitchRad);
    float newY = sin(pitchRad);
    float newZ = sin(yawRad) * cos(pitchRad);

    m_Front[0] = newX;
    m_Front[1] = newY;
    m_Front[2] = newZ;

    // Normaliser le vecteur
    float length = sqrt(m_Front[0] * m_Front[0] + m_Front[1] * m_Front[1] + m_Front[2] * m_Front[2]);
    m_Front[0] /= length;
    m_Front[1] /= length;
    m_Front[2] /= length;
}

void CameraController::SetRotation(float pitch, float yaw) {
    m_Pitch = pitch;
    m_Yaw = yaw;
    UpdateCameraVectors();
}
