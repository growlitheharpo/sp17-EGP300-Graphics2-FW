/*
	Pass Texcoord
	By Dan Buckstein
	Vertex shader that passes texcoord attribute down pipeline.
	
	Modified by: ______________________________________________________________
*/

// version
#version 410


// ****
// attributes

layout (location = 0) in vec4 position;
layout (location = 3) in vec4 color;

layout (location = 2) in vec4 normal;
layout (location = 8) in vec4 texcoord;

// ****
// uniforms

uniform mat4 mvp;

// ****
// varyings

out v2f {
	vec2 texcoord;
} passData;


// shader function
void main()
{
	// ****
	// set proper clip position
	gl_Position = mvp * position;

	// ****
	// pass data
	passData.texcoord = texcoord.xy;
}