// By Dan Buckstein
// Modified by: _______________________________________________________________
#include "egpfw/egpfw/egpfwFrameBuffer.h"


// OpenGL
#ifdef _WIN32
#include "GL/glew.h"
#else	// !_WIN32
#include <OpenGL/gl3.h>
#endif	// _WIN32


#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// these are the colour targets for OpenGL
const unsigned int egpfwTargetName[] = {
	GL_COLOR_ATTACHMENT0,	GL_COLOR_ATTACHMENT1,	GL_COLOR_ATTACHMENT2,	GL_COLOR_ATTACHMENT3,
	GL_COLOR_ATTACHMENT4,	GL_COLOR_ATTACHMENT5,	GL_COLOR_ATTACHMENT6,	GL_COLOR_ATTACHMENT7,
	GL_COLOR_ATTACHMENT8,	GL_COLOR_ATTACHMENT9,	GL_COLOR_ATTACHMENT10,	GL_COLOR_ATTACHMENT11,
	GL_COLOR_ATTACHMENT12,	GL_COLOR_ATTACHMENT13,	GL_COLOR_ATTACHMENT14,	GL_COLOR_ATTACHMENT15,
};

// color formats
const unsigned int egpfwInternalColorFormat[] = {
	0, GL_RGB8, GL_RGB16, GL_RGB32F, GL_RGBA8, GL_RGBA16, GL_RGBA32F,
};

// color storage type
const unsigned int egpfwInternalColorStorage[] = {
	0, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_FLOAT
};

// depth formats
const unsigned int egpfwInternalDepthFormat[] = {
	0, GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32F, GL_DEPTH24_STENCIL8,
};

// depth storage type
const unsigned int egpfwInternalDepthStorage[] = {
	0, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_INT
};


//-----------------------------------------------------------------------------

// ****
egpFrameBufferObjectDescriptor egpfwCreateFBO(const unsigned int frameWidth, const unsigned int frameHeight, const unsigned int numColorTargets, const egpColorFormat colorFormat, const egpDepthFormat depthFormat, const egpWrapSmoothFormat wrapSmoothFormat)
{
	egpFrameBufferObjectDescriptor fbo = { 0 };
	//...
	return fbo;
}


// ****
void egpfwActivateFBO(const egpFrameBufferObjectDescriptor *fbo)
{
	//...
}


// ****
int egpfwReleaseFBO(egpFrameBufferObjectDescriptor *fbo)
{
	//...
	return 0;
}


// ****
int egpfwBindColorTargetTexture(const egpFrameBufferObjectDescriptor *fbo, const unsigned int glBinding, const unsigned int targetIndex)
{
	//...
	return 0;
}

// ****
int egpfwBindDepthTargetTexture(const egpFrameBufferObjectDescriptor *fbo, const unsigned int glBinding)
{
	//...
	return 0;
}
