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
    
    // Direction de la lumière venant du soleil vers le fragment
    vec3 lightVec = u_light.direction - v_position;
    float dist = length(lightVec);
    vec3 lightDir = normalize(lightVec);  // Normaliser pour avoir uniquement la direction
    
    // Calcul de l'éclairage diffus avec cut-off pour l'ombre
    float diff = max(dot(norm, lightDir), 0.0);
    
    // Lumière ambiante très faible pour les zones dans l'ombre
    vec3 ambient = vec3(0.05) * texture(u_texture, v_uv).rgb;
    
    // Calcul diffus plus prononcé
    vec3 diffuse = u_light.diffuseColor * diff * texture(u_texture, v_uv).rgb;
    
    // Calcul spéculaire uniquement si la surface est éclairée
    vec3 viewDir = normalize(u_viewPos - v_position);
    vec3 specular = vec3(0.0);
    float specularStrength = 0.5;
    if (diff > 0.0) {
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_material.shininess);
        specular = specularStrength * spec * u_light.specularColor;
    }
    
    // Atténuation quadratique plus prononcée avec la distance
    float constant = 1.0;
    float linear = 0.09;
    float quadratic = 0.032;
    float attenuation = 1.0 / (constant + linear * dist + quadratic * (dist * dist));
    
    // Combiner les composantes
    vec3 result = (ambient + (diffuse + specular) * attenuation) * u_intensity;
    
    // Tone mapping HDR
    result = result / (result + vec3(1.0));
    result = pow(result, vec3(1.0/2.2)); // Correction gamma
    
    FragColor = vec4(result, 1.0);
}