/*
	Phong (VS)
	By Dan Buckstein
	Vertex shader that passes data required to perform Phong shading.
	
	Modified by: ______________________________________________________________
*/

// version
#version 410


// ****
// attributes

layout (location = 0) in vec4 position;
layout (location = 2) in vec4 normal;
layout (location = 8) in vec4 texCoord;

// ****
// uniforms
uniform mat4 mvp;
uniform vec4 lightPos;
uniform vec4 eyePos;

// ****
// varyings
out v2f {
	vec4 normal;
	vec4 lightVec;
	vec4 eyeVec;
	vec2 texcoord;
} data;

// shader function
void main()
{
	// ****
	// set clip position
	gl_Position = mvp * position;

	// ****
	// pass data
	data.normal = vec4(normal.xyz, 0.0);

	data.lightVec = lightPos - position;
	data.eyeVec = eyePos - position;

	data.texcoord = texCoord.xy;
}