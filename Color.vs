#version 330 core

layout(location = 0) in vec3 a_position;

uniform mat4 u_transform;
uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
    // Applique bien la matrice de transformation (u_transform)
    gl_Position = u_projection * u_view * u_transform * vec4(a_position, 1.0);
}
