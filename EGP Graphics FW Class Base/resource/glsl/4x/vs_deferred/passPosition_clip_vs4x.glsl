/*
	Pass Position Clip
	By Dan Buckstein
	Vertex shader that passes final position in clip space.
	
	Modified by: ______________________________________________________________
*/

// version
#version 410


// attributes
layout (location = 0) in vec4 position;


// uniforms
uniform mat4 mvp;


// ****
// varyings


// shader function
void main()
{
	// ****
	// set proper clip position
	gl_Position = mvp * position;
}