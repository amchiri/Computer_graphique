#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 u_projection;
uniform mat4 u_view;

void main()
{
    TexCoords = aPos;
    vec4 pos = u_projection * mat4(mat3(u_view)) * vec4(aPos * 100.0, 1.0);
    gl_Position = pos.xyww;
}
