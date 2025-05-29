#version 330 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;

uniform mat4 u_transform;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 v_normal;
out vec2 v_uv;
out vec3 v_position;

void main() {
    // Position en espace monde
    vec4 worldPos = u_transform * vec4(a_position, 1.0);
    v_position = worldPos.xyz;
    
    // Normal en espace monde
    v_normal = normalize(mat3(u_transform) * a_normal);
    v_uv = a_uv;
    
    gl_Position = u_projection * u_view * worldPos;
}