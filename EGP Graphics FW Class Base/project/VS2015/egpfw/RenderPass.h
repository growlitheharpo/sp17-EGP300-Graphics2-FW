#pragma once
#include "render_enums.h"
#include "egpfw/egpfw.h"

class RenderPass
{
	private:
		GLSLProgramIndex mProgram;
		FBOIndex mPipelineStage;
		egpFrameBufferObjectDescriptor* mFBOArray;
		egpProgram* mProgramArray;

	public:
		RenderPass();
		RenderPass(egpFrameBufferObjectDescriptor* fbos, egpProgram* programs);
		RenderPass(egpFrameBufferObjectDescriptor* fbos, egpProgram* programs, GLSLProgramIndex program, FBOIndex fbo);
		~RenderPass() = default;

		void setProgram(GLSLProgramIndex p);
		void setPipelineStage(FBOIndex i);

		void setFBOArray(egpFrameBufferObjectDescriptor* fboArray);
		void setProgramArray(egpProgram* programs);

		void activate() const;
};
