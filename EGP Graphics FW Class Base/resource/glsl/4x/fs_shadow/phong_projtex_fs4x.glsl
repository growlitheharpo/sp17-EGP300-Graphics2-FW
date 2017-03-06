/*
	Phong with Projective Texturing
	By Dan Buckstein
	Fragment shader that performs Phong shading and projective texturing.
	
	Modified by: ______________________________________________________________
*/

#version 410


// ****
// varyings: 


// ****
// uniforms: 


// target: 
layout (location = 0) out vec4 fragColor;


// ****
// constants/globals: 


void main()
{
	fragColor = vec4(1.0, 0.5, 0.0, 1.0);
}