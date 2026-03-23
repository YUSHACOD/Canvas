#version 460 core

out vec4 color;

uniform sampler2D tx;

in VS_OUT {
    vec4 color;
	vec2 texture_coord;
} fs_in;

void main(void) {
    color = texture(tx, fs_in.texture_coord) * fs_in.color; 
}
