#include "vector3.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

#define COMPARE_VEC(vec1, vec2) (vec1.x == vec2.x && vec1.y == vec2.y && vec1.z == vec2.z)

#define ASSERT_PASSED_TEST(vec1, vec2, func)										\
		if (!COMPARE_VEC(vec1, vec2))												\
		{																			\
			printf("DID NOT GET EXPECTED RESULT FOR %s. EXPECTED, ACTUAL:\n", func);	\
			printVector(&expectedResult, '\n');										\
			printVector(&actualResult, '\n');										\
		}																			\

const Vector3 UP_VECTOR3 = { 0.0f, 0.0f, 1.0f };
const Vector3 RIGHT_VECTOR3 = { 1.0f, 0.0f, 0.0f };
const Vector3 FORWARD_VECTOR3 = { 0.0f, 1.0f, 0.0f };
const Vector3 ZERO_VECTOR3 = { 0.0f, 0.0f, 0.0f };
const Vector3 ONE_VECTOR3 = { 1.0f, 1.0f, 1.0f };

void setVector(Vector3* vec, float tX, float tY, float tZ)
{
	vec->x = tX;
	vec->y = tY;
	vec->z = tZ;
}

Vector3 add(Vector3 const* one, Vector3 const* two)
{
	if (one == NULL)
		one = &ZERO_VECTOR3;
	if (two == NULL)
		two = &ZERO_VECTOR3;

	Vector3 result = { 
			.x = one->x + two->x,
			.y = one->y + two->y,
			.z = one->z + two->z };
	return result;
}

Vector3 subtract(Vector3 const* one, Vector3 const* two)
{
	if (one == NULL)
		one = &ZERO_VECTOR3;
	if (two == NULL)
		two = &ZERO_VECTOR3;

	Vector3 result = { 
			.x = one->x - two->x,
			.y = one->y - two->y,
			.z = one->z - two->z };
	return result;
}

Vector3 multiply(Vector3 const* vector, float scalar)
{
	if (vector == NULL)
		return ZERO_VECTOR3;

	Vector3 result = {
		.x = vector->x * scalar,
		.y = vector->y * scalar,
		.z = vector->z * scalar };
	return result;
}

Vector3 lerp(Vector3 const* start, Vector3 const* end, float t)
{
	if (start == NULL)
		start = &ZERO_VECTOR3;
	if (end == NULL)
		end = &ZERO_VECTOR3;

	if (t < 0.0f)
		t = 0.0f;
	if (t > 1.0f)
		t = 1.0f;

	Vector3 result = {
		.x = start->x * (1.0f - t) + end->x * t,
		.y = start->y * (1.0f - t) + end->y * t,
		.z = start->z * (1.0f - t) + end->z * t };
	return result;
}

float vectorMagnitude(Vector3 const* vec)
{
	return sqrt(vectorMagnitudeSqrd(vec));
}

float vectorMagnitudeSqrd(Vector3 const* vec)
{
	if (vec == NULL)
		vec = &ZERO_VECTOR3;

	return 
		vec->x * vec->x + 
		vec->y * vec->y + 
		vec->z * vec->z;
}

float dot(Vector3 const* one, Vector3 const* two)
{
	if (one == NULL)
		one = &ZERO_VECTOR3;
	if (two == NULL)
		two = &ZERO_VECTOR3;

	return 
		one->x * two->x + 
		one->y * two->y + 
		one->z * two->z;
}

Vector3 cross(Vector3 const* one, Vector3 const* two)
{
	if (one == NULL)
		one = &ZERO_VECTOR3;
	if (two == NULL)
		two = &ZERO_VECTOR3;

	Vector3 result = {
		.x = one->y * two->z - one->z * two->y,
		.y = one->x * two->z - one->z * two->x,
		.z = one->x * two->y - one->y * two->x };
	return result;
}

void printVector(Vector3 const* vec, char end)
{
	printf("X: %.3f, Y: %.3f, Z: %.3f%c", vec->x, vec->y, vec->z, end);
}

void runVectorTestSuite()
{
	Vector3 expectedResult;
	Vector3 actualResult;
	
	expectedResult.x = 2.0f;
	expectedResult.y = 3.0f;
	expectedResult.z = 4.0f;
	setVector(&actualResult, 2.0f, 3.0f, 4.0f);
	ASSERT_PASSED_TEST(expectedResult, actualResult, "set");

	{	//TEST FOR ADD
		setVector(&expectedResult, 0.0f, 0.0f, 2.0f);
		actualResult = add(&UP_VECTOR3, &UP_VECTOR3);
		ASSERT_PASSED_TEST(expectedResult, actualResult, "add up+up");

		setVector(&expectedResult, 1.0f, 0.0f, 1.0f);
		actualResult = add(&UP_VECTOR3, &RIGHT_VECTOR3);
		ASSERT_PASSED_TEST(expectedResult, actualResult, "add up+left");

		setVector(&expectedResult, 1.0f, 1.0f, 0.0f);
		actualResult = add(&FORWARD_VECTOR3, &RIGHT_VECTOR3);
		ASSERT_PASSED_TEST(expectedResult, actualResult, "add forward+left");

		setVector(&expectedResult, 2.0f, 2.0f, 0.0f);
		actualResult = add(&actualResult, &actualResult);
		ASSERT_PASSED_TEST(expectedResult, actualResult, "add (forward+left) + (forward+left)");

		setVector(&expectedResult, 0.0f, 0.0f, 1.0f);
		actualResult = add(NULL, &UP_VECTOR3);
		ASSERT_PASSED_TEST(expectedResult, actualResult, "add NULL+up");

		setVector(&expectedResult, 0.0f, 0.0f, 1.0f);
		actualResult = add(&UP_VECTOR3, NULL);
		ASSERT_PASSED_TEST(expectedResult, actualResult, "add up+NULL");

		setVector(&expectedResult, 0.0f, 0.0f, 0.0f);
		actualResult = add(NULL, NULL);
		ASSERT_PASSED_TEST(expectedResult, actualResult, "add NULL+NULL");
	}

	{	//TEST FOR SUB
		setVector(&expectedResult, 0.0f, 0.0f, 0.0f);
		actualResult = subtract(&UP_VECTOR3, &UP_VECTOR3);
		ASSERT_PASSED_TEST(expectedResult, actualResult, "sub up-up");

		setVector(&expectedResult, -1.0f, 0.0f, 1.0f);
		actualResult = subtract(&UP_VECTOR3, &RIGHT_VECTOR3);
		ASSERT_PASSED_TEST(expectedResult, actualResult, "sub up-left");

		setVector(&expectedResult, -1.0f, 1.0f, 0.0f);
		actualResult = subtract(&FORWARD_VECTOR3, &RIGHT_VECTOR3);
		ASSERT_PASSED_TEST(expectedResult, actualResult, "sub forward-left");

		setVector(&expectedResult, 0.0f, 0.0f, 0.0f);
		actualResult = subtract(&actualResult, &actualResult);
		ASSERT_PASSED_TEST(expectedResult, actualResult, "sub (forward-left) - (forward-left)");

		setVector(&expectedResult, 0.0f, 0.0f, -1.0f);
		actualResult = subtract(NULL, &UP_VECTOR3);
		ASSERT_PASSED_TEST(expectedResult, actualResult, "sub NULL-up");

		setVector(&expectedResult, 0.0f, 0.0f, 1.0f);
		actualResult = subtract(&UP_VECTOR3, NULL);
		ASSERT_PASSED_TEST(expectedResult, actualResult, "sub up-NULL");

		setVector(&expectedResult, 0.0f, 0.0f, 0.0f);
		actualResult = subtract(NULL, NULL);
		ASSERT_PASSED_TEST(expectedResult, actualResult, "sub NULL-NULL");
	}

	{	//TEST FOR MUL
		setVector(&expectedResult, 0.0f, 0.0f, 5.0f);
		actualResult = multiply(&UP_VECTOR3, 5.0f);
		ASSERT_PASSED_TEST(expectedResult, actualResult, "MUL up*5");

		setVector(&expectedResult, -3.5f, -3.5f, -3.5f);
		actualResult = multiply(&ONE_VECTOR3, -3.5f);
		ASSERT_PASSED_TEST(expectedResult, actualResult, "MUL one*-3.5");

		setVector(&expectedResult, 0.0f, 0.0f, 0.0f);
		actualResult = multiply(NULL, 2.0f);
		ASSERT_PASSED_TEST(expectedResult, actualResult, "MUL NULL*2");
	}

	{	//TEST FOR LERP
		setVector(&expectedResult, 0.5f, 0.5f, 0.5f);
		actualResult = lerp(&ZERO_VECTOR3, &ONE_VECTOR3, 0.5f);
		ASSERT_PASSED_TEST(expectedResult, actualResult, "lerp zero to one, 0.5f");

		setVector(&expectedResult, 0.0f, 0.0f, 0.0f);
		actualResult = lerp(&ZERO_VECTOR3, &ONE_VECTOR3, -1.0f);
		ASSERT_PASSED_TEST(expectedResult, actualResult, "lerp zero to one, -1f");

		setVector(&expectedResult, 1.0f, 1.f, 1.f);
		actualResult = lerp(&ZERO_VECTOR3, &ONE_VECTOR3, 5.0f);
		ASSERT_PASSED_TEST(expectedResult, actualResult, "lerp zero to one, 5f");

		setVector(&expectedResult, 0.5f, 0.5f, 0.5f);
		actualResult = lerp(&ONE_VECTOR3, NULL, 0.5f);
		ASSERT_PASSED_TEST(expectedResult, actualResult, "lerp one to NULL, 0.5f");

		setVector(&expectedResult, 0.5f, 0.5f, 0.5f);
		actualResult = lerp(NULL, &ONE_VECTOR3, 0.5f);
		ASSERT_PASSED_TEST(expectedResult, actualResult, "lerp NULL to one, 0.5f");
	}

	{	//TEST FOR MAG
		float expectedFloat = 1.0f;
		float actualFloat = vectorMagnitude(&UP_VECTOR3);
		assert(expectedFloat == actualFloat);

		Vector3 testVec = { .x = 4.f,.y = 1.f,.z = 8.f };
		expectedFloat = 9.0f;
		actualFloat = vectorMagnitude(&testVec);
		assert(expectedFloat == actualFloat);

		expectedFloat = 0.0f;
		actualFloat = vectorMagnitude(NULL);
		assert(expectedFloat == actualFloat);
	}

	{	//TEST FOR MAGSQR
		float expectedFloat = 1.0f;
		float actualFloat = vectorMagnitudeSqrd(&UP_VECTOR3);
		assert(expectedFloat == actualFloat);

		Vector3 testVec = { .x = 4.f,.y = 1.f,.z = 8.f };
		expectedFloat = 81.0f;
		actualFloat = vectorMagnitudeSqrd(&testVec);
		assert(expectedFloat == actualFloat);

		expectedFloat = 0.0f;
		actualFloat = vectorMagnitudeSqrd(NULL);
		assert(expectedFloat == actualFloat);
	}

	{	//TEST FOR DOT
		float expectedFloat = 1.0f;
		float actualFloat = dot(&UP_VECTOR3, &UP_VECTOR3);
		assert(expectedFloat == actualFloat);

		expectedFloat = 0.0f;
		actualFloat = dot(&UP_VECTOR3, &RIGHT_VECTOR3);
		assert(expectedFloat == actualFloat);

		expectedFloat = -1.0f;
		Vector3 downVector = multiply(&UP_VECTOR3, -1.0f);
		actualFloat = dot(&UP_VECTOR3, &downVector);
		assert(expectedFloat == actualFloat);

		Vector3 testVec1 = { .x = 4,.y = 4,.z = 4 };
		Vector3 testVec2 = { .x = 2,.y = 2,.z = 2 };
		expectedFloat = 24.0f;
		actualFloat = dot(&testVec1, &testVec2);
		assert(expectedFloat == actualFloat);

		expectedFloat = 0.0f;
		actualFloat = dot(NULL, &UP_VECTOR3);
		assert(expectedFloat == actualFloat);

		expectedFloat = 0.0f;
		actualFloat = dot(&UP_VECTOR3, NULL);
		assert(expectedFloat == actualFloat);
	}

	{	//TEST FOR CROSS
		expectedResult = UP_VECTOR3;
		actualResult = cross(&RIGHT_VECTOR3, &FORWARD_VECTOR3);
		ASSERT_PASSED_TEST(expectedResult, actualResult, "cross leftxforward");

		expectedResult = multiply(&UP_VECTOR3, -1.0f);
		actualResult = cross(&FORWARD_VECTOR3, &RIGHT_VECTOR3);
		ASSERT_PASSED_TEST(expectedResult, actualResult, "cross forwardxleft");

		expectedResult = ZERO_VECTOR3;
		actualResult = cross(NULL, &FORWARD_VECTOR3);
		ASSERT_PASSED_TEST(expectedResult, actualResult, "cross nullxforward");

		expectedResult = ZERO_VECTOR3;
		actualResult = cross(&FORWARD_VECTOR3, NULL);
		ASSERT_PASSED_TEST(expectedResult, actualResult, "cross forwardxnull");
	}
}
