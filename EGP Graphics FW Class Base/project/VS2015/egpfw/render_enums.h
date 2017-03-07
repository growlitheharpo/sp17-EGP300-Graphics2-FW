#pragma once

// GLSL program handle objects: 
// very convenient way to organize programs and their uniforms!
enum GLSLProgramIndex
{
	testColorProgramIndex,
	testTextureProgramIndex,
	testTexturePassthruProgramIndex,

	phongProgramIndex,
	silhouetteOutlineProgramIndex,
	celshadeProgramIndex,
	testTransformProgramIndex,

	// shadow mapping and projective texturing
	projectiveTextureProgram,
	shadowMapProgram,

	//-----------------------------
	GLSLProgramCount
};

// framebuffer objects (FBOs)
enum FBOIndex
{
	// scene
	sceneFBO,

	// shadow map
	shadowFBO,

	//-----------------------------
	fboCount
};
