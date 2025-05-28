#version 330 core

in vec3 v_normal;
in vec3 v_position;

struct Material {
    vec3 diffuseColor;
    vec3 specularColor;
    float shininess;
    bool isEmissive;
};

uniform Material u_material;
uniform float u_intensity;
uniform vec3 u_viewPos;

out vec4 FragColor;

void main() {
    vec3 color = u_material.diffuseColor;
    
    if (u_material.isEmissive) {
        // Si l'objet est émissif, utiliser directement la couleur
        FragColor = vec4(color, 1.0);
    } else {
        // Sinon, appliquer un éclairage simple
        vec3 ambient = color * 0.3;
        vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
        float diff = max(dot(normalize(v_normal), lightDir), 0.0);
        vec3 diffuse = color * diff;
        
        vec3 result = (ambient + diffuse) * u_intensity;
        result = pow(result, vec3(1.0/2.2));  // Correction gamma
        
        FragColor = vec4(result, 1.0);
    }
}
