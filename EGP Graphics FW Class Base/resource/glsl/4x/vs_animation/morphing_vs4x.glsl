/*
	Morphing
	By Dan Buckstein
	Vertex shader that performs morph target animation.
	
	Modified by: ______________________________________________________________
*/

#version 430
#define KEYFRAME_COUNT 4

// ****
// attributes: one per morphing attribute (e.g. multiple positions)
layout (location = 0) in vec4 position0;
layout (location = 1) in vec4 position1;
layout (location = 2) in vec4 position2;
layout (location = 3) in vec4 position3;

// ****
// uniforms: animation controls
uniform int index[KEYFRAME_COUNT];
uniform float param;
uniform mat4 mvp;

// varying output: solid color
out vec4 passColor;


// ****
// LERP
vec4 lerp(in vec4 p0, in vec4 p1, const float t)
{
	return p0 + (p1 - p0) * t;
}

// ****
// Catmull-Rom spline interpolation
vec4 sampleCatmullRom(in vec4 pPrev, in vec4 p0, in vec4 p1, in vec4 pNext, const float t)
{
	float t2 = t * t;
	float t3 = t2 * t;

	mat4 catmullMat = transpose(mat4(
			0.0f, -1.0f, 2.0f, -1.0f,
			2.0f, 0.0f, -5.0f, 3.0f,
			0.0f, 1.0f, 4.0f, -3.0f,
			0.0f, 0.0f, -1.0f, 1.0f));

	vec4 tVals = vec4(1.0f, t, t2, t3);

	vec4 cat = catmullMat * tVals;
	mat4 points = mat4(pPrev, p0, p1, pNext);

	vec4 result = points * cat;

	return 0.5f * result;
}


// ****
// cubic Hermite spline interpolation
vec4 sampleCubicHermite(in vec4 p0, in vec4 m0, in vec4 p1, in vec4 m1, const float t)
{
	float t2 = t * t;
	float t3 = t2 * t;

	mat4 hermiteMat = transpose(mat4(
			1.0f, 0.0f, -3.0f, 2.0f,
			0.0f, 1.0f, -2.0f, 1.0f,
			0.0f, 0.0f, 3.0f, -2.0f,
			0.0f, 0.0f, -1.0f, 1.0f));

	vec4 tVals = vec4(1.0f, t, t2, t3);

	vec4 cat = hermiteMat * tVals;
	mat4 points = mat4(p0, m0, p1, m1);

	vec4 result = points * cat;

	return result;
}


void main()
{

	vec4 posList[KEYFRAME_COUNT] = { position0, position1, position2, position3 };

	vec4 pos = sampleCatmullRom(posList[index[0]], posList[index[1]], posList[index[2]], posList[index[3]], param);

	// ****
	// do morphing, transform the correct result before assigning to output
	gl_Position = mvp * pos;

	// TESTING: send position as color to give us some variance
	passColor = position0*0.5+0.5;
}