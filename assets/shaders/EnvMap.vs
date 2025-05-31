#version 330 core

// Définition des uniform blocks
layout(std140) uniform ProjectionView {
    mat4 u_projection;
    mat4 u_view;
};

layout(std140) uniform Transform {
    mat4 u_transform;
};

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
out vec3 v_normal;
out vec3 v_worldPos;
void main() {
    gl_Position = u_projection * u_view * u_transform * vec4(a_position, 1.0);
    vec4 worldPos = u_transform * vec4(a_position, 1.0);
    v_worldPos = worldPos.xyz;
    v_normal = mat3(u_transform) * a_normal;
}
