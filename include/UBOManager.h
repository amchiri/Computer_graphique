#pragma once
#include <GL/glew.h>

class UBOManager {
public:
    static UBOManager& Get();

    void Initialize();
    void Cleanup();

    // Mise à jour des données UBO
    void UpdateProjectionView(const float* projection, const float* view);
    void UpdateTransform(const float* transform);

    // Binding points
    static const GLuint PROJECTION_VIEW_BINDING = 0;
    static const GLuint TRANSFORM_BINDING = 1;

private:
    UBOManager() = default;
    ~UBOManager() = default;

    GLuint m_projViewUBO = 0;
    GLuint m_transformUBO = 0;

    static const size_t MATRIX_SIZE = 16 * sizeof(float);
    static const size_t PROJ_VIEW_SIZE = 2 * MATRIX_SIZE;  // projection + view
};
