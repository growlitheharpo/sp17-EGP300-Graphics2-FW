#pragma once
#include <vector>
#include "egpfw/egpfw/utils/egpfwShaderProgramUtils.h"
#include <GL/glew.h>
#include <cbmath/cbtkMatrix.h>

template <typename E, typename T>
struct UniformDataSimple
{
	int location;
	E type;
	unsigned int count;
	T* values;

	UniformDataSimple(int l, E t, unsigned int c, T* v) : location(l), type(t), count(c), values(v) {}
};

template <typename E, typename T>
struct UniformDataComplex
{
	int location;
	E type;
	unsigned int count;
	std::vector<T*> values;

	UniformDataComplex(int l, E t, unsigned int c, std::initializer_list<T*> v) : location(l), type(t), count(c), values(v) {}
};


struct UniformMatrixData
{
	int location;
	unsigned int count;
	int transpose;
	cbmath::mat4* value;

	UniformMatrixData(int l, unsigned int c, int tr, cbmath::mat4* v) : location(l), count(c), transpose(tr), value(v) {}
};

struct FBOTargetColorTexture
{
	int fboIndex;
	unsigned int glBinding;
	int targetIndex;

	FBOTargetColorTexture(int f, unsigned int b, int t) : fboIndex(f), glBinding(b), targetIndex(t) {}
};

struct FBOTargetDepthTexture
{
	int fboIndex;
	unsigned int glBinding;

	FBOTargetDepthTexture(int f, unsigned int b) : fboIndex(f), glBinding(b) {}
};

struct RenderPassTextureData
{
	GLenum textureType;
	GLuint textureLane;
	GLuint textureHandle;

	RenderPassTextureData(GLenum type, GLuint lane, GLuint handle) : textureType(type), textureLane(lane), textureHandle(handle) {}
};

typedef UniformDataSimple<egpUniformIntType, int> render_pass_uniform_int;
typedef UniformDataSimple<egpUniformFloatType, float> render_pass_uniform_float;
typedef UniformDataComplex<egpUniformFloatType, float> render_pass_uniform_float_complex;
typedef UniformMatrixData render_pass_uniform_float_matrix;