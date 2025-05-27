#version 330 core

in vec3 v_normal;
in vec2 v_uv;
in vec3 v_position;

struct Material {
    vec3 diffuseColor;
    vec3 specularColor;
    float shininess;
    bool isEmissive;
    float emissiveIntensity;
    vec3 lightColor;
    float specularStrength;  // Ajouté pour correspondre au calcul
};

// Définir une structure pour les lumières émissives
struct EmissiveLight {
    vec3 position;
    vec3 color;
    float intensity;
};

uniform Material u_material;
uniform sampler2D u_texture;
uniform vec3 u_viewPos;
uniform bool u_hasTexture;  // Ajout d'un uniform pour gérer la présence de texture

// Nouveau uniform pour recevoir les lumières émissives
#define MAX_EMISSIVE_LIGHTS 10
uniform EmissiveLight u_emissiveLights[MAX_EMISSIVE_LIGHTS];
uniform int u_numEmissiveLights;

out vec4 FragColor;

vec3 CalculateLight(vec3 normal, vec3 fragPos, vec3 lightPos, vec3 lightColor, float lightIntensity) {
    vec3 lightDir = normalize(lightPos - fragPos);
    
    // Diffuse plus intense
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Ajustement de la spécularité - ne pas remplacer la shininess par une valeur fixe
    vec3 viewDir = normalize(u_viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), max(u_material.shininess, 1.0)); // Utiliser au moins 1.0
    vec3 specular = spec * lightColor * u_material.specularStrength;

    // Atténuation moins forte
    float distance = length(lightPos - fragPos);
    float attenuation = 1.0 / (1.0 + 0.045 * distance + 0.0075 * distance * distance);
    
    return (diffuse + specular) * attenuation * lightIntensity;
}

void main() {
    // Utiliser la couleur de base si pas de texture
    vec4 texColor = u_hasTexture ? texture(u_texture, v_uv) : vec4(u_material.diffuseColor, 1.0);
    vec3 norm = normalize(v_normal);
    
    if (u_material.isEmissive) {
        // Les objets émissifs ne devraient pas être affectés par la shininess
        vec3 emissiveColor = u_material.lightColor * u_material.emissiveIntensity * texColor.rgb;
        FragColor = vec4(emissiveColor, texColor.a);
        return;
    }

    // Lumière ambiante de base
    vec3 ambient = vec3(0.15) * texColor.rgb * u_material.diffuseColor;
    vec3 result = ambient;
    
    // Lumière diffuse et spéculaire seulement pour les objets non émissifs
    for(int i = 0; i < u_numEmissiveLights; i++) {
        vec3 lightContrib = CalculateLight(
            norm, 
            v_position,
            u_emissiveLights[i].position,
            u_emissiveLights[i].color,
            u_emissiveLights[i].intensity
        );
        
        result += lightContrib * texColor.rgb * u_material.diffuseColor;
    }
    
    result = pow(result, vec3(1.0/2.2)); // Correction gamma
    FragColor = vec4(result, texColor.a);
}