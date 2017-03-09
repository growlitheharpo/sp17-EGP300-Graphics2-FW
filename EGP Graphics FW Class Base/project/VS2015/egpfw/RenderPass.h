#pragma once
#include "render_enums.h"
#include "egpfw/egpfw.h"
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
			T* values;

			UniformData(int l, E t, unsigned int c, T* v) : location(l), type(t), count(c), values(v) {}
		};

		template <typename E, typename T>
		struct UniformMatrixData
		{
			int location;
			E type;
			unsigned int count;
			int transpose;
			T* values;

			UniformMatrixData(int l, E t, unsigned int c, int tr, T* v) : location(l), type(t), count(c), transpose(tr), values(v) {}
		};

		typedef UniformData<egpUniformIntType, int> uniform_int;
		typedef UniformData<egpUniformFloatType, float> uniform_float;
		typedef UniformMatrixData<egpUniformFloatMatrixType, float> uniform_float_matrix;

		struct FBOTargetColorTexture
		{
			egpFrameBufferObjectDescriptor* fbo;
			unsigned int glBinding;
			int targetIndex;

			FBOTargetColorTexture(egpFrameBufferObjectDescriptor* f, unsigned int b, int t) : fbo(f), glBinding(b), targetIndex(t) {}
		};

		struct FBOTargetDepthTexture
		{
			egpFrameBufferObjectDescriptor* fbo;
			unsigned int glBinding;
			
			FBOTargetDepthTexture(egpFrameBufferObjectDescriptor* f, unsigned int b) : fbo(f), glBinding(b) {}
		};
		
	private:
		GLSLProgramIndex mProgram;
		FBOIndex mPipelineStage;
		egpFrameBufferObjectDescriptor* mFBOArray;
		egpProgram* mProgramArray;

		std::vector<uniform_int> mIntUniforms;
		std::vector<uniform_float> mFloatUniforms;
		std::vector<uniform_float_matrix> mFloatMatrixUniforms;

		std::vector<FBOTargetColorTexture> mColorTargets;
		std::vector<FBOTargetDepthTexture> mDepthTargets;

	public:
		RenderPass();
		RenderPass(egpFrameBufferObjectDescriptor* fbos, egpProgram* programs);
		RenderPass(egpFrameBufferObjectDescriptor* fbos, egpProgram* programs, GLSLProgramIndex program, FBOIndex fbo);
		~RenderPass() = default;

		void addUniform(const uniform_int& i) { mIntUniforms.push_back(i); }
		void addUniform(const uniform_float& f) { mFloatUniforms.push_back(f); }
		void addUniform(const uniform_float_matrix& fm) { mFloatMatrixUniforms.push_back(fm); }
		
		void addColorTarget(const FBOTargetColorTexture& ct) { mColorTargets.push_back(ct); }
		void addDepthTarget(const FBOTargetDepthTexture& dt) { mDepthTargets.push_back(dt); }

		void setProgram(GLSLProgramIndex p);
		void setPipelineStage(FBOIndex i);

		void setFBOArray(egpFrameBufferObjectDescriptor* fboArray);
		void setProgramArray(egpProgram* programs);

		void sendData() const;
		void activate() const;
};
