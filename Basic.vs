attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec2 a_uv;

uniform mat4 u_transform;
uniform mat4 u_projection;

varying vec4 v_color;
varying vec2 v_uv;
varying vec3 v_normal; 
varying vec3 v_position;

void main(void) {
    // The perspective projection is applied last.
    gl_Position = u_projection * u_transform * vec4(a_position, 1.0);
   
    v_normal = (u_transform * vec4(a_normal, 0.0)).xyz;
    v_position = (u_transform * vec4(a_position, 1.0)).xyz;
   
    v_color = vec4(a_normal, 1.0);
    v_uv = vec2(a_uv.s, 1.0 - a_uv.t);
}