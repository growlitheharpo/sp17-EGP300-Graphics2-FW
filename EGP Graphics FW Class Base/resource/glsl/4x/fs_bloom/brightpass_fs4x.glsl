/*
	Bright Pass
	By Dan Buckstein
	Fragment shader that leaves bright pixels bright and makes dark darker.
	
	Modified by: ______________________________________________________________
*/

// version
#version 410


// varyings
in vec2 passTexcoord;


// uniforms
uniform sampler2D img;


// target
layout (location = 0) out vec4 fragColor;


// shader function
void main()
{
	// ****
	// output: make bright values brighter, make dark values darker
	fragColor = texture(img, passTexcoord);
}