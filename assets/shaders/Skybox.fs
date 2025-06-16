#version 330 core

in vec3 v_texCoords;
out vec4 FragColor;

uniform samplerCube u_skybox;

void main() {
    FragColor = texture(u_skybox, v_texCoords);
}
