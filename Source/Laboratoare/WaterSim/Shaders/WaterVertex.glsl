#version 430

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;
layout(location = 3) in vec3 v_color;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform float time;
uniform float material_kd;
uniform float material_ks;
uniform int material_shininess;
uniform vec3 camera_pos;
uniform vec3 light_position;

// out 
out vec2 tex_coord;
out vec3 frg_normal;
out vec3 frag_pos;
out float light_color;
out float height;

vec3 GerstnerWave(vec4 wave, vec3 p, inout vec3 tangent, inout vec3 binormal) {
	float steepness = wave.z;
	float wavelength = wave.w;
	float k = 2 * 3.1415 / wavelength;
	float c = sqrt(9.8 / k);
	vec2 dir = normalize(wave.xy);
	float f = k * (dot(dir, p.xz) - c * 1.5f * time);
	float a = steepness / k;


	tangent += vec3(-dir.x * dir.x * (steepness * sin(f)),
					 dir.x * (steepness * cos(f)),
					-dir.x * dir.y * (steepness * sin(f))
	);

	binormal += vec3(-dir.x * dir.y * (steepness * sin(f)),
				      dir.y * (steepness * cos(f)),
		             -dir.y * dir.y * (steepness * sin(f))
	);

	return vec3(dir.x * (a * cos(f)),
				a * sin(f),
				dir.y * (a * cos(f))
	);
}

float ComputeLight() {
	vec3 world_pos = (Model * vec4(v_position, 1)).xyz;
	vec3 world_normal = normalize(mat3(Model) * normalize(v_normal));

	vec3 N = normalize(world_normal);
	vec3 L = normalize(light_position - world_pos);
	vec3 V = normalize(camera_pos - world_pos);
	vec3 H = normalize(L + V);

	float ambient_light = 0.5f;
	float diffuse_light = material_kd * max(dot(N, L), 0);
	float specular_light = 0.25f;

	if (diffuse_light > 0)
	{
		specular_light = material_ks * pow(max(dot(normalize(N), H), 0), material_shininess);
	}

	return ambient_light + specular_light + diffuse_light;
}

void main()
{
	tex_coord = vec2(v_texture_coord.x / 50 - time / 48, v_texture_coord.y / 50 + time / 48);

	// GERSTNER WAVES
	vec4 Wave1 = vec4(-1, 1, 0.08f, 25f);
	vec4 Wave2 = vec4(1, 0, 0.08f, 17.5f);
	vec4 Wave3 = vec4(1, -1, 0.08f, 15);
	vec4 Wave4 = vec4(-1, -1, 0.08f, 17.5);
	vec4 Wave5 = vec4(0, 1, 0.1f, 17.5);

	// APPLY GERSTNER WAVES
	vec3 planePoint = v_position.xyz;
	vec3 tangent = vec3(1, 0, 0);
	vec3 binormal = vec3(0, 0, 1);
	vec3 p = planePoint;

	p += GerstnerWave(Wave1, planePoint, tangent, binormal);
	p += GerstnerWave(Wave2, planePoint, tangent, binormal);
	p += GerstnerWave(Wave3, planePoint, tangent, binormal);
	p += GerstnerWave(Wave4, planePoint, tangent, binormal);
	p += GerstnerWave(Wave5, planePoint, tangent, binormal);

	// out values
	height = p.y;
	frag_pos = p;
	frg_normal = normalize(cross(binormal, tangent));
	light_color = ComputeLight();

	// RIPPLE EFFECT
	float amplt = 0.15f;
	float freq = 1.5f;
	float distance = length(p);
	float y = amplt * sin(-3.14 * distance * freq + time);

	gl_Position = Projection * View * Model * vec4(p.x, p.y + y, p.z , 1.0);
}