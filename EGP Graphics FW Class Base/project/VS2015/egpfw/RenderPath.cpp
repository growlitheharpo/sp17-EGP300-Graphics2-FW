#include "RenderPath.h"

RenderPath::RenderPath()
{
}

RenderPath::~RenderPath()
{
}

void RenderPath::addRenderPass(const RenderPass& pass)
{
	mPasses.push_back(pass);
}

void RenderPath::addRenderPass(RenderPass&& pass)
{
	mPasses.push_back(std::move(pass));
}

void RenderPath::doRender(renderFuncPtr func, const cbmath::mat4* mvp_main, const cbmath::mat4* mvp_other, const cbmath::mat4* atlas, const unsigned numOtherMatrices, const unsigned numSceneObjects, int lighting, int cullBack)
{
	for (auto pass : mPasses)
	{
		pass.activate();
		(*func)(mvp_main, mvp_other, atlas, numOtherMatrices, numSceneObjects, lighting, cullBack);
	}
}
