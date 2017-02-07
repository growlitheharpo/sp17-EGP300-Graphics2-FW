#ifndef __TRANSFORM_MATRIX_H
#define __TRANSFORM_MATRIX_H


#ifdef __cplusplus
extern "C"
{
#endif	// __cplusplus

	struct Vector3_struct;
	typedef struct Vector3_struct Vector3;

	typedef struct TransformMatrix_struct{
		float vals[4][4];
	} TransformMatrix;

	void multiplyMatrixByScalar(TransformMatrix* matrix, float scalar);
	void printMatrix(TransformMatrix const* matrix);
	void copyMatrix(TransformMatrix const* src, TransformMatrix* dest);
	void transposeMatrix(TransformMatrix* matrix);
	void makeIdentiyMatrix(TransformMatrix* matrix);
	void makeXDirectionTransformMatrix(TransformMatrix* matrix, float amount);
	void makeYDirectionTransformMatrix(TransformMatrix* matrix, float amount);
	void makeZDirectionTransformMatrix(TransformMatrix* matrix, float amount);
	void makeTransformFromRotation(TransformMatrix* matrix, float pitch, float yaw, float roll);
	void makeTransformFromScale(TransformMatrix* matrix, float x, float y, float z);
	void makeTransformFromUniformScale(TransformMatrix* matrix, float uniformScale);
	void makeTransformFromTranslation(TransformMatrix* matrix, float dX, float dY, float dZ);
	void inverseTransformUnscaled(TransformMatrix* matrix);
	int areTransformsEqual(TransformMatrix* a, TransformMatrix* b);
	void multiplyMatrix(TransformMatrix* matrixOut, TransformMatrix* matrixLeft, TransformMatrix* matrixRight);
	void concatenateTransform(TransformMatrix* matrixOut, TransformMatrix* scaleMatrix, TransformMatrix* rotationMatrix, TransformMatrix* translationMatrix);

	void applyTransformToPoint(Vector3* pointInOut, TransformMatrix const* transform);

	void runTransformTestSuite();


#ifdef __cplusplus
}
#endif	// __cplusplus


#endif