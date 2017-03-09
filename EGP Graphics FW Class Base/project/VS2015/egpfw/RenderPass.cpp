#include "RenderPass.h"
#include <stdexcept>
#include <iostream>

RenderPass::RenderPass(egpFrameBufferObjectDescriptor* fbos, egpProgram* programs)
{
	mFBOArray = fbos;
	mProgramArray = programs;

	mProgram = GLSLProgramCount;
	mPipelineStage = fboCount;
}

RenderPass::RenderPass(egpFrameBufferObjectDescriptor* fbos, egpProgram* programs, GLSLProgramIndex program, FBOIndex fbo)
{
	mFBOArray = fbos;
	mProgramArray = programs;

	mProgram = program;
	mPipelineStage = fbo;
}

void RenderPass::setProgram(GLSLProgramIndex p)
{
	mProgram = p;
}

void RenderPass::setPipelineStage(FBOIndex i)
{
	mPipelineStage = i;
}

void RenderPass::sendData() const
{
	for (auto data : mIntUniforms)
		egpSendUniformInt(data.location, data.type, data.count, data.values);
	
	for (auto data : mFloatUniforms)
		egpSendUniformFloat(data.location, data.type, data.count, data.values);

	for (auto data : mFloatMatrixUniforms)
		egpSendUniformFloatMatrix(data.location, data.type, data.count, data.transpose, data.values);

	for (auto target : mColorTargets)
	{
		if (egpfwBindColorTargetTexture(target.fbo, target.glBinding, target.targetIndex) == false)
			std::cout << "ERROR!! COULD NOT BIND A COLOR TARGET!" << std::endl;
	}

	for (auto target : mDepthTargets)
		egpfwBindDepthTargetTexture(target.fbo, target.glBinding);
}

void RenderPass::activate() const
{
	if (mFBOArray == nullptr || mProgramArray == nullptr)
		throw std::invalid_argument("Did not initailize RenderPass");

	egpfwActivateFBO(mFBOArray + mPipelineStage);
	egpActivateProgram(mProgramArray + mProgram);
}
