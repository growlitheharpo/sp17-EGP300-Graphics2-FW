/*
	Blend
	By Dan Buckstein
	Fragment shader that blends 4 textures using the "screen" filter.
	
	Modified by: ______________________________________________________________
*/

// version
#version 410


// varyings
in vec2 passTexcoord;


// ****
// uniforms
uniform sampler2D img;


// target
layout (location = 0) out vec4 fragColor;


// shader function
void main()
{
	// ****
	// output: screen four images together
	fragColor = texture(img, passTexcoord);
}