#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform sampler2D skyboxTexture;

void main()
{
    vec2 uv;
    // Conversion des coordonnées du cube en coordonnées sphériques
    vec3 normalizedCoords = normalize(TexCoords);
    uv.x = 0.5 + (atan(normalizedCoords.z, normalizedCoords.x) / (2.0 * 3.1415926));
    uv.y = 0.5 + (asin(normalizedCoords.y) / 3.1415926);
    
    FragColor = texture(skyboxTexture, uv);
}
