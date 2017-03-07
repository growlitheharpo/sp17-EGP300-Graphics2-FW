#pragma once
#include <vector>
#include "RenderPass.h"

namespace cbmath
{
	union mat4;
}

class RenderPath
{
	public:
		typedef void(*renderFuncPtr)(const cbmath::mat4*, const cbmath::mat4*, const cbmath::mat4*, const unsigned int, const unsigned int, int, int);

	private:
		std::vector<RenderPass> mPasses;

	public:
		RenderPath();
		~RenderPath();

		void addRenderPass(const RenderPass& pass);
		void addRenderPass(RenderPass&& pass);

		void doRender(renderFuncPtr renderFunc, const cbmath::mat4 *mvp_main, const cbmath::mat4 *mvp_other, const cbmath::mat4 *atlas, const unsigned int numOtherMatrices, const unsigned int numSceneObjects, int lighting, int cullBack);
};
