#version 460 core

layout(location = 0) in vec4 offset;
layout(location = 1) in vec4 color;
layout(location = 2) in vec4 rot2D;

uniform mat4 proj;

out VS_OUT {
    vec4 color;
} vs_out;

void main(void) {
    const vec4 vertices[7] = vec4[7](
            vec4(0.0, 0.0, 0.5, 1.0),
            vec4(0.25, 0.0, 0.5, 1.0),
            vec4(0.0, 0.25, 0.5, 1.0),
            vec4(0.25, 0.0, 0.5, 1.0),
            vec4(0.0, 0.25, 0.5, 1.0),
            vec4(0.25, 0.25, 0.5, 1.0),
            vec4(0.0, 0.0, 0.5, 1.0)
        );

	vec4 vertex = vertices[gl_VertexID] + offset;

	if (gl_VertexID == 6) {
		vertex = vertex + rot2D - offset;
	}
    gl_Position = proj * vertex;

    vs_out.color = color;
}
