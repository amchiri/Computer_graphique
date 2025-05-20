#version 330 core

in vec3 v_normal;
in vec2 v_uv;
in vec3 v_position;

// Structures pour la lumière et le matériau
struct Light {
    vec3 direction;
    vec3 diffuseColor;
    vec3 specularColor;
};

struct Material {
    vec3 diffuseColor;
    vec3 specularColor;
    float shininess;
    bool isEmissive;
};

uniform Light u_light;
uniform Material u_material;
uniform sampler2D u_texture;
uniform vec3 u_viewPos;

out vec4 FragColor;

void main() {
    if (u_material.isEmissive) {
        // Pour les objets émissifs, utiliser la couleur diffuse directement
        FragColor = vec4(u_material.diffuseColor, 1.0);
        return;
    }
    
    // Calculs d'éclairage normaux pour les objets non-émissifs
    vec3 norm = normalize(v_normal);
    
    // La direction de la lumière doit être calculée du point vers le soleil
    vec3 lightDir = normalize(u_light.direction - v_position);
    vec3 viewDir = normalize(u_viewPos - v_position);
    
    // Diffuse avec atténuation par la distance
    float dist = length(u_light.direction - v_position);
    float attenuation = 1.0 / (1.0 + 0.09 * dist + 0.032 * dist * dist);
    
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = u_light.diffuseColor * u_material.diffuseColor * diff * attenuation;
    
    // Specular
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_material.shininess);
    vec3 specular = u_light.specularColor * u_material.specularColor * spec * attenuation;
    
    // Ambient
    vec3 ambient = vec3(0.1) * u_material.diffuseColor;
    
    // Texture
    vec4 texColor = texture(u_texture, v_uv);
    
    // Combinaison finale
    vec3 result = (ambient + diffuse + specular) * texColor.rgb;
    FragColor = vec4(result, texColor.a);
}