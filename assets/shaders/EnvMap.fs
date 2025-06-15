#version 330 core
in vec3 v_normal;
in vec3 v_worldPos;
in vec2 v_uv;
out vec4 FragColor;

uniform samplerCube u_envmap;
uniform sampler2D u_texture;
uniform vec3 u_viewPos;
uniform bool u_useTexture;
uniform bool u_hasTexture;
uniform struct Material {
    vec3 diffuseColor;
    vec3 specularColor;
    float shininess;
    float specularStrength;
} u_material;

void main() {
    vec3 N = normalize(v_normal);
    vec3 I = normalize(v_worldPos - u_viewPos);
    vec3 R = reflect(I, N);
    
    // Échantillonner le cubemap avec le vecteur de réflexion
    vec3 reflectionColor = texture(u_envmap, R).rgb;
    
    // Couleur de base - soit la texture soit la couleur du matériau
    vec3 baseColor = u_material.diffuseColor;
    if (u_useTexture && u_hasTexture) {
        // Si on utilise la texture, on l'échantillonne et on la multiplie avec la couleur de base
        vec3 texColor = texture(u_texture, v_uv).rgb;
        baseColor = texColor * u_material.diffuseColor;
    }
    
    // Calculer l'effet Fresnel pour un rendu plus réaliste
    float fresnel = pow(1.0 - max(dot(normalize(-I), N), 0.0), 3.0);
    float reflectivity = mix(0.1, u_material.specularStrength, fresnel);
    
    // Mélanger la couleur de base avec la réflexion
    vec3 finalColor = mix(baseColor, reflectionColor, reflectivity);
    
    FragColor = vec4(finalColor, 1.0);
}
