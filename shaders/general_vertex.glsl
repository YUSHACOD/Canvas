#version 460 core

layout(location = 0) in vec4 pos;
layout(location = 1) in vec3 scale;
layout(location = 2) in vec4 quat;
layout(location = 3) in vec4 color;

layout(location = 0) uniform mat4 proj;
layout(location = 1) uniform mat4 view;

out VS_OUT {
    vec4 color;
} vs_out;

void main(void) {
    vec4 vertex = vec4(0, 0, 0, 1);

    gl_Position = proj * view * vertex;

    vs_out.color = color;
}
