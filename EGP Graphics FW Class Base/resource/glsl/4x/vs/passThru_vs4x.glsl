/*
	Pass Thru
	By Dan Buckstein
	Vertex shader that does nothing special.
	
	Modified by: ______________________________________________________________
*/

#version 410


layout (location = 0) in vec4 position;

uniform mat4 mvp;

void main()
{
	gl_Position = position;
}