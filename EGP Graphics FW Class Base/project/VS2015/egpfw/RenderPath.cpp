#include "RenderPath.h"
#include "egpfw/egpfw.h"

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

void RenderPath::render()
{
	for (auto pass : mPasses)
	{
		pass.activate();
		pass.sendData();
		egpDrawActiveVAO();
	}
}
