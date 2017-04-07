/*
	Skinning & Pass Normal as Color
	By Dan Buckstein
	Vertex shader that performs vertex skinning, passing resulting normal out
		as a solid color for debugging purposes.
	
	Modified by: ______________________________________________________________
*/

#version 410


// ****
// the usual attributes, but also need skinning attributes: 
// weights and bone indices
layout (location = 0) in vec4 position;


uniform mat4 mvp;


#define BONES_MAX 64

// ****
// skinning uniforms


// pass some piece of data along for debugging
out vec4 passColor;


// ****
// skinning algorithm: 
// vector goes indeformed vector comes out
vec4 skin(in vec4 v)
{
	vec4 v_out = vec4(0.0);

	return v_out;
}


void main()
{
	// ****
	// skin any attribute that has to do with the SURFACE
	// i.e. anything that could potentially affect shading

	gl_Position = mvp * position;

	// pass debug color
	passColor = vec4(1.0);
}