#include "Mat4.h"

Mat4::Mat4() {
    std::fill(m_data.begin(), m_data.end(), 0.0f);
    m_data[0] = m_data[5] = m_data[10] = m_data[15] = 1.0f;
}

Mat4::Mat4(const float* data) {
    std::copy(data, data + 16, m_data.begin());
}

Mat4::Mat4(const Mat4& other) : m_data(other.m_data) {}

Mat4 Mat4::identity() {
    Mat4 result;
    std::fill(result.m_data.begin(), result.m_data.end(), 0.0f);
    result.m_data[0] = result.m_data[5] = result.m_data[10] = result.m_data[15] = 1.0f;
    return result;
}

Mat4& Mat4::operator=(const Mat4& other) {
    if (this != &other) {
        m_data = other.m_data;
    }
    return *this;
}

Mat4 Mat4::operator*(const Mat4& other) const {
    Mat4 result;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float sum = 0.0f;
            for (int k = 0; k < 4; k++) {
                sum += m_data[i + k * 4] * other.m_data[k + j * 4];
            }
            result.m_data[i + j * 4] = sum;
        }
    }
    return result;
}

Mat4 Mat4::translate(float x, float y, float z) {
    Mat4 result = identity();
    result.m_data[12] = x;
    result.m_data[13] = y;
    result.m_data[14] = z;
    return result;
}

Mat4 Mat4::rotate(float angle, float x, float y, float z) {
    Mat4 result = identity();
    
    float c = cosf(angle);
    float s = sinf(angle);
    float t = 1.0f - c;
    
    // Normaliser l'axe de rotation
    float len = sqrtf(x*x + y*y + z*z);
    if (len != 0) {
        x /= len;
        y /= len;
        z /= len;
    }

    result.m_data[0] = t*x*x + c;
    result.m_data[1] = t*x*y + s*z;
    result.m_data[2] = t*x*z - s*y;
    
    result.m_data[4] = t*x*y - s*z;
    result.m_data[5] = t*y*y + c;
    result.m_data[6] = t*y*z + s*x;
    
    result.m_data[8] = t*x*z + s*y;
    result.m_data[9] = t*y*z - s*x;
    result.m_data[10] = t*z*z + c;

    return result;
}

Mat4 Mat4::scale(float x, float y, float z) {
    Mat4 result = identity();
    result.m_data[0] = x;
    result.m_data[5] = y;
    result.m_data[10] = z;
    return result;
}

Mat4 Mat4::perspective(float fov, float aspect, float near, float far) {
    Mat4 result;
    float tanHalfFov = tanf(fov / 2.0f);
    
    result.m_data[0] = 1.0f / (aspect * tanHalfFov);
    result.m_data[5] = 1.0f / tanHalfFov;
    result.m_data[10] = -(far + near) / (far - near);
    result.m_data[11] = -1.0f;
    result.m_data[14] = -(2.0f * far * near) / (far - near);
    result.m_data[15] = 0.0f;
    
    return result;
}

Mat4 Mat4::lookAt(const float* eye, const float* center, const float* up) {
    Mat4 result;
    
    float f[3] = {
        center[0] - eye[0],
        center[1] - eye[1],
        center[2] - eye[2]
    };
    
    // Normalize f
    float len = sqrtf(f[0]*f[0] + f[1]*f[1] + f[2]*f[2]);
    f[0] /= len;
    f[1] /= len;
    f[2] /= len;
    
    // Calculate s = f × up
    float s[3] = {
        f[1] * up[2] - f[2] * up[1],
        f[2] * up[0] - f[0] * up[2],
        f[0] * up[1] - f[1] * up[0]
    };
    
    // Normalize s
    len = sqrtf(s[0]*s[0] + s[1]*s[1] + s[2]*s[2]);
    s[0] /= len;
    s[1] /= len;
    s[2] /= len;
    
    // Calculate u = s × f
    float u[3] = {
        s[1] * f[2] - s[2] * f[1],
        s[2] * f[0] - s[0] * f[2],
        s[0] * f[1] - s[1] * f[0]
    };
    
    result.m_data[0] = s[0];
    result.m_data[4] = s[1];
    result.m_data[8] = s[2];
    result.m_data[1] = u[0];
    result.m_data[5] = u[1];
    result.m_data[9] = u[2];
    result.m_data[2] = -f[0];
    result.m_data[6] = -f[1];
    result.m_data[10] = -f[2];
    result.m_data[12] = -(s[0]*eye[0] + s[1]*eye[1] + s[2]*eye[2]);
    result.m_data[13] = -(u[0]*eye[0] + u[1]*eye[1] + u[2]*eye[2]);
    result.m_data[14] = (f[0]*eye[0] + f[1]*eye[1] + f[2]*eye[2]);
    result.m_data[15] = 1.0f;
    
    return result;
}
