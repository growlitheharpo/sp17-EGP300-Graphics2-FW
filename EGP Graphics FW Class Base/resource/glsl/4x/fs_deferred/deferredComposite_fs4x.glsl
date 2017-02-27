/*
	Deferred Lighting Composite
	By Dan Buckstein
	Fragment shader that composites precomputed lighting with surface color.
	
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
	// get object texture coordinates to sample surface textures, 
	//	and also sample results from light pre-pass
}