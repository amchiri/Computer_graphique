#include "MatrixUtils.h"
#include <cstring>
#include <cmath>

void multiplyMatrix4x4(const float A[16], const float B[16], float result[16])
{
    // Initialize result to zero.
    for (int i = 0; i < 16; ++i)
        result[i] = 0.0f;
    // For matrices in column-major order, use:
    // result[r + c*4] = sum_{k=0}^{3} A[r + k*4] * B[k + c*4]
    for (int c = 0; c < 4; ++c)
    {
        for (int r = 0; r < 4; ++r)
        {
            for (int k = 0; k < 4; ++k)
            {
                result[r + c * 4] += A[r + k * 4] * B[k + c * 4];
            }
        }
    }
}

void combineTRS(const float translation[16], const float rotation[16], const float scale[16], float outMatrix[16])
{
    float temp[16];
    // First: temp = rotation * scale
    multiplyMatrix4x4(rotation, scale, temp);
    // Then: outMatrix = translation * (rotation * scale)
    multiplyMatrix4x4(translation, temp, outMatrix);
}

void createPerspectiveMatrix(float fov, float aspect, float zNear, float zFar, float outMatrix[16])
{
    // Calculate the cotangent of half the field-of-view angle.
    float tanHalfFov = tan(fov / 2.0f);
    // Set all elements to zero.
    for (int i = 0; i < 16; ++i)
        outMatrix[i] = 0.0f;

    // Column-major order
    outMatrix[0] = 1.0f / (aspect * tanHalfFov);
    outMatrix[5] = 1.0f / tanHalfFov;
    outMatrix[10] = -(zFar + zNear) / (zFar - zNear);
    outMatrix[11] = -1.0f;
    outMatrix[14] = -(2.0f * zFar * zNear) / (zFar - zNear);
}

void createLookAtMatrix(const float eye[3], const float center[3], const float up[3], float outMatrix[16])
{
    // Calculer le vecteur z (forward) normalisé
    float z[3] = {
        eye[0] - center[0],
        eye[1] - center[1],
        eye[2] - center[2]
    };
    float zLength = sqrt(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
    z[0] /= zLength;
    z[1] /= zLength;
    z[2] /= zLength;

    // Calculer le vecteur x (right) comme le produit vectoriel de up et z
    float x[3] = {
        up[1] * z[2] - up[2] * z[1],
        up[2] * z[0] - up[0] * z[2],
        up[0] * z[1] - up[1] * z[0]
    };
    float xLength = sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
    x[0] /= xLength;
    x[1] /= xLength;
    x[2] /= xLength;

    // Calculer le vecteur y (up) comme le produit vectoriel de z et x
    float y[3] = {
        z[1] * x[2] - z[2] * x[1],
        z[2] * x[0] - z[0] * x[2],
        z[0] * x[1] - z[1] * x[0]
    };

    // Construire la matrice de vue (en format colonne-majeure)
    outMatrix[0] = x[0];
    outMatrix[1] = y[0];
    outMatrix[2] = z[0];
    outMatrix[3] = 0.0f;

    outMatrix[4] = x[1];
    outMatrix[5] = y[1];
    outMatrix[6] = z[1];
    outMatrix[7] = 0.0f;

    outMatrix[8] = x[2];
    outMatrix[9] = y[2];
    outMatrix[10] = z[2];
    outMatrix[11] = 0.0f;

    outMatrix[12] = -(x[0] * eye[0] + x[1] * eye[1] + x[2] * eye[2]);
    outMatrix[13] = -(y[0] * eye[0] + y[1] * eye[1] + y[2] * eye[2]);
    outMatrix[14] = -(z[0] * eye[0] + z[1] * eye[1] + z[2] * eye[2]);
    outMatrix[15] = 1.0f;
}
