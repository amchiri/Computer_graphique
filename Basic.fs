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
    vec4 texColor = texture(u_texture, v_uv);
    
    if (u_material.isEmissive) {
        // Multiplier la couleur émissive par la texture
        FragColor = vec4(u_material.diffuseColor * texColor.rgb, 1.0);
        return;
    }
    
    // Direction de la lumière du soleil vers le fragment
    vec3 lightDir = normalize(u_light.direction - v_position);
    float dist = length(u_light.direction - v_position);
    
    // Atténuation plus progressive
    float attenuationFactor = 1.0 / (1.0 + 0.005 * dist);
    
    vec3 norm = normalize(v_normal);
    float diff = max(dot(norm, lightDir), 0.0);
    
    // Lumière ambiante plus forte pour compenser
    vec3 ambient = vec3(0.2) * texColor.rgb;
    
    // Lumière diffuse avec texture
    vec3 diffuse = u_light.diffuseColor * diff * texColor.rgb * attenuationFactor;
    
    // Spéculaire
    vec3 viewDir = normalize(u_viewPos - v_position);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_material.shininess);
    vec3 specular = u_light.specularColor * spec * attenuationFactor;
    
    vec3 result = (ambient + diffuse + specular) * u_intensity;
    result = pow(result, vec3(1.0/2.2));  // Correction gamma
    
    FragColor = vec4(result, texColor.a);
}