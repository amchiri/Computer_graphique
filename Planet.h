#pragma once
#include <GL/glew.h>
#include "Mesh.h"

class Planet {
public:
    Planet();
    ~Planet();

    Planet(const Planet&) = delete;  // Interdire la copie
    Planet& operator=(const Planet&) = delete;
    
    Planet(Planet&& other) noexcept;  // Permettre le déplacement
    Planet& operator=(Planet&& other) noexcept;

    void Initialize(float orbitRadius, float rotationSpeed, float size);
    void Update(float deltaTime);
    void SetMaterial(const Material& material);
    bool LoadTexture(const char* texturePath);

    // Getters
    Mesh* GetMesh() const { return m_Mesh; }
    float GetOrbitRadius() const { return m_OrbitRadius; }
    float GetRotationSpeed() const { return m_RotationSpeed; }
    float GetSize() const { return m_Size; }
    GLuint GetTexture() const { return m_Texture; }

    // Setters
    void SetOrbitRadius(float radius) { m_OrbitRadius = radius; UpdateTransform(); }
    void SetRotationSpeed(float speed) { m_RotationSpeed = speed; }
    void SetSize(float size);  // Déplacer l'implémentation dans le .cpp

private:
    void UpdateTransform();
    void CreateMesh();

    Mesh* m_Mesh;
    GLuint m_Texture;
    float m_OrbitRadius;
    float m_RotationSpeed;
    float m_Size;
    float m_SelfRotation;
    float m_CurrentAngle;
};
