#version 460 core

out vec4 color;

uniform sampler2D tx;

in VS_OUT {
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
} fs_in;

vec3 calcLighting(vec3 normal, vec3 light_dir, vec3 light_color) {
    vec3 ambient = light_color * fs_in.ambient;

    float incident_angle = dot(normal, light_dir);
    float diff = max(incident_angle, 0.0);
    vec3 diffuse = light_color * (diff * fs_in.diffuse);

    return ambient + diffuse;
}

void main(void) {
    vec3 normal = normalize(fs_in.normal);
    vec3 light_dir = normalize(fs_in.light_pos - fs_in.frag_pos);

    vec3 view_dir = normalize(-fs_in.frag_pos);
    vec3 reflect_dir = reflect(-light_dir, normal);

    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), fs_in.shine);
    vec3 specular = fs_in.light_color * (spec * fs_in.specular);

    vec3 light = calcLighting(normal, light_dir, fs_in.light_color) + specular;
    // vec3 light = calcLighting(normal, light_dir, fs_in.light_color);

    // color = texture(tx, fs_in.texture_coord) * vec4(light, 1.0);
    color = vec4(light, 1.0);
}
