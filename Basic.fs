varying vec4 v_color;
varying vec2 v_uv;
varying vec3 v_position;
varying vec3 v_normal;

// Structures pour la lumière et le matériau
struct Light {
    vec3 direction;    // Direction de la lumière (L)
    vec3 diffuseColor; // Couleur/intensité diffuse de la lumière (Id)
    vec3 specularColor;// Couleur/intensité spéculaire de la lumière (Is)
};

struct Material {
    vec3 diffuseColor;  // Coefficient de réflexion diffuse (Kd)
    vec3 specularColor; // Coefficient de réflexion spéculaire (Ks)
    float shininess;    // Exposant spéculaire
};

uniform Light u_light;
uniform Material u_material;
uniform sampler2D u_texture;
uniform vec3 u_viewPos;

vec3 diffuse(vec3 N, vec3 L) {
    float NdotL = max(dot(N, L), 0.0);
    return u_light.diffuseColor * u_material.diffuseColor * NdotL;
}

vec3 specular(vec3 N, vec3 L, vec3 V) {
    vec3 R = reflect(-L, N);
    float RdotV = max(dot(R, V), 0.0);
    float spec = pow(RdotV, u_material.shininess);
    return u_light.specularColor * u_material.specularColor * spec;
}

void main(void) {
    vec3 N = normalize(v_normal);
    vec3 L = normalize(u_light.direction);
    vec3 V = normalize(u_viewPos - v_position);
    
    vec3 diffuseColor = diffuse(N, L);
    vec3 specularColor = specular(N, L, V);
    vec4 textureColor = texture2D(u_texture, v_uv);
    
    vec3 finalColor = (diffuseColor + specularColor) * textureColor.rgb;
    gl_FragColor = vec4(finalColor, textureColor.a);
}