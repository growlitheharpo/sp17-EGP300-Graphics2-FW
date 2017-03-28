/*
	Draw Curve - Morphing
	By Dan Buckstein
	Geometry shader that draws the path that a morphing vertex follows.
	
	Modified by: ______________________________________________________________
*/

#version 410

// ****
#define SAMPLES_MAX 128

// geometry type layout qualifiers
layout (triangles) in;
layout (line_strip, max_vertices = SAMPLES_MAX) out;

// ****
// receive varying data
in vertexdata
{
	vec4 pos0, pos1, pos2, pos3;
} pass[];

// uniforms
uniform mat4 mvp;

// varying output: solid color
out vec4 passColor;

const vec4 BLUE = vec4(0.0, 0.5, 1.0, 1.0);

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
// draw Catmull-Rom spline segment
void drawCatmullRomSplineSegment(in vec4 pPrev, in vec4 p0, in vec4 p1, in vec4 pNext, const int samples, const float dt)
{
	int i = 0;
	float t = 0.0;
	
	for (i; i <= samples; i++)
	{
		gl_Position = mvp * sampleCatmullRom(pPrev, p0, p1, pNext, t);
		EmitVertex();

		t += dt;
	}

	EndPrimitive();
}

void drawCurve(const int samples, int i, const float dt)
{
	passColor = BLUE;

	vec4 p0 = pass[i].pos0, 
		p1 = pass[i].pos1, 
		p2 = pass[i].pos2, 
		p3 = pass[i].pos3;

	drawCatmullRomSplineSegment(p0, p1, p2, p3, samples, dt);
	drawCatmullRomSplineSegment(p1, p2, p3, p0, samples, dt);
	drawCatmullRomSplineSegment(p2, p3, p0, p1, samples, dt);
	drawCatmullRomSplineSegment(p3, p0, p1, p2, samples, dt);
}

void drawLineFull(in vec4 p0, in vec4 p1)
{
	gl_Position = mvp * p0;
	EmitVertex();
	gl_Position = mvp * p1;
	EmitVertex();
	EndPrimitive();
}

void main()
{
	// ****
	// do line drawing
	int samples = 8, i, j;
	float dt = 1.0 / float(samples);

	passColor = vec4(0.0, 0.5, 1.0, 1.0);

	for (i = 0; i < 3; i++)
	{
		drawCurve(samples, i, dt);
	}

	// testing: pass solid color*/

	/*
	// DEBUGGING: draw a normal vector for each vertex
	const vec4 n = vec4(0.0, 0.5, 0.5, 0.0);

	drawLineFull(pass[0].pos0, pass[0].pos1);
	drawLineFull(pass[0].pos1, pass[0].pos2);
	drawLineFull(pass[0].pos2, pass[0].pos3);
	drawLineFull(pass[0].pos3, pass[0].pos0);
	
	drawLineFull(pass[1].pos0, pass[1].pos1);
	drawLineFull(pass[1].pos1, pass[1].pos2);
	drawLineFull(pass[1].pos2, pass[1].pos3);
	drawLineFull(pass[1].pos3, pass[1].pos0);
	
	drawLineFull(pass[2].pos0, pass[2].pos1);
	drawLineFull(pass[2].pos1, pass[2].pos2);
	drawLineFull(pass[2].pos2, pass[2].pos3);
	drawLineFull(pass[2].pos3, pass[2].pos0);*/
}