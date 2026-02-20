#version 460 core

layout(location = 0) in vec4 color;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

out VS_OUT {
    vec4 color;
} vs_out;

void main(void) {
    vec4 vertex = vec4(0, 0, 0, 1);

    gl_Position = proj * view * world * vertex;

    vs_out.color = color;
}
