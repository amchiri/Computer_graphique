#pragma once
#include <array>
#include <cmath>

class Mat4 {
private:
    std::array<float, 16> m_data;  // Column-major order

public:
    Mat4();
    
    // Constructeurs
    explicit Mat4(const float* data);
    Mat4(const Mat4& other);

    // Opérations basiques
    static Mat4 identity();  // Changé en méthode statique
    Mat4& operator=(const Mat4& other);
    Mat4 operator*(const Mat4& other) const;
    
    // Opérateurs de comparaison
    bool operator==(const Mat4& other) const;
    bool operator!=(const Mat4& other) const;

    // Transformations
    static Mat4 translate(float x, float y, float z);
    static Mat4 rotate(float angle, float x, float y, float z);
    static Mat4 scale(float x, float y, float z);
    static Mat4 perspective(float fov, float aspect, float near, float far);
    static Mat4 lookAt(const float* eye, const float* center, const float* up);

    // Accès aux données
    const float* data() const { return m_data.data(); }
    float* data() { return m_data.data(); }
    
    float& operator[](int i) { return m_data[i]; }
    const float& operator[](int i) const { return m_data[i]; }
};
