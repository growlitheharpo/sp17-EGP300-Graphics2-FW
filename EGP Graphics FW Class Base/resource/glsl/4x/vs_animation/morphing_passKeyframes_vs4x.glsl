/*
	Morphing - Pass Keyframes
	By Dan Buckstein
	Vertex shader that passes keyframes for morphing to geometry shader.
	
	Modified by: ______________________________________________________________
*/

#version 410


// ****
// attributes: one per morphing attribute (e.g. multiple positions)
layout (location = 0) in vec4 position;

// ****
// varying output: pass all attributes
out vertexdata
{
	vec4 position;
} pass;

void main()
{
	// ****
	// pass data along
	pass.position = position;
}