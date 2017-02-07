#include "transformMatrix.h"
#include "vector3.h"
#include <math.h>
#include <stdio.h>
#include "utils.h"

void multiplyMatrixByScalar(TransformMatrix* matrix, float scalar)
{
	for (char i = 0; i < 4; i++)
	{
		for (char j = 0; j < 4; j++)
		{
			matrix->vals[i][j] *= scalar;
		}
	}
}

void printMatrix(TransformMatrix const* matrix)
{
	for (char i = 0; i < 4; i++)
	{
		printf("[%.1f, %.1f, %.1f, %.1f]\n", matrix->vals[i][0], matrix->vals[i][1], matrix->vals[i][2], matrix->vals[i][3]);
	}
	printf("\n");
}

void copyMatrix(TransformMatrix const* src, TransformMatrix* dest)
{
	for (char i = 0; i < 4; i++)
	{
		for (char j = 0; j < 4; j++)
		{
			dest->vals[i][j] = src->vals[i][j];
		}
	}
}

void transposeMatrix(TransformMatrix* matrix)
{
	TransformMatrix tmp;
	copyMatrix(matrix, &tmp);

	for (char i = 0; i < 4; i++)
	{
		for (char j = 0; j < 4; j++)
		{
			tmp.vals[i][j] = matrix->vals[j][i];
		}
	}

	copyMatrix(&tmp, matrix);
}

void makeIdentiyMatrix(TransformMatrix* matrix)
{
	for (char i = 0; i < 4; i++)
	{
		for (char j = 0; j < 4; j++)
		{
			matrix->vals[i][j] = 0.0f;
		}
	}

	matrix->vals[0][0] = 1.0f;
	matrix->vals[1][1] = 1.0f;
	matrix->vals[2][2] = 1.0f;
	matrix->vals[3][3] = 1.0f;
}

void makeXDirectionTransformMatrix(TransformMatrix* matrix, float amount)
{
	makeIdentiyMatrix(matrix);
	matrix->vals[1][1] = cos(amount);
	matrix->vals[1][2] = sin(amount) * -1.0f;
	matrix->vals[2][1] = matrix->vals[1][2] * -1.0f;
	matrix->vals[2][2] = matrix->vals[1][1];
}

void makeYDirectionTransformMatrix(TransformMatrix* matrix, float amount)
{
	makeIdentiyMatrix(matrix);
	matrix->vals[0][0] = cos(amount);
	matrix->vals[0][2] = sin(amount);
	matrix->vals[2][0] = matrix->vals[0][2] * -1.0f;
	matrix->vals[2][2] = matrix->vals[0][0];
}

void makeZDirectionTransformMatrix(TransformMatrix* matrix, float amount)
{
	makeIdentiyMatrix(matrix);
	matrix->vals[0][0] = cos(amount);
	matrix->vals[0][1] = sin(amount) * -1.0f;
	matrix->vals[1][0] = matrix->vals[0][1] * -1.0f;
	matrix->vals[1][1] = matrix->vals[1][1];
}

void makeTransformFromRotation(TransformMatrix* matrix, float pitch, float yaw, float roll)
{
	TransformMatrix x, y, z, result;
	makeXDirectionTransformMatrix(&x, pitch);
	makeYDirectionTransformMatrix(&y, yaw);
	makeZDirectionTransformMatrix(&z, roll);

	multiplyMatrix(&result, &y, &x);
	multiplyMatrix(matrix, &z, &result);
}

void makeTransformFromScale(TransformMatrix* matrix, float x, float y, float z)
{
	makeIdentiyMatrix(matrix);
	matrix->vals[0][0] = x;
	matrix->vals[1][1] = y;
	matrix->vals[2][2] = z;
}

void makeTransformFromUniformScale(TransformMatrix* matrix, float uniformScale)
{
	makeTransformFromScale(matrix, uniformScale, uniformScale, uniformScale);
}

void makeTransformFromTranslation(TransformMatrix* matrix, float dX, float dY, float dZ)
{
	makeIdentiyMatrix(matrix);
	matrix->vals[0][3] = dX;
	matrix->vals[1][3] = dY;
	matrix->vals[2][3] = dZ;
}

void inverseTransformUnscaled(TransformMatrix* matrix)
{
	TransformMatrix rotTranspose;
	copyMatrix(matrix, &rotTranspose);
	transposeMatrix(&rotTranspose);

	TransformMatrix negativeRotTranspose;
	copyMatrix(&rotTranspose, &negativeRotTranspose);
	multiplyMatrixByScalar(&negativeRotTranspose, -1.0f);

	TransformMatrix result;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			result.vals[i][j] = rotTranspose.vals[i][j];
		}
	}

	Vector3 tmp = {
	.x = matrix->vals[0][3],
	.y = matrix->vals[1][3],
	.z = matrix->vals[2][3] };
	applyTransformToPoint(&tmp, &negativeRotTranspose);

	result.vals[0][3] = tmp.x;
	result.vals[1][3] = tmp.y;
	result.vals[2][3] = tmp.z;

	result.vals[3][0] = 0.0f;
	result.vals[3][1] = 0.0f;
	result.vals[3][2] = 0.0f;
	result.vals[3][3] = 1.0f;

	copyMatrix(&result, matrix);
}

int areTransformsEqual(TransformMatrix* a, TransformMatrix* b)
{
	for (char i = 0; i < 4; i++)
	{
		for (char j = 0; j < 4; j++)
		{
			if (a->vals[i][j] != b->vals[i][j])
				return 0;
		}
	}

	return 1;
}

//Thanks to http://ogldev.atspace.co.uk/www/tutorial11/tutorial11.html for some help understanding
//the next two functions.
void multiplyMatrix(TransformMatrix* matrixOut, TransformMatrix* matrixLeft, TransformMatrix* matrixRight)
{
	for (char i = 0; i < 4; i++)
	{
		for (char j = 0; j < 4; j++)
		{
			matrixOut->vals[i][j] = 
				matrixLeft->vals[i][0] * matrixRight->vals[0][j] +
				matrixLeft->vals[i][1] * matrixRight->vals[1][j] +
				matrixLeft->vals[i][2] * matrixRight->vals[2][j] +
				matrixLeft->vals[i][3] * matrixRight->vals[3][j];
		}
	}
}

void concatenateTransform(TransformMatrix* matrixOut, TransformMatrix* scaleMatrix, TransformMatrix* rotationMatrix, TransformMatrix* translationMatrix)
{
	TransformMatrix result1;
	multiplyMatrix(&result1, rotationMatrix, scaleMatrix);
	multiplyMatrix(matrixOut, translationMatrix, &result1);
}

void applyTransformToPoint(Vector3* pointInOut, TransformMatrix const* transform)
{
	Vector3 tmp = ZERO_VECTOR3;
	tmp.x = transform->vals[0][0] * pointInOut->x + transform->vals[0][1] * pointInOut->y + transform->vals[0][2] * pointInOut->z + transform->vals[0][3];
	tmp.y = transform->vals[1][0] * pointInOut->x + transform->vals[1][1] * pointInOut->y + transform->vals[1][2] * pointInOut->z + transform->vals[1][3];
	tmp.z = transform->vals[2][0] * pointInOut->x + transform->vals[2][1] * pointInOut->y + transform->vals[2][2] * pointInOut->z + transform->vals[2][3];
	
	setVector(pointInOut, tmp.x, tmp.y, tmp.z);
}

void runTransformTestSuite()
{
	Vector3 simplePoint;
	TransformMatrix test;
	makeIdentiyMatrix(&test);
	printMatrix(&test);

	multiplyMatrixByScalar(&test, 4.0f);
	printMatrix(&test);

	makeTransformFromScale(&test, 4.0f, 6.0f, 2.0f);
	printMatrix(&test);

	makeXDirectionTransformMatrix(&test, M_PI / 2.0f);
	printMatrix(&test);

	simplePoint = UP_VECTOR3;
	applyTransformToPoint(&simplePoint, &test);
	printVector(&simplePoint, '\n');
	printf("\n");

	makeYDirectionTransformMatrix(&test, M_PI / 2.0f);
	printMatrix(&test);

	simplePoint = UP_VECTOR3;
	applyTransformToPoint(&simplePoint, &test);
	printVector(&simplePoint, '\n');
	printf("\n");

	makeZDirectionTransformMatrix(&test, M_PI / 2.0f);
	printMatrix(&test);

	simplePoint = UP_VECTOR3;
	applyTransformToPoint(&simplePoint, &test);
	printVector(&simplePoint, '\n');
	printf("\n");

	makeTransformFromRotation(&test, M_PI / 2.0f, 0.0f, 0.0f);
	simplePoint = UP_VECTOR3;
	applyTransformToPoint(&simplePoint, &test);
	printVector(&simplePoint, '\n');
	printf("\n");

	makeTransformFromScale(&test, 4.0f, 2.0f, 3.0f);
	simplePoint = UP_VECTOR3;
	applyTransformToPoint(&simplePoint, &test);
	printVector(&simplePoint, '\n');
	printf("\n");

	makeTransformFromTranslation(&test, 2.0f, 3.0f, 2.0f);
	simplePoint = UP_VECTOR3;
	applyTransformToPoint(&simplePoint, &test);
	printVector(&simplePoint, '\n');
	printf("\n");

	inverseTransformUnscaled(&test);
	applyTransformToPoint(&simplePoint, &test);
	printVector(&simplePoint, '\n');
	printf("\n");

	TransformMatrix rotation, scale, translation;
	simplePoint = UP_VECTOR3;

	makeXDirectionTransformMatrix(&rotation, M_PI / -2.0f);
	makeTransformFromUniformScale(&scale, 5.0f);
	makeTransformFromTranslation(&translation, 0.0f, 1.0f, 0.0f);
	
	TransformMatrix finalResult;
	concatenateTransform(&finalResult, &scale, &rotation, &translation);
	applyTransformToPoint(&simplePoint, &finalResult);
	printVector(&simplePoint, '\n');
	printf("\n");

	inverseTransformUnscaled(&finalResult);
	applyTransformToPoint(&simplePoint, &finalResult);
	printVector(&simplePoint, '\n');
	printf("\n");
}
