#version 330 core

uniform vec3 u_color;
uniform bool u_useTexture;
uniform sampler2D u_texture;
uniform bool u_hasTexture;

in vec2 v_uv;
out vec4 FragColor;

void main() {
    vec3 finalColor = u_color;
    
    if (u_useTexture && u_hasTexture) {
        vec3 texColor = texture(u_texture, v_uv).rgb;
        finalColor = texColor * u_color;
    }
    
    FragColor = vec4(finalColor, 1.0);
}
