#ifndef __VECTOR_3_H
#define __VECTOR_3_H


#ifdef __cplusplus
extern "C"
{
#endif	// __cplusplus

typedef struct Vector3_struct
{
	float x, y, z;
} Vector3;

void setVector(Vector3* vec, float tX, float tY, float tZ);
Vector3 add(Vector3 const* one, Vector3 const* two);
Vector3 subtract(Vector3 const* one, Vector3 const* two);
Vector3 multiply(Vector3 const* vector, float scalar);
Vector3 lerp(Vector3 const* start, Vector3 const* end, float t);
float vectorMagnitude(Vector3 const* vec);
float vectorMagnitudeSqrd(Vector3 const* vec);
float dot(Vector3 const* one, Vector3 const* two);
Vector3 cross(Vector3 const* one, Vector3 const* two);
void printVector(Vector3 const* vec, char end);

void runVectorTestSuite();

const extern Vector3 UP_VECTOR3;
const extern Vector3 RIGHT_VECTOR3;
const extern Vector3 FORWARD_VECTOR3;
const extern Vector3 ZERO_VECTOR3;
const extern Vector3 ONE_VECTOR3;

#ifdef __cplusplus
}
#endif	// __cplusplus

#endif
