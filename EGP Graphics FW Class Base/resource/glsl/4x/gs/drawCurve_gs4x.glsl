/*
	Draw Curve
	By Dan Buckstein
	Geometry shader that draws waypoints to form a curve.
	
	Modified by: ______________________________________________________________
*/

#version 410

#define WAYPOINTS_MAX 16
#define SAMPLES_PER_SEGMENT 16
#define SAMPLES_MAX 256

#define CURVE_LINES 0
#define CURVE_BEZIER 1
#define CURVE_CATMULLROM 2
#define CURVE_HERMITE 3


// ****
// layout qualifiers for geometry shader: 
//	-> input primitive
//	-> output primitive (max count optional)


// uniforms
uniform mat4 mvp;

uniform vec4 waypoint[WAYPOINTS_MAX];
uniform int waypointCount = 0;
uniform int curveMode = 0;
uniform int useWaypoints = 0;

// varyings
out vec4 passColor;

// constants
const vec4 BLUE = vec4(0.0, 0.5, 1.0, 1.0);
const vec4 GREEN = vec4(0.0, 1.0, 0.5, 1.0);


// ****
// LERP
vec4 lerp(in vec4 p0, in vec4 p1, const float t)
{
	return p0;
}

// ****
// Bezier curve interpolation
// easier with recursion... but that's not an option here...
//	...so we'll have to implement specific cases
vec4 sampleBezier0(in vec4 p0, const float t)
{
	return p0;
}
vec4 sampleBezier1(in vec4 p0, in vec4 p1, const float t)
{
	return p0;
}
vec4 sampleBezier2(in vec4 p0, in vec4 p1, in vec4 p2, const float t)
{
	return p0;
}
vec4 sampleBezier3(in vec4 p0, in vec4 p1, in vec4 p2, in vec4 p3, const float t)
{
	return p0;
}

// ****
// Catmull-Rom spline interpolation
vec4 sampleCatmullRom(in vec4 pPrev, in vec4 p0, in vec4 p1, in vec4 pNext, const float t)
{
	return p0;
}

// ****
// cubic Hermite spline interpolation
vec4 sampleCubicHermite(in vec4 p0, in vec4 m0, in vec4 p1, in vec4 m1, const float t)
{
	return p0;
}


// ****
// draw line
void drawLine(in vec4 p0, in vec4 p1, const int samples, const float dt)
{
	// no need for segments, it's just a straight line from p0 to p1...
	// but to prove that lerp works...
	int i = 0;
	float t = 0.0;
}

// ****
void drawLineFull(in vec4 p0, in vec4 p1)
{
}

// ****
// draw entire Bezier curve
void drawBezierCurve0(in vec4 p0, const int samples, const float dt)
{
	int i = 0;
	float t = 0.0;
}
void drawBezierCurve1(in vec4 p0, in vec4 p1, const int samples, const float dt)
{
	int i = 0;
	float t = 0.0;
}
void drawBezierCurve2(in vec4 p0, in vec4 p1, in vec4 p2, const int samples, const float dt)
{
	int i = 0;
	float t = 0.0;
}
void drawBezierCurve3(in vec4 p0, in vec4 p1, in vec4 p2, in vec4 p3, const int samples, const float dt)
{
	int i = 0;
	float t = 0.0;
}

// ****
// draw Catmull-Rom spline segment
void drawCatmullRomSplineSegment(in vec4 pPrev, in vec4 p0, in vec4 p1, in vec4 pNext, const int samples, const float dt)
{
	int i = 0;
	float t = 0.0;
}

// ****
// draw cubic Hermite spline segment
void drawCubicHermiteSplineSegment(in vec4 p0, in vec4 m0, in vec4 p1, in vec4 m1, const int samples, const float dt)
{
	int i = 0;
	float t = 0.0;
}


void drawCurve(const int samples, const float dt)
{
	int i0, i1, n;
	vec4 p0, p1, p2, p3, pPrev, pNext, m0, m1;

	passColor = BLUE;

	switch(curveMode)
	{
	case CURVE_BEZIER: 
		// curve up to degree 3
		for (i0 = 0, i1 = i0 + 3, n = waypointCount; i1 < n; i0 = i1, i1 += 3)
		{
			p0 = waypoint[i0];
			p1 = waypoint[i0+1];
			p2 = waypoint[i0+2];
			p3 = waypoint[i1];
			drawBezierCurve3(p0, p1, p2, p3, samples, dt);
		}
		passColor = GREEN;
	//	break;	// no break to draw the lines that are used to form the curve
	case CURVE_LINES:
		// draw line segments between each waypoint
		for (i0 = 0, i1 = i0 + 1, n = waypointCount; i1 < n; i0 = i1, ++i1)
		{
			p0 = waypoint[i0];
			p1 = waypoint[i1];
			drawLineFull(p0, p1);
		}
		break;
	case CURVE_CATMULLROM: 
		// draw curve between waypoint 1 and n-1
		for (i0 = 1, i1 = i0 + 1, n = waypointCount - 1; i1 < n; i0 = i1, ++i1)
		{
			pPrev = waypoint[i0-1];
			p0 = waypoint[i0];
			p1 = waypoint[i1];
			pNext = waypoint[i1+1];
			drawCatmullRomSplineSegment(pPrev, p0, p1, pNext, samples, dt);
		}
		if (waypointCount > 2)
		{
			// draw lines on first and last segment separately to avoid loop
			passColor = GREEN;
			pPrev = waypoint[0];
			p0 = waypoint[1];
			p1 = waypoint[n-1];
			pNext = waypoint[n];
			drawLineFull(pPrev, p0);
			drawLineFull(p1, pNext);
		}
		break;
	case CURVE_HERMITE: 
		// let every pair of points be a waypoint and its tangent handle
		for (i0 = 0, i1 = i0 + 2, n = waypointCount - 1; i1 < n; i0 = i1, i1 += 2)
		{
			passColor = BLUE;
			p0 = waypoint[i0];
			m0 = waypoint[i0+1] - p0;	// bi-directional tangents
		//	m0 = p0 - waypoint[i0+1];	// single-direction tangents (split curve)
			p1 = waypoint[i1];
			m1 = waypoint[i1+1] - p1;
			drawCubicHermiteSplineSegment(p0, m0, p1, m1, samples, dt);

			passColor = GREEN;
			drawLineFull(p1, p1 + m1);
			drawLineFull(p1, p1 - m1);
		}
		if (waypointCount > 1)
		{
			// draw bi-directional tangent for first point
			passColor = GREEN;
			p0 = waypoint[0];
			m0 = waypoint[1] - p0;
			drawLineFull(p0, p0 + m0);
			drawLineFull(p0, p0 - m0);
		}
		break;
	};
}


// test all using fixed points
void testCurve(const int samples, const float dt)
{
	// test points
	vec4 testP0 = vec4(200.0, 200.0, 0.0, 1.0);
	vec4 testP1 = vec4(400.0, 400.0, 0.0, 1.0);
	vec4 testP2 = vec4(800.0, 400.0, 0.0, 1.0);
	vec4 testP3 = vec4(1000.0, 200.0, 0.0, 1.0);
	
	vec4 p0, p1, p2, p3, pPrev, pNext, m0, m1;

	passColor = BLUE;

	switch(curveMode)
	{
	case CURVE_BEZIER: 
		// full curve
		drawBezierCurve3(testP0, testP1, testP2, testP3, samples, dt);
		passColor = GREEN;
	//	break;	// no break to draw the lines that are used to form the curve
	case CURVE_LINES: 
		// multiple segments
		drawLine(testP0, testP1, samples, dt);
		drawLine(testP1, testP2, samples, dt);
		drawLine(testP2, testP3, samples, dt);
		break;
	case CURVE_CATMULLROM: 
		// curve segment
		pPrev = testP0;
		p0 = testP1;
		p1 = testP2;
		pNext = testP3;
		drawCatmullRomSplineSegment(pPrev, p0, p1, pNext, samples, dt);
		break;
	case CURVE_HERMITE: 
		// curve segment
		p0 = testP0;
		m0 = testP1 - p0;
		p1 = testP2;
		m1 = testP3 - p1;
		drawCubicHermiteSplineSegment(p0, m0, p1, m1, samples, dt);
		// tangents
		passColor = GREEN;
		drawLineFull(p0, p0 + m0);
		drawLineFull(p1, p1 + m1);
		break;
	};
}


void main()
{
	const int samples = SAMPLES_PER_SEGMENT;
	const float dt = 1.0 / float(SAMPLES_PER_SEGMENT);

	if (useWaypoints != 0)
		drawCurve(samples, dt);
	else
		testCurve(samples, dt);
}