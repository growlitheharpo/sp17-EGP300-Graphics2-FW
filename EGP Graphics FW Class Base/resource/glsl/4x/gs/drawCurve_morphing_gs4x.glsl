/*
	Draw Curve - Morphing
	By Dan Buckstein
	Geometry shader that draws the path that a morphing vertex follows.
	
	Modified by: ______________________________________________________________
*/

#version 410

// ****
#define SAMPLES_MAX 6

// geometry type layout qualifiers
layout (triangles) in;
layout (line_strip, max_vertices = SAMPLES_MAX) out;

// ****
// receive varying data
in vertexdata
{
	vec4 position;
} pass[];

// uniforms
uniform mat4 mvp;

// varying output: solid color
out vec4 passColor;


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


	// testing: pass solid color
	passColor = vec4(0.0, 0.5, 1.0, 1.0);


// DEBUGGING: draw a normal vector for each vertex
	const vec4 n = vec4(0.0, 0.0, 1.0, 0.0);
	drawLineFull(pass[0].position, pass[0].position + n);
	drawLineFull(pass[1].position, pass[1].position + n);
	drawLineFull(pass[2].position, pass[2].position + n);
}