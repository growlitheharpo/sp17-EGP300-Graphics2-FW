#pragma once
#include "render_enums.h"
#include "egpfw/egpfw/utils/egpfwShaderProgramUtils.h"
#include "egpfw/egpfw/egpfwFrameBuffer.h"
#include <vector>

class RenderPass
{
	public:
		template <typename E, typename T>
		struct UniformData
		{
			int location;
			E type;
			unsigned int count;
			std::vector<T*> values;

			UniformData(int l, E t, unsigned int c, std::initializer_list<T*> v) : location(l), type(t), count(c), values(v) {}
		};

		template <typename E, typename T>
		struct UniformMatrixData
		{
			int location;
			E type;
			unsigned int count;
			int transpose;
			std::vector<T*> values;

			UniformMatrixData(int l, E t, unsigned int c, int tr, std::initializer_list<T*> v) : location(l), type(t), count(c), transpose(tr), values(v) {}
		};

		typedef UniformData<egpUniformIntType, int> uniform_int;
		typedef UniformData<egpUniformFloatType, float> uniform_float;
		typedef UniformMatrixData<egpUniformFloatMatrixType, float> uniform_float_matrix;

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
		
	private:
		int mProgram;
		int mPipelineStage;
		egpFrameBufferObjectDescriptor* mFBOArray;
		egpProgram* mProgramArray;

		std::vector<uniform_int> mIntUniforms;
		std::vector<uniform_float> mFloatUniforms;
		std::vector<uniform_float_matrix> mFloatMatrixUniforms;

		std::vector<FBOTargetColorTexture> mColorTargets;
		std::vector<FBOTargetDepthTexture> mDepthTargets;

		template <typename T>
		static std::vector<T> fetchVals(std::vector<T*>& refs);

	public:
		RenderPass(egpFrameBufferObjectDescriptor* fbos, egpProgram* programs);
		RenderPass(egpFrameBufferObjectDescriptor* fbos, egpProgram* programs, int program, int fbo);
		~RenderPass() = default;

		void addUniform(const uniform_int& i);
		void addUniform(const uniform_float& f);
		void addUniform(const uniform_float_matrix& fm);
		
		void addColorTarget(const FBOTargetColorTexture& ct) { mColorTargets.push_back(ct); }
		void addDepthTarget(const FBOTargetDepthTexture& dt) { mDepthTargets.push_back(dt); }

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
