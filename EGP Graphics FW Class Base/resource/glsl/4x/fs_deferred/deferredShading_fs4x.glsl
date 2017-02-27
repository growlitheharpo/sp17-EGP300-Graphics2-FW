/*
	Deferred Shading
	By Dan Buckstein
	Fragment shader that performs shading using geometry buffers.
	
	Modified by: ______________________________________________________________
*/

// version
#version 410


// ****
// varyings
in vec2 passTexcoord;


// ****
// uniforms


// target
layout (location = 0) out vec4 fragColor;


// shader function
void main()
{
	// ****
	// output: calculate lighting for each light by reading in 
	//	attribute data from g-buffers
}