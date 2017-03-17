#pragma once
#include "render_enums.h"
#include "egpfw/egpfw/egpfwFrameBuffer.h"
#include "RenderPassData.h"
#include <egpfw/egpfw/utils/egpfwVertexBufferUtils.h>

class RenderPass
{
	public:

	private:
		int mProgram;
		int mPipelineStage;
		egpFrameBufferObjectDescriptor* mFBOArray;
		egpProgram* mProgramArray;

		std::vector<render_pass_uniform_int> mIntUniforms;
		std::vector<render_pass_uniform_float> mFloatUniforms;
		std::vector<render_pass_uniform_float_matrix> mFloatMatrixUniforms;

		std::vector<FBOTargetColorTexture> mColorTargets;
		std::vector<FBOTargetDepthTexture> mDepthTargets;

		std::vector<RenderPassTextureData> mTextures;
		egpVertexArrayObjectDescriptor* mAssociatedVAO;

		template <typename T>
		static std::vector<T> fetchVals(std::vector<T*>& refs);

	public:
		RenderPass(egpFrameBufferObjectDescriptor* fbos, egpProgram* programs);
		RenderPass(egpFrameBufferObjectDescriptor* fbos, egpProgram* programs, int program, int fbo);
		~RenderPass() = default;

		void addUniform(const render_pass_uniform_int& i);
		void addUniform(const render_pass_uniform_float& f);
		void addUniform(const render_pass_uniform_float_matrix& fm);
		
		void addColorTarget(const FBOTargetColorTexture& ct) { mColorTargets.push_back(ct); }
		void addDepthTarget(const FBOTargetDepthTexture& dt) { mDepthTargets.push_back(dt); }

		void addTexture(const RenderPassTextureData& t);
		void setVAO(egpVertexArrayObjectDescriptor* vao);

		void setProgram(GLSLProgramIndex p);
		void setPipelineStage(FBOIndex i);

		void sendData() const;
		void activate() const;

};

template <typename T>
inline std::vector<T> RenderPass::fetchVals(std::vector<T*>& refs)
{
	std::vector<T> result;
	result.reserve(refs.size());

	for (auto iter : refs)
		result.push_back(*iter);

	return result;
}
