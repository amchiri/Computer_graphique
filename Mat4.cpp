#include "Mat4.h"

Mat4::Mat4(const float* data) {
    std::copy(data, data + 16, m_data.begin());
}

Mat4::Mat4(const Mat4& other) : m_data(other.m_data) {}

void Mat4::identity() {
    std::fill(m_data.begin(), m_data.end(), 0.0f);
    m_data[0] = m_data[5] = m_data[10] = m_data[15] = 1.0f;
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
            result[i + j * 4] = sum;
        }
    }
    return result;
}

// Implémentation des transformations...
