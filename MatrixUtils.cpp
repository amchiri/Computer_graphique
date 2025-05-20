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
