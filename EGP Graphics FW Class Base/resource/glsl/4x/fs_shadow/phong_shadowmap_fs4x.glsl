/*
	Phong with Shadow Mapping
	By Dan Buckstein
	Fragment shader that performs Phong shading and shadow mapping.
	
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
	fragColor = vec4(0.0, 0.5, 1.0, 1.0);
}