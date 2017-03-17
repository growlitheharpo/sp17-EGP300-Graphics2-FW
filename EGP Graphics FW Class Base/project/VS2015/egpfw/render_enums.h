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

	// bloom
	bloomBrightProgramIndex,
	bloomBlurProgramIndex,
	bloomBlendProgramIndex,

	// shadow mapping and projective texturing
	projectiveTextureProgram,
	shadowMapProgram,


	// deferred rendering
	gbufferProgramIndex,
	// deferred shading
	deferredShadingProgramIndex,
	// deferred lighting
	deferredLightPassProgramIndex,
	deferredCompositeProgramIndex,

	//-----------------------------
	GLSLProgramCount
};

// framebuffer objects (FBOs)
enum FBOIndex
{
	// scene
	sceneFBO,

	// bloom
	brightFBO_d2,
	hblurFBO_d2,
	vblurFBO_d2,
	hblurFBO_d4,
	vblurFBO_d4,
	hblurFBO_d8,
	vblurFBO_d8,
	compositeFBO,

	// deferred rendering
	gbufferSceneFBO,
	// deferred shading
	deferredShadingFBO,
	// deferred lighting
	lightPassFBO,
	deferredLightingCompositeFBO,

	// shadow map
	shadowFBO,

	//-----------------------------
	fboCount
};
