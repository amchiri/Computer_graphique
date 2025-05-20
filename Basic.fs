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
    vec3 lightVec = u_light.direction - v_position;
    float dist = length(lightVec);
    vec3 lightDir = normalize(lightVec);
    vec3 viewDir = normalize(u_viewPos - v_position);
    
    // Utiliser l'intensité de la lumière
    vec3 adjustedLightColor = u_light.diffuseColor * u_intensity;
    
    // Ambient avec la couleur de la lumière
    vec3 ambient = vec3(0.1) * adjustedLightColor * texture(u_texture, v_uv).rgb;
    
    // Diffuse avec la couleur et l'intensité de la lumière
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = adjustedLightColor * u_material.diffuseColor * diff * texture(u_texture, v_uv).rgb;
    
    // Specular avec la couleur et l'intensité de la lumière
    vec3 H = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, H), 0.0), u_material.shininess);
    vec3 specular = u_light.specularColor * u_material.specularColor * spec;
    
    // Atténuation basée sur la distance
    float attenuation = 1.0 / (1.0 + 0.045 * dist + 0.0075 * dist * dist);
    
    vec3 result = ambient + (diffuse + specular) * attenuation;
    
    // HDR tone mapping
    result = result / (result + vec3(1.0));
    result = pow(result, vec3(1.0/2.2));  // gamma correction
    
    FragColor = vec4(result, 1.0);
}