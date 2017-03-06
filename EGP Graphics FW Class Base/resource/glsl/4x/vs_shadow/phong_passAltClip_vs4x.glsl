/*
	Phong - Pass Alt Clip Position
	By Dan Buckstein
	Vertex shader that performs vertex requirements for Phong shading, but also
		passes clip position of light for shadow mapping/projective texturing.
	
	Modified by: ______________________________________________________________
*/

#version 410


// ****
// attributes: 
layout (location = 0) in vec4 position;


// ****
// uniforms: 
uniform mat4 mvp;


// ****
// varyings: 


void main()
{
	// set clip position 'gl_Position'
	gl_Position = mvp * position;
}