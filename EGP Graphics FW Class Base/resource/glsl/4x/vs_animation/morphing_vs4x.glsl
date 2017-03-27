/*
	Morphing
	By Dan Buckstein
	Vertex shader that performs morph target animation.
	
	Modified by: ______________________________________________________________
*/

#version 410


// ****
// attributes: one per morphing attribute (e.g. multiple positions)
layout (location = 0) in vec4 position;

// ****
// uniforms: animation controls
uniform mat4 mvp;

// varying output: solid color
out vec4 passColor;

void main()
{
	// ****
	// do morphing, transform the correct result before assigning to output
	gl_Position = mvp * position;

	// TESTING: send position as color to give us some variance
	passColor = position*0.5+0.5;
}