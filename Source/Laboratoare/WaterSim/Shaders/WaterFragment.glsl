#version 430

layout(location = 0) out vec4 out_color;

uniform sampler2D texture_1;
uniform samplerCube texture_cubemap;
uniform vec3 camera_pos;

in vec2 tex_coord;
in vec3 frg_normal;
in vec3 frag_pos;
in float height;
in float light_color;

void main()
{
	float interpolate = clamp(height / 10f, -1, 1);
	vec3 incident = frag_pos - camera_pos;

	vec4 color = texture2D(texture_1, tex_coord);
	vec3 cube_color = texture(texture_cubemap, reflect(incident, frg_normal)).xyz;
	// cube_color += texture(texture_cubemap, refract(incident, frg_normal, 1.33)).xyz;

	color = mix(color, vec4(cube_color, 1), interpolate * 1.5f);
	color = mix(color, vec4(1, 1, 1, 1), interpolate);
	out_color = color * light_color;
}