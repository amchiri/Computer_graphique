#version 330 core

// Définition des uniform blocks
layout(std140) uniform ProjectionView {
    mat4 u_projection;
    mat4 u_view;
};

layout(location = 0) in vec3 a_position;
out vec3 v_texCoord;

void main() {
    vec4 pos = u_projection * mat4(mat3(u_view)) * vec4(a_position, 1.0);
    gl_Position = pos.xyww;
    v_texCoord = a_position.xyz;
}
