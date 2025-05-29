#include "../include/Planet.h"
#include <cmath>

Planet::Planet()
    : m_Mesh(nullptr)
    , m_Texture(0)
    , m_OrbitRadius(0.0f)
    , m_RotationSpeed(0.0f)
    , m_Size(1.0f)
    , m_SelfRotation(0.0f)
    , m_CurrentAngle(0.0f)
{
    CreateMesh();
}

Planet::~Planet() {
    if (m_Mesh) {
        delete m_Mesh;
        m_Mesh = nullptr;
    }
}

Planet::Planet(Planet&& other) noexcept 
    : m_Mesh(other.m_Mesh)
    , m_Texture(other.m_Texture)
    , m_OrbitRadius(other.m_OrbitRadius)
    , m_RotationSpeed(other.m_RotationSpeed)
    , m_Size(other.m_Size)
    , m_SelfRotation(other.m_SelfRotation)
    , m_CurrentAngle(other.m_CurrentAngle)
{
    other.m_Mesh = nullptr;
    other.m_Texture = 0;
}

Planet& Planet::operator=(Planet&& other) noexcept {
    if (this != &other) {
        if (m_Mesh) delete m_Mesh;
        
        m_Mesh = other.m_Mesh;
        m_Texture = other.m_Texture;
        m_OrbitRadius = other.m_OrbitRadius;
        m_RotationSpeed = other.m_RotationSpeed;
        m_Size = other.m_Size;
        m_SelfRotation = other.m_SelfRotation;
        m_CurrentAngle = other.m_CurrentAngle;

        other.m_Mesh = nullptr;
        other.m_Texture = 0;
    }
    return *this;
}

void Planet::CreateMesh() {
    if (!m_Mesh) {
        m_Mesh = new Mesh();
        m_Mesh->createSphere(1.0f, 32, 32);
    }
}

void Planet::Initialize(float orbitRadius, float rotationSpeed, float size) {
    m_OrbitRadius = orbitRadius;
    m_RotationSpeed = rotationSpeed;
    m_Size = size;
    m_SelfRotation = 0.0f;
    m_CurrentAngle = 0.0f;
    SetSize(size);
    UpdateTransform();
}

void Planet::Update(float deltaTime) {
    m_CurrentAngle += m_RotationSpeed * deltaTime;
    m_SelfRotation += deltaTime * 0.5f;
    UpdateTransform();
}

void Planet::SetSize(float size) {
    m_Size = size;
    if (m_Mesh) {
        m_Mesh->setScale(size, size, size);
    }
    UpdateTransform();
}

void Planet::SetMaterial(const Material& material) {
    if (m_Mesh) {
        m_Mesh->setMaterial(material);
    }
}

bool Planet::LoadTexture(const char* texturePath) {
    if (m_Mesh && m_Mesh->loadTexture(texturePath)) {
        m_Texture = m_Mesh->getMaterial().diffuseMap;
        return true;
    }
    return false;
}

void Planet::UpdateTransform() {
    float x = cos(m_CurrentAngle) * m_OrbitRadius;
    float z = sin(m_CurrentAngle) * m_OrbitRadius;
    
    Mat4 translation = Mat4::translate(x, 0.0f, z);
    Mat4 rotation = Mat4::rotate(m_SelfRotation, 0.0f, 1.0f, 0.0f);
    Mat4 scale = Mat4::scale(m_Size, m_Size, m_Size);
    
    Mat4 transform = translation * rotation * scale;
    if (m_Mesh) {
        m_Mesh->setTransform(transform);
    }
}
