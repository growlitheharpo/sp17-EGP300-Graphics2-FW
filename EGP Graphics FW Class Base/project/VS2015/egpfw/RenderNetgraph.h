#pragma once
#include "render_enums.h"
#include "egpfw/egpfw/egpfwFrameBuffer.h"
#include "egpfw/egpfw/utils/egpfwVertexBufferUtils.h"
#include "egpfw/egpfw/utils/egpfwShaderProgramUtils.h"
#include <vector>
#include "RenderPath.h"

class RenderNetgraph
{
	private:
		int* mTextureProgramUniforms;
		egpVertexArrayObjectDescriptor* mQuadModel;
		egpFrameBufferObjectDescriptor* mFBOArray;
		egpProgram* mProgramArray;
		std::vector<FBOTargetColorTexture> mFBOsToDraw;

		std::vector<cbmath::mat4> mQuadMVPs;

		RenderPath mInternalRenderPath;

		void generateMVPs();
		void setupRenderPasses();

	public:
		RenderNetgraph(egpFrameBufferObjectDescriptor* fboArray, egpVertexArrayObjectDescriptor* quad, egpProgram* programs, int* textureProgramUniforms);
		~RenderNetgraph() = default;

		void clearFBOList();
		void addFBO(FBOTargetColorTexture fbo);
		void addFBOs(std::initializer_list<FBOTargetColorTexture> fbos);

		void render(int mvpLane, unsigned int* tex);
};
