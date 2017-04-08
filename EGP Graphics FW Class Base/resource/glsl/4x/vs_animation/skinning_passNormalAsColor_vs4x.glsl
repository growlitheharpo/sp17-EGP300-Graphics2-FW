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
layout (location = 2) in vec4 normal;

layout (location = 1) in vec4 blendWeight;
layout (location = 7) in vec4 blendIndex;


uniform mat4 mvp;


#define BONES_MAX 64

// ****
// skinning uniforms
uniform mat4 bones_skinning[BONES_MAX];


// pass some piece of data along for debugging
out vec4 passColor;


// ****
// skinning algorithm: 
// vector goes in... deformed vector comes out
vec4 skin(in vec4 v)
{
	vec4 v_out = vec4(0.0);

	v_out += bones_skinning[int(blendIndex.x)] * (blendWeight.x * v);
	v_out += bones_skinning[int(blendIndex.y)] * (blendWeight.y * v);
	v_out += bones_skinning[int(blendIndex.z)] * (blendWeight.z * v);
	v_out += bones_skinning[int(blendIndex.w)] * (blendWeight.w * v);

	return v_out;
}


void main()
{
	// ****
	// skin any attribute that has to do with the SURFACE
	// i.e. anything that could potentially affect shading

	gl_Position = mvp * skin(position);

	// pass debug color
	vec4 skinnedNormal = skin(vec4(normal.xyz, 0.0f));

	skinnedNormal += vec4(1.0f);
	skinnedNormal /= 2.0f;

	passColor = skinnedNormal;
}