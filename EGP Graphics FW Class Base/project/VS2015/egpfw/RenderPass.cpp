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

RenderPass::RenderPass(egpFrameBufferObjectDescriptor* fbos, egpProgram* programs, int program, int fbo)
{
	mFBOArray = fbos;
	mProgramArray = programs;

	mProgram = program;
	mPipelineStage = fbo;
}

void RenderPass::addUniform(const uniform_int& i)
{
	mIntUniforms.push_back(i);
}

void RenderPass::addUniform(const uniform_float& f)
{
	mFloatUniforms.push_back(f);
}

void RenderPass::addUniform(const uniform_float_matrix& fm)
{
	mFloatMatrixUniforms.push_back(fm);
}

void RenderPass::setProgram(int p)
{
	mProgram = p;
}

void RenderPass::setPipelineStage(int i)
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

	egpActivateProgram(mProgramArray + mProgram);

	egpfwActivateFBO(mFBOArray + mPipelineStage);
}
