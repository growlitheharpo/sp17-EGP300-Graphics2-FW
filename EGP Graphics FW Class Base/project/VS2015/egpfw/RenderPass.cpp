#include "RenderPass.h"
#include <stdexcept>
#include <iostream>

RenderPass::RenderPass(egpFrameBufferObjectDescriptor* fbos, egpProgram* programs)
{
	mFBOArray = fbos;
	mProgramArray = programs;

	mProgram = GLSLProgramCount;
	mPipelineStage = fboCount;

	mAssociatedVAO = nullptr;
}

RenderPass::RenderPass(egpFrameBufferObjectDescriptor* fbos, egpProgram* programs, int program, int fbo)
{
	mFBOArray = fbos;
	mProgramArray = programs;

	mProgram = program;
	mPipelineStage = fbo;

	mAssociatedVAO = nullptr;
}

void RenderPass::addUniform(const render_pass_uniform_int& i)
{
	mIntUniforms.push_back(i);
}

void RenderPass::addUniform(const render_pass_uniform_float& f)
{
	mFloatUniforms.push_back(f);
}

void RenderPass::addUniform(const render_pass_uniform_float_complex& f)
{
	mComplexFloatUniforms.push_back(f);
}

void RenderPass::addUniform(const render_pass_uniform_float_matrix& fm)
{
	mFloatMatrixUniforms.push_back(fm);
}

void RenderPass::addTexture(const RenderPassTextureData& t)
{
	mTextures.push_back(t);
}

void RenderPass::setVAO(egpVertexArrayObjectDescriptor* vao)
{
	mAssociatedVAO = vao;
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
	for (auto target : mColorTargets)
		egpfwBindColorTargetTexture(mFBOArray + target.fboIndex, target.glBinding, target.targetIndex);

	for (auto target : mDepthTargets)
		egpfwBindDepthTargetTexture(mFBOArray + target.fboIndex, target.glBinding);

	for (auto data : mIntUniforms)
		egpSendUniformInt(data.location, data.type, data.count, data.values);//fetchVals(data.values).data());
	
	for (auto data : mFloatUniforms)
		egpSendUniformFloat(data.location, data.type, data.count, data.values);// (data.values).data());

	for (auto data : mComplexFloatUniforms)
		egpSendUniformFloat(data.location, data.type, data.count, fetchVals(data.values).data());

	for (auto data : mFloatMatrixUniforms)
		egpSendUniformFloatMatrix(data.location, UNIF_MAT4, data.count, data.transpose, data.value->m);

	for (auto data : mTextures)
	{
		glActiveTexture(data.textureLane);
		glBindTexture(data.textureType, data.textureHandle);
	}
}

void RenderPass::activate() const
{
	if (mFBOArray == nullptr || mProgramArray == nullptr)
		throw std::invalid_argument("Did not initailize RenderPass");

	if (mProgram != GLSLProgramCount)
		egpActivateProgram(mProgramArray + mProgram);

	if (mPipelineStage != fboCount)
		egpfwActivateFBO(mFBOArray + mPipelineStage);

	if (mAssociatedVAO != nullptr)
		egpActivateVAO(mAssociatedVAO);
}
