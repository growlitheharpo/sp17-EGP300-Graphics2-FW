#include "RenderNetgraph.h"
#include "vector3.h"
#include "transformMatrix.h"
#include <GL/glew.h>
#include <cbmath/cbtkMathUtils.h>

void RenderNetgraph::generateMVPs()
{
	mQuadMVPs.clear();

	float scale = 0.5f / mFBOsToDraw.size();
	float xPos;

	const float START = 0.0f + (scale / 2.0f);
	const float END = 1.0f + scale / 2.0f;// -(scale / 2.0f);

	float yPos = (-1.0f + scale * 1.25f);
	if (yPos < -0.75f)
		yPos = -0.75f;

	for (size_t i = 0; i < mFBOsToDraw.size(); i++)
	{
		float t = (float)i / (float)mFBOsToDraw.size();
		xPos = START + (END - START) * t;

		cbmath::mat4 newMatrix = cbmath::makeTranslation4(xPos, yPos, 0.0f) * cbmath::makeScale4(scale);

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
		p.addColorTarget(mFBOsToDraw[i]);
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

void RenderNetgraph::addFBO(FBOTargetColorTexture fbo)
{
	fbo.glBinding = 0;
	mFBOsToDraw.push_back(fbo);
	setupRenderPasses();
}

void RenderNetgraph::addFBOs(std::initializer_list<FBOTargetColorTexture> fbos)
{
	for (auto fbo : fbos)
	{
		fbo.glBinding = 0;
		mFBOsToDraw.push_back(fbo);
	}

	setupRenderPasses();
}

void RenderNetgraph::render(int mvpLane, unsigned int* tex) 
{
	mInternalRenderPath.render();
}
