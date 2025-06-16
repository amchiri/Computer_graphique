#version 330 core

layout(location = 0) in vec3 a_position;

uniform mat4 u_projection;
uniform mat4 u_view;

out vec3 v_texCoords;

void main() {
    // Utiliser la position comme coordonnées de texture pour le cubemap
    v_texCoords = a_position;
    
    // Supprimer la translation de la matrice de vue pour que le skybox reste centré
    mat4 viewNoTranslation = mat4(mat3(u_view));
    
    vec4 pos = u_projection * viewNoTranslation * vec4(a_position, 1.0);
    
    // Forcer la profondeur au maximum pour que le skybox soit toujours en arrière-plan
    gl_Position = pos.xyww;
}
