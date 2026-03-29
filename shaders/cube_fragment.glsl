#version 460 core

out vec4 color;

uniform sampler2D tx;

in VS_OUT {
    vec3 light_pos;
    vec3 light_color;
    vec3 frag_pos;
    vec3 normal;
    vec4 color;
    vec2 texture_coord;
} fs_in;

const float ambience_factor = 0.2;
const float specular_strength = 0.5;

vec3 calcLighting(vec3 normal, vec3 light_dir, vec3 light_color) {

    vec3 ambient = ambience_factor * light_color;
	
	float incident_angle = dot(normal, light_dir);
    vec3 diffuse = max(incident_angle, 0.0) * light_color;

    return ambient + diffuse;
}

void main(void) {
    vec3 normal = normalize(fs_in.normal);
    vec3 light_dir = normalize(fs_in.light_pos - fs_in.frag_pos);

	vec3 view_dir = normalize(-fs_in.frag_pos);
	vec3 reflect_dir = reflect(-light_dir, normal);

	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 256);
	vec3 specular = specular_strength * spec * fs_in.light_color;

    vec3 light = calcLighting(normal, light_dir, fs_in.light_color) + specular;
    // vec3 light = calcLighting(normal, light_dir, fs_in.light_color);

    color = fs_in.color * vec4(light, 1.0);
}
