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

// Ajouter ces uniformes
uniform float u_time;
uniform float u_intensity;

out vec4 FragColor;

void main() {
    if (u_material.isEmissive) {
        FragColor = vec4(u_material.diffuseColor * 2.0, 1.0);
        return;
    }
    
    vec3 norm = normalize(v_normal);
    
    // Calcul de la direction de la lumière en fonction de la position du soleil
    vec3 lightVec = normalize(u_light.direction - v_position);
    float dist = length(u_light.direction - v_position);
    
    // Atténuation en fonction de la distance
    float attenuationFactor = 1.0 / (1.0 + 0.01 * dist + 0.001 * dist * dist);
    
    // Calcul de l'éclairage diffus avec cut-off pour l'ombre
    float diff = max(dot(norm, lightVec), 0.0);
    
    // Lumière ambiante ajustée pour les zones dans l'ombre
    vec3 ambient = vec3(0.1) * texture(u_texture, v_uv).rgb;
    
    // Calcul diffus avec atténuation de distance
    vec3 diffuse = u_light.diffuseColor * diff * texture(u_texture, v_uv).rgb * attenuationFactor;
    
    // Calcul spéculaire amélioré
    vec3 viewDir = normalize(u_viewPos - v_position);
    vec3 reflectDir = reflect(-lightVec, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_material.shininess);
    vec3 specular = u_light.specularColor * spec * attenuationFactor;
    
    // Combiner les composantes avec intensité
    vec3 result = (ambient + diffuse + specular) * u_intensity;
    
    // Tone mapping HDR amélioré
    result = result / (result + vec3(1.0));
    result = pow(result, vec3(1.0/2.2)); // Correction gamma
    
    FragColor = vec4(result, 1.0);
}