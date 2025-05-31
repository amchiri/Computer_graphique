#include "../include/UBOManager.h"
#include <iostream>

UBOManager& UBOManager::Get() {
    static UBOManager instance;
    return instance;
}

void UBOManager::Initialize() {
    // Création de l'UBO pour projection + view
    glGenBuffers(1, &m_projViewUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, m_projViewUBO);
    glBufferData(GL_UNIFORM_BUFFER, PROJ_VIEW_SIZE, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, PROJECTION_VIEW_BINDING, m_projViewUBO);
    
    // Création de l'UBO pour transform
    glGenBuffers(1, &m_transformUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, m_transformUBO);
    glBufferData(GL_UNIFORM_BUFFER, MATRIX_SIZE, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, TRANSFORM_BINDING, m_transformUBO);
    
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UBOManager::Cleanup() {
    if (m_projViewUBO) glDeleteBuffers(1, &m_projViewUBO);
    if (m_transformUBO) glDeleteBuffers(1, &m_transformUBO);
    m_projViewUBO = m_transformUBO = 0;
}

void UBOManager::UpdateProjectionView(const float* projection, const float* view) {
    glBindBuffer(GL_UNIFORM_BUFFER, m_projViewUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, MATRIX_SIZE, projection);
    glBufferSubData(GL_UNIFORM_BUFFER, MATRIX_SIZE, MATRIX_SIZE, view);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UBOManager::UpdateTransform(const float* transform) {
    glBindBuffer(GL_UNIFORM_BUFFER, m_transformUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, MATRIX_SIZE, transform);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
