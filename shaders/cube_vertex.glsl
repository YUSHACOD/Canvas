#version 460 core

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 scale;
layout(location = 2) in vec4 color;
layout(location = 3) in float lerp_offset;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

out VS_OUT {
    vec4 color;
} vs_out;

vec4 quadratic_bezier(vec4 a, vec4 b, vec4 c, float lerp_offset) {
    vec4 d = mix(a, b, lerp_offset);
    vec4 e = mix(b, c, lerp_offset);

    return mix(d, e, lerp_offset);
}

void main(void) {
    const vec4 cube_offsets[8 + 12] = vec4[8 + 12](
            // Side faces
            vec4(-1.0, 1.0, 1.0, 0.0),
            vec4(-1.0, -1.0, 1.0, 0.0),
            vec4(1.0, 1.0, 1.0, 0.0),
            vec4(1.0, -1.0, 1.0, 0.0),
            vec4(1.0, 1.0, -1.0, 0.0),
            vec4(1.0, -1.0, -1.0, 0.0),
            vec4(-1.0, 1.0, -1.0, 0.0),
            vec4(-1.0, -1.0, -1.0, 0.0),

            // Up and down faces
            vec4(-1.0, -1.0, -1.0, 0.0),
            vec4(-1.0, -1.0, 1.0, 0.0),
            vec4(1.0,  -1.0, -1.0, 0.0),

            vec4(-1.0, -1.0,  1.0, 0.0),
            vec4(1.0,  -1.0, -1.0, 0.0),
            vec4(1.0,  -1.0,  1.0, 0.0),

            vec4(-1.0, 1.0, -1.0, 0.0),
            vec4(-1.0, 1.0, 1.0, 0.0),
            vec4(1.0,  1.0, -1.0, 0.0),

            vec4(-1.0, 1.0, 1.0, 0.0),
            vec4(1.0,  1.0, -1.0, 0.0),
            vec4(1.0,  1.0, 1.0, 0.0)
        );

    mat4 cube_scale;
    cube_scale[0][0] = 20.0;
    cube_scale[1][1] = 20.0;
    cube_scale[2][2] = 20.0;
    cube_scale[3][3] = 1.0;

    vec4 a = vec4(-210, 450, -950, 1.0);
    vec4 b = vec4(484, 10, -100, 1.0);
    vec4 c = vec4(-100, -50, -70, 1.0);
    vec4 cube_pos = quadratic_bezier(a, b, c, lerp_offset);

    float t = lerp_offset;

    vec4 d = mix(a, b, lerp_offset);
    vec4 e = mix(b, c, lerp_offset);
    vec4 z4 = d - e;

    // Extract direction part
    vec3 z = normalize(z4.xyz);

    vec3 up = vec3(0.0, 1.0, 0.0);

    // Build orthonormal basis
    vec3 x = normalize(cross(up, z));
    vec3 y = cross(z, x);

    // Rebuild as vec4 (direction vectors → w = 0)
    vec4 X = vec4(x, 0.0);
    vec4 Y = vec4(y, 0.0);
    vec4 Z = vec4(z, 0.0);

    mat4 direction = mat4(X, Y, Z, vec4(0, 0, 0, 1));

    vec4 vertex;
    vec4 cube_offset;

    if (gl_VertexID < 24) {
        int idx = (int(floor(gl_VertexID / 3)) + int((gl_VertexID % 3))) % 8;
        cube_offset = cube_offsets[idx];
    } else {
        cube_offset = cube_offsets[gl_VertexID - 24 + 8];
    }

    vertex = cube_scale * cube_offset;
    vertex = direction * vertex;
    vertex = vertex + cube_pos;

    gl_Position = proj * vertex;
    vs_out.color = vec4(0.5, 0.5, 0.5, 1.0) + (0.5 * cube_offset);
}
