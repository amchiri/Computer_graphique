#version 330 core
in vec3 v_normal;
in vec3 v_worldPos;
out vec4 FragColor;
uniform samplerCube u_envmap;
uniform vec3 u_viewPos;
void main() {
    vec3 I = normalize(v_worldPos - u_viewPos);
    vec3 R = reflect(I, normalize(v_normal));
    vec3 envColor = vec3(0.7, 0.8, 1.0); // Couleur de fallback si pas de cubemap
    FragColor = vec4(envColor, 1.0);
}
