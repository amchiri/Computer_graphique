#ifndef MATRIX_UTILS_H
#define MATRIX_UTILS_H

// Multiplies two 4x4 matrices (in column-major order): result = A * B.
void multiplyMatrix4x4(const float A[16], const float B[16], float result[16]);

// Combines translation, rotation and scale into one transformation matrix.
// The order is: combined = translation * rotation * scale.
void combineTRS(const float translation[16], const float rotation[16], const float scale[16], float outMatrix[16]);

// Creates a perspective projection matrix
// fov: field of view angle (radians)
// aspect: width/height
// zNear, zFar: near and far clipping planes.
void createPerspectiveMatrix(float fov, float aspect, float zNear, float zFar, float outMatrix[16]);

#endif // MATRIX_UTILS_H
