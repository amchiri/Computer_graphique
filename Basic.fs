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
        // Effet de halo pour le soleil avec une couleur plus vive
        FragColor = vec4(u_material.diffuseColor * 2.0, 1.0);
        return;
    }
    
    // Calculs d'éclairage normaux pour les objets non-émissifs
    vec3 norm = normalize(v_normal);
    
    // Vecteur du point vers le soleil
    vec3 lightVec = u_light.direction - v_position;
    float lightDistance = length(lightVec);
    vec3 lightDir = lightVec / lightDistance;
    vec3 viewDir = normalize(u_viewPos - v_position);
    
    // Atténuation quadratique améliorée
    float constant = 1.0;
    float linear = 0.014;
    float quadratic = 0.0007;
    float attenuation = 1.0 / (constant + linear * lightDistance + quadratic * lightDistance * lightDistance);
    
    // Composante diffuse avec intensité ajustée
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = u_light.diffuseColor * u_material.diffuseColor * diff * attenuation * 2.0;
    
    // Spéculaire avec effet Blinn-Phong
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), u_material.shininess * 2.0);
    vec3 specular = u_light.specularColor * u_material.specularColor * spec * attenuation;
    
    // Ambiant avec intensité ajustée selon la distance
    float ambientStrength = 0.15 * (1.0 - min(lightDistance/50.0, 0.5));
    vec3 ambient = ambientStrength * u_material.diffuseColor;
    
    // Texture et combinaison finale
    vec4 texColor = texture(u_texture, v_uv);
    vec3 result = (ambient + diffuse + specular) * texColor.rgb;
    
    // Tone mapping simple pour meilleure exposition
    result = result / (result + vec3(1.0));
    result = pow(result, vec3(1.0/2.2));  // Correction gamma
    
    FragColor = vec4(result, texColor.a);
}