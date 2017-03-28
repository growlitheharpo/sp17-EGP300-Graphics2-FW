/*
	Morphing - Pass Keyframes
	By Dan Buckstein
	Vertex shader that passes keyframes for morphing to geometry shader.
	
	Modified by: ______________________________________________________________
*/

#version 410


// ****
// attributes: one per morphing attribute (e.g. multiple positions)
layout (location = 0) in vec4 p0;
layout (location = 1) in vec4 p1;
layout (location = 2) in vec4 p2;
layout (location = 3) in vec4 p3;

// uniforms
uniform mat4 mvp;

// ****
// varying output: pass all attributes
out vertexdata
{
	vec4 pos0, pos1, pos2, pos3;
} pass;

void main()
{
	// ****
	// pass data along
	pass.pos0 = p0;
	pass.pos1 = p1;
	pass.pos2 = p2;
	pass.pos3 = p3;
}