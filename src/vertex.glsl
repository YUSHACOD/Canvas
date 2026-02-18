#version 460 core

layout(location = 0) in vec4 offset;
layout(location = 1) in vec4 color;
layout(location = 2) in vec4 rot2D;
layout(location = 3) in vec4 input_pos;
layout(location = 4) in float t;

uniform mat4 proj;

out VS_OUT {
    vec4 color;
} vs_out;

vec4 quadratic_bezier(vec4 a, vec4 b, vec4 c, float t) {
    vec4 d = mix(a, b, t);
    vec4 e = mix(b, c, t);

    vec4 p = mix(d, e, t);

    return p;
}

void main(void) {
    const vec4 cube_offsets[24] = vec4[24](
            // Upper face
            vec4(1.0, 1.0, 1.0, 0.0),
            vec4(1.0, 1.0, -1.0, 0.0),

            vec4(1.0, 1.0, -1.0, 0.0),
            vec4(-1.0, 1.0, -1.0, 0.0),

            vec4(-1.0, 1.0, -1.0, 0.0),
            vec4(-1.0, 1.0, 1.0, 0.0),

            vec4(-1.0, 1.0, 1.0, 0.0),
            vec4(1.0, 1.0, 1.0, 0.0),

            // Lower face
            vec4(1.0, -1.0, 1.0, 0.0),
            vec4(1.0, -1.0, -1.0, 0.0),

            vec4(1.0, -1.0, -1.0, 0.0),
            vec4(-1.0, -1.0, -1.0, 0.0),

            vec4(-1.0, -1.0, -1.0, 0.0),
            vec4(-1.0, -1.0, 1.0, 0.0),

            vec4(-1.0, -1.0, 1.0, 0.0),
            vec4(1.0, -1.0, 1.0, 0.0),

            // Remaining edges
            vec4(1.0, 1.0, 1.0, 0.0),
            vec4(1.0, -1.0, 1.0, 0.0),

            vec4(1.0, 1.0, -1.0, 0.0),
            vec4(1.0, -1.0, -1.0, 0.0),

            vec4(-1.0, 1.0, -1.0, 0.0),
            vec4(-1.0, -1.0, -1.0, 0.0),

            vec4(-1.0, 1.0, 1.0, 0.0),
            vec4(-1.0, -1.0, 1.0, 0.0)
        );

    mat4 cube_scale;
    cube_scale[0][0] = 0.25;
    cube_scale[1][1] = 0.25;
    cube_scale[2][2] = 0.25;
    cube_scale[3][3] = 1.0;

    const vec4 vertices[7] = vec4[7](
            vec4(0.0, 0.0, 0.5, 1.0),
            vec4(0.25, 0.0, 0.5, 1.0),
            vec4(0.0, 0.25, 0.5, 1.0),
            vec4(0.25, 0.0, 0.5, 1.0),
            vec4(0.0, 0.25, 0.5, 1.0),
            vec4(0.25, 0.25, 0.5, 1.0),
            vec4(0.0, 0.0, 0.5, 1.0)
        );

    vec4 cube_pos = quadratic_bezier(
            vec4(-1.00, 1.0, 1.0, 1.0),
            vec4(1.0, 1.0, 0.0, 1.0),
            vec4(-1.00, -1.0, -1.0, 1.0),
            t
        );

    vec4 vertex;
    if (gl_VertexID > 6) {
        vertex = cube_offsets[gl_VertexID - 7] + cube_pos + input_pos;
        vertex = cube_scale * vertex;
    } else if (gl_VertexID == 6) {
        vertex = vertices[gl_VertexID] + rot2D + input_pos;
    } else {
        vertex = vertices[gl_VertexID] + offset + input_pos;
    }

    gl_Position = proj * vertex;

    vs_out.color = mix(vec4(1.0, 0.5, 0.0, 1.0), vec4(0.0, 0.5, 1.0, 1.0), t);
}
