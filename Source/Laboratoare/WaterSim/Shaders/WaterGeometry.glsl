#version 430
layout(lines) in;
layout(triangle_strip, max_vertices = 170) out;

uniform mat4 View;
uniform mat4 Projection;
uniform vec3 control_p1, control_p2, control_p3, control_p4;
uniform int no_of_instances;
uniform int no_of_generated_points;
uniform float max_translate;
uniform float y_variation;

in int instance[2];
in VertexData{
	vec2 TexCoord;
} VertexIn[];
//out vec2 tex_Coord;
out VertexData{
	vec2 mTexCoord;
} VertexOut;

vec3 bezier(float t)
{
	return control_p1 * pow((1 - t), 3) + control_p2 * 3 * t * pow((1 - t), 2) + control_p3 * 3 * pow(t, 2) * (1 - t) + control_p4 * pow(t, 3);
}

void main()
{
	float dt = 1.0 / float(no_of_generated_points);
	float translate = max_translate / float(no_of_instances);

	if (instance[0] < no_of_instances)
	{

		for (float t = 0.0; t < 1; t += dt) {
			vec3 bez = bezier(t);
			vec3 bez_next = bezier(t + dt);

			vec3 p1 = bez;
			vec3 p2 = bez_next;
			vec3 d1 = instance[0] * vec3(0, y_variation, translate);
			vec3 d2 = (instance[0] + 1) * vec3(0, y_variation, translate);

			vec3 p1f = p1 + d2;
			vec3 p2f = p1 + d1;
			vec3 p3f = p2 + d1;
			vec3 p4f = p2 + d2;

			gl_Position = Projection * View * vec4(p1f, 1);
			VertexOut.mTexCoord = vec2(0, 1);
			//tex_Coord = tex_coord;
			EmitVertex();

			gl_Position = Projection * View * vec4(p2f, 1);
			VertexOut.mTexCoord = vec2(0, 0);
			//tex_Coord = tex_coord;
			EmitVertex();

			gl_Position = Projection * View * vec4(p3f, 1);
			VertexOut.mTexCoord = vec2(1, 0);
			//tex_Coord = tex_coord;
			EmitVertex();

			EndPrimitive();

			gl_Position = Projection * View * vec4(p3f, 1); 
			VertexOut.mTexCoord = vec2(1, 0);
			//tex_Coord = tex_coord;
			EmitVertex();

			gl_Position = Projection * View * vec4(p1f, 1);
			VertexOut.mTexCoord = vec2(0, 1);
			//tex_Coord = tex_coord;
			EmitVertex();

			gl_Position = Projection * View * vec4(p4f, 1);
			VertexOut.mTexCoord = vec2(1, 1);
			//tex_Coord = tex_coord;
			EmitVertex();

			EndPrimitive();
		}

	}
	
}
