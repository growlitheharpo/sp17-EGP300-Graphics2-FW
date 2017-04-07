/*
	Draw Bone
	By Dan Buckstein
	Vertex shader that renders an instanced primitive and adds scale.
	
	Modified by: ______________________________________________________________
*/

#version 410


layout (location = 0) in vec4 position;

uniform mat4 mvp;

#define BONES_MAX 64

// ****
// other uniform data


void main()
{
	// ****
	// calculate final bone transform
	
	// ****
	// transform by bone transform before MVP
	gl_Position = mvp * position;
}