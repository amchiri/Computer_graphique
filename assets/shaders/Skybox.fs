#version 330 core

in vec3 v_texCoord;
uniform sampler2D u_texture;
out vec4 FragColor;

void main() {
    vec3 texCoord = normalize(v_texCoord);
    vec2 uv;
    
    // Convert from Cartesian to spherical coordinates
    float phi = atan(texCoord.z, texCoord.x);
    float theta = acos(texCoord.y);
    
    // Map to UV coordinates
    uv.x = (phi + 3.1415926) / (2.0 * 3.1415926);
    uv.y = theta / 3.1415926;
    
    FragColor = texture(u_texture, uv);
}
