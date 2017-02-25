/*
	Cel (FS)
	By Dan Buckstein (kind of)
	Fragment shader that computes Cel shading.
	
	Modified by: James Keats
*/

// version
#version 410


// ****
// varyings
in VertexData
{
	vec4 normal;
	vec4 lightVec;
	vec4 eyeVec;
	vec4 texcoord;
} pass;


// ****
// uniforms
uniform sampler2D tex_dm;	//texture 0
uniform sampler2D tex_sm;	//texture 1


// ****
// target
layout (location = 0) out vec4 fragColor;


const vec4 AMBIENT = vec4(0.08, 0.04, 0.02, 1.0);

// shader function
void main()
{
	
	// 1. diffuse
	vec4 N = normalize(pass.normal);
	vec4 L = normalize(pass.lightVec);
	float kd = dot(N, L);

	// 3. clamp & exponent
	kd = max(0.0, kd);

	vec4 texColor = (texture(tex_sm, vec2(kd, 0.5)) * (kd * 0.8 + 0.2));
	
	fragColor = texture(tex_dm, pass.texcoord.xy) * texColor;
}