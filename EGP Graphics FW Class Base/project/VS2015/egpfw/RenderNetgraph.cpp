﻿#include "RenderNetgraph.h"
#include "vector3.h"
#include "transformMatrix.h"
#include <cbmath/cbtkMatrix.h>
#include <GL/glew.h>

void RenderNetgraph::generateMVPs()
{
	mQuadMVPs.clear();

	float scale = 0.5f / mFBOsToDraw.size();
	float pos;

	for (size_t i = 0; i < mFBOsToDraw.size(); i++)
	{
		pos = (i * scale);
		cbmath::mat4 newMatrix = cbmath::makeTranslation4(pos, -pos, 0.0f) * cbmath::makeScale4(scale);

		mQuadMVPs.push_back(newMatrix);
	}
}

void RenderNetgraph::setupRenderPasses()
{
	mInternalRenderPath.clearAllPasses();

	generateMVPs();

	for (size_t i = 0; i < mFBOsToDraw.size(); i++)
	{
		RenderPass p(mFBOArray, mProgramArray);

		p.setProgram(testTextureProgramIndex);
		p.addColorTarget(FBOTargetColorTexture(mFBOsToDraw[i], 0, 0));
		p.addUniform(render_pass_uniform_float_matrix(
			mTextureProgramUniforms[unif_mvp], 1, 0, &mQuadMVPs[i]));
		p.setVAO(mQuadModel);

		mInternalRenderPath.addRenderPass(p);
	}
}

RenderNetgraph::RenderNetgraph(egpFrameBufferObjectDescriptor* fboArray, egpVertexArrayObjectDescriptor* quad, egpProgram* programs, int* textureProgramUniforms)
{
	mFBOArray = fboArray;
	mQuadModel = quad;
	mProgramArray = programs;
	mTextureProgramUniforms = textureProgramUniforms;
}

void RenderNetgraph::clearFBOList()
{
	mFBOsToDraw.clear();
	mInternalRenderPath.clearAllPasses();
}

void RenderNetgraph::addFBO(FBOIndex fbo)
{
	mFBOsToDraw.push_back(fbo);
	setupRenderPasses();
}

void RenderNetgraph::addFBOs(std::initializer_list<FBOIndex> fbos)
{
	for (auto fbo : fbos)
		mFBOsToDraw.push_back(fbo);

	setupRenderPasses();
}

void RenderNetgraph::render(int mvpLane, unsigned int* tex) 
{
	//uhhh...
	//Let's start simple-ish.
	/*
	cbmath::mat4 matrix = cbmath::m4Identity;
	matrix = cbmath::makeTranslation4(0.75f, -0.75f, 0.0f) * cbmath::makeScale4(0.25f);

	egpActivateProgram(&mProgramArray[testTextureProgramIndex]);

	egpfwBindColorTargetTexture(mFBOArray + mFBOsToDraw[0], 0, 0);

	egpSendUniformFloatMatrix(mvpLane, UNIF_MAT4, 1, 0, matrix.m);

	egpActivateVAO(mQuadModel);
	egpDrawActiveVAO();*/
	mInternalRenderPath.render();
}
