#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 scale;
layout(location = 2) in vec4 quat;
layout(location = 3) in vec4 color;

layout(location = 0) uniform mat4 proj;
layout(location = 1) uniform mat4 view;

out VS_OUT {
    vec4 color;
} vs_out;


mat4 createModelMatrix(vec3 pos, vec3 scale, vec4 quat) {
    // Assumes quat is already normalized
    float xx = quat.x * quat.x;
    float yy = quat.y * quat.y;
    float zz = quat.z * quat.z;
    float xy = quat.x * quat.y;
    float xz = quat.x * quat.z;
    float yz = quat.y * quat.z;
    float wx = quat.w * quat.x;
    float wy = quat.w * quat.y;
    float wz = quat.w * quat.z;
    
    return mat4(
        (1.0 - 2.0 * (yy + zz)) * scale.x,
        (2.0 * (xy + wz)) * scale.x,
        (2.0 * (xz - wy)) * scale.x,
        0.0,
        
        (2.0 * (xy - wz)) * scale.y,
        (1.0 - 2.0 * (xx + zz)) * scale.y,
        (2.0 * (yz + wx)) * scale.y,
        0.0,
        
        (2.0 * (xz + wy)) * scale.z,
        (2.0 * (yz - wx)) * scale.z,
        (1.0 - 2.0 * (xx + yy)) * scale.z,
        0.0,
        
        pos.x, pos.y, pos.z, 1.0
    );
}

void main(void) {
    // Position table for cube corners
    const vec3 positions[8] = vec3[](
            vec3(-0.5, -0.5, -0.5), vec3(0.5, -0.5, -0.5),
            vec3(0.5, 0.5, -0.5), vec3(-0.5, 0.5, -0.5),
            vec3(-0.5, -0.5, 0.5), vec3(0.5, -0.5, 0.5),
            vec3(0.5, 0.5, 0.5), vec3(-0.5, 0.5, 0.5)
        );

    // Index table for each face
    const int indices[36] = int[](
            0, 1, 2, 2, 3, 0, // back
            4, 5, 6, 6, 7, 4, // front
            0, 1, 5, 5, 4, 0, // bottom
            2, 3, 7, 7, 6, 2, // top
            0, 3, 7, 7, 4, 0, // left
            1, 2, 6, 6, 5, 1 // right
        );

    vec3 vert = positions[indices[gl_VertexID]];

	mat4 transform = createModelMatrix(pos, scale, quat);
    vec4 vertex = transform * vec4(vert, 1.0);

    gl_Position = proj * view * vertex;

	// Gradient Cube
    vs_out.color = vec4(0.5, 0.5, 0.5, 0.0) + vec4(vert, 1.0);

    // vs_out.color = color;
}
