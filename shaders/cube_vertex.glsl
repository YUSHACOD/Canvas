#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 scale;
layout(location = 2) in vec4 quat;
layout(location = 3) in vec4 color;

layout(location = 0) uniform mat4 proj;
layout(location = 1) uniform mat4 view;
layout(location = 2) uniform vec3 light_pos;
layout(location = 3) uniform vec3 light_color;
layout(location = 4) uniform vec3 ambient;
layout(location = 5) uniform vec3 diffuse;
layout(location = 6) uniform vec3 specular;
layout(location = 7) uniform float shine;

out VS_OUT {
    vec3 light_pos;
    vec3 light_color;
    vec3 frag_pos;
    vec3 normal;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shine;
    vec4 color;
    vec2 texture_coord;
} vs_out;

// Position table for cube corners
const vec3 positions[8] = vec3[](
        vec3(-0.5, -0.5, -0.5), vec3(0.5, -0.5, -0.5),
        vec3(0.5, 0.5, -0.5), vec3(-0.5, 0.5, -0.5),
        vec3(-0.5, -0.5, 0.5), vec3(0.5, -0.5, 0.5),
        vec3(0.5, 0.5, 0.5), vec3(-0.5, 0.5, 0.5)
    );

// Index table for each face (6 indices per face, 6 faces = 36 total)
const int indices[36] = int[](
        0, 1, 2, 2, 3, 0, // back
        4, 5, 6, 6, 7, 4, // front
        0, 1, 5, 5, 4, 0, // bottom
        2, 3, 7, 7, 6, 2, // top
        0, 3, 7, 7, 4, 0, // left
        1, 2, 6, 6, 5, 1 // right
    );

const vec2 tex_coords[6] = vec2[](
        vec2(0, 0), vec2(1, 0), vec2(1, 1), // left triangle
        vec2(1, 1), vec2(0, 1), vec2(0, 0) // right triangle
    );

mat4 createModelMatrix(vec3 pos, vec3 scale, vec4 quat) {
    // Assumes quat is already normalized
    float xx = quat.x * quat.x, yy = quat.y * quat.y, zz = quat.z * quat.z;
    float xy = quat.x * quat.y, xz = quat.x * quat.z, yz = quat.y * quat.z;
    float wx = quat.w * quat.x, wy = quat.w * quat.y, wz = quat.w * quat.z;

    vec3 c0 = vec3(1.0 - 2.0 * (yy + zz), 2.0 * (xy + wz), 2.0 * (xz - wy)) * scale.x;
    vec3 c1 = vec3(2.0 * (xy - wz), 1.0 - 2.0 * (xx + zz), 2.0 * (yz + wx)) * scale.y;
    vec3 c2 = vec3(2.0 * (xz + wy), 2.0 * (yz - wx), 1.0 - 2.0 * (xx + yy)) * scale.z;

    return mat4(
        vec4(c0, 0.0),
        vec4(c1, 0.0),
        vec4(c2, 0.0),
        vec4(pos, 1.0)
    );
}

vec3 getFaceNormal(int face) {
    int base = face * 6;

    return positions[indices[base]]
        + positions[indices[base + 1]]
        + positions[indices[base + 2]]
        + positions[indices[base + 4]];
}

void main(void) {
    mat4 transform = createModelMatrix(pos, scale, quat);
    vec4 vertex = transform * vec4(positions[indices[gl_VertexID]], 1.0);

    gl_Position = proj * view * vertex;

    mat3 normal_matrix = mat3(transpose(inverse(view * transform)));
    int face = gl_VertexID / 6;

    vs_out.color = color;
    vs_out.texture_coord = tex_coords[gl_VertexID % 6];

    vs_out.frag_pos = vec3(view * vertex);
    vs_out.normal = normal_matrix * normalize(getFaceNormal(face));
    vs_out.light_pos = vec3(view * vec4(light_pos, 1.0));
    vs_out.light_color = light_color;

    vs_out.ambient = ambient;
    vs_out.diffuse = diffuse;
    vs_out.specular = specular;
}
