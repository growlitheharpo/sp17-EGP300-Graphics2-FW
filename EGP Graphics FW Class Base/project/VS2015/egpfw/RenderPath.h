#pragma once
#include <vector>
#include "RenderPass.h"

namespace cbmath
{
	union mat4;
}

class RenderPath
{
	private:
		std::vector<RenderPass> mPasses;

	public:
		RenderPath();
		~RenderPath();

		void addRenderPass(const RenderPass& pass);
		void addRenderPass(RenderPass&& pass);

		void render();
};
