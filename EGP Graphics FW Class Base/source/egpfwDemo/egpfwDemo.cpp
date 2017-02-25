///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////	EGP Graphics Framework Demo
////	By Dan Buckstein
////	January 2017
////
////	****Modified by: ______________________________________________________
////	^^^^
////	NOTE: 4 asterisks anywhere means something you will have to either 
////		modify or complete to get it working! Good practice for starters: 
////		write your name beside "modified by" above!
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "egpfwDemo.h"

//-----------------------------------------------------------------------------
// external includes

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// third party math lib
#include "cbmath/cbtkMatrix.h"

// OpenGL
#ifdef _WIN32
#include "GL/glew.h"
#else	// !_WIN32
#include <OpenGL/gl3.h>
#endif	// _WIN32

// DevIL (images)
#include "IL/ilut.h"


//-----------------------------------------------------------------------------
// framework includes

#include "egpfw/egpfw.h"


//-----------------------------------------------------------------------------
// globals


//-----------------------------------------------------------------------------

// window info
unsigned int win_x = 0;
unsigned int win_y = 0;
unsigned int win_w = 1280;
unsigned int win_h = 720;
const unsigned int viewport_b = 8;
const unsigned int viewport_tb = viewport_b + viewport_b;
const int viewport_nb = -(int)viewport_b;
unsigned int viewport_tw = win_w + viewport_tb;
unsigned int viewport_th = win_h + viewport_tb;
float win_aspect;
float fovy = 1.0f, znear = 0.5f, zfar = 50.0f;
float maxClipDist = 0.0f, minClipDist = 0.0f;

// update flag: play speed as a percentage
unsigned int playing = 1;
unsigned int playrate = 100;


// timers
egpTimer renderTimer[1], secondTimer[1];

// controls
egpMouse mouse[1];
egpKeyboard keybd[1];


// general: camera's view matrix and projection matrix
cbtk::cbmath::mat4 viewMatrix, projectionMatrix, viewProjMat;
// camera controls
float cameraElevation = 0.25f, cameraAzimuth = 4.0f;
float cameraRotateSpeed = 0.25f, cameraMoveSpeed = 4.0f, cameraDistance = 8.0f;
cbtk::cbmath::vec4 cameraPosWorld(1.0f, -2.0f, -cameraDistance, 1.0f), deltaCamPos;



//-----------------------------------------------------------------------------
// graphics-related data and handles
// good practice: default values for everything

// test vertex buffers for built-in and loaded primitive data
// use same model index for VAO and VBO (since VAOs only work 
//	if the respective VBO has been created and associated)
enum ModelIndex
{
	// built-in models
	axesModel, 
	fsqModel, 
	skyboxModel, sphere8x6Model, sphere32x24Model,

	// loaded models
	sphereLowResObjModel,
	sphereHiResObjModel,

//-----------------------------
	modelCount
};
egpVertexArrayObjectDescriptor vao[modelCount] = { 0 };
egpVertexBufferObjectDescriptor vbo[modelCount] = { 0 };


// loaded textures
enum TextureIndex
{
	skyboxTexHandle,
	earthTexHandle_dm, earthTexHandle_sm,
	moonTexHandle_dm,
	celshadeRamp,

	//-----------------------------
	textureCount
};
unsigned int tex[textureCount] = { 0 };


// GLSL program handle objects: 
// very convenient way to organize programs and their uniforms!
enum GLSLProgramIndex
{
	testColorProgramIndex,
	testTextureProgramIndex,
	testTexturePassthruProgramIndex, 

	phongProgramIndex,
	celshadeProgramIndex,

	// bloom
	bloomBrightProgramIndex, 
	bloomBlurProgramIndex, 
	bloomBlendProgramIndex, 

//-----------------------------
	GLSLProgramCount
};
enum GLSLCommonUniformIndex
{
	unif_mvp,

	unif_lightPos,
	unif_eyePos,

	unif_dm,
	unif_sm,

	unif_pixelSizeInv, 
	unif_img, 
	unif_img1, 
	unif_img2, 
	unif_img3, 

//-----------------------------
	GLSLCommonUniformCount
};
egpProgram glslPrograms[GLSLProgramCount] = { 0 }, *currentProgram = 0;
int glslCommonUniforms[GLSLProgramCount][GLSLCommonUniformCount] = { -1 }, *currentUniformSet = 0;
int currentProgramIndex = 0;


// framebuffer objects (FBOs)
enum FBOIndex
{
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
	
//-----------------------------
	fboCount
};
egpFrameBufferObjectDescriptor fbo[fboCount], *fboFinalDisplay = fbo;
cbmath::vec2 pixelSizeInv[fboCount];
int testDrawAxes = 0;
int displayMode = 0;



//-----------------------------------------------------------------------------
// our game objects

// transformation matrices
cbmath::mat4 skyboxModelViewMatrix, skyboxModelViewProjectionMatrix;
cbmath::mat4 earthModelMatrix, earthModelViewProjectionMatrix, earthModelInverseMatrix;
cbmath::mat4 moonModelMatrix, moonModelViewProjectionMatrix, moonModelInverseMatrix;


// light and camera for shading
cbmath::vec4 lightPos_world(10.0f, 10.0f, 20.0f, 1.0f), lightPos_object, eyePos_object;


// raw animation values: 
float earthDaytime = 0.0f;
float earthOrbit = 0.0f;
const float earthAnimateRate = 100.0f;
const float earthDaytimePeriod = earthAnimateRate / 24.0f;
const float earthOrbitPeriod = earthDaytimePeriod / 365.25f;
const float earthTilt = Deg2Rad(23.44f);
const float earthDistance = 6.0f;

float moonOrbit = 0.0f;
const float moonOrbitPeriod = earthDaytimePeriod / 27.32f;
const float moonTilt = Deg2Rad(6.69f);
const float moonDistance = 4.0f;
const float moonSize = 0.27f;



//-----------------------------------------------------------------------------
// game functions

// access window info
unsigned int windowWidth()
{
	return win_w;
}

unsigned int windowHeight()
{
	return win_h;
}

unsigned int windowPosX()
{
	return win_x;
}

unsigned int windowPosY()
{
	return win_y;
}


// set initial GL states
int initGL()
{
	const float lineWidth = 4.0f;
	const float pointSize = lineWidth;

	// textures ON
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);

	// backface culling ON
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// depth testing OFF
	glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// alpha blending OFF
	// result = ( new*[new alpha] ) + ( old*[1 - new alpha] )
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// lines and points
	glLineWidth(lineWidth);
	glPointSize(pointSize);

	// done
	return 1;
}


// initialize third-party image handling using DevIL
int initIL()
{
	ilInit();
	iluInit();
	ilutInit();
	ilutRenderer(ILUT_OPENGL);

	// done
	return 1;
}


// display OpenGL version
void printGLVersion()
{
	printf("%s", glGetString(GL_VERSION));
}



//-----------------------------------------------------------------------------

// load and delete geometry
void setupGeometry()
{
	// prepare sample geometry
	// all of these are statically defined, so there is no cleanup for them
	egpInitializeCube();
	egpInitializeCubeIndexed();
	egpInitializeWireCube();
	egpInitializeWireCubeIndexed();
	egpInitializeSphere8x6();
	egpInitializeSphere32x24();


	// prepare sample geometry using VBOs
	// these should be cleaned

	// good news is they all use the same attribute descriptors, 
	//	so prepare those first
	egpAttributeDescriptor attribs[] = {
		egpCreateAttributeDescriptor(ATTRIB_POSITION, ATTRIB_VEC3, 0),
		egpCreateAttributeDescriptor(ATTRIB_COLOR, ATTRIB_VEC3, 0),
		egpCreateAttributeDescriptor(ATTRIB_NORMAL, ATTRIB_VEC3, 0),
		egpCreateAttributeDescriptor(ATTRIB_TEXCOORD, ATTRIB_VEC2, 0),
	};

	// axes
	attribs[0].data = egpGetAxesPositions();
	attribs[1].data = egpGetAxesColors();
	vao[axesModel] = egpCreateVAOInterleaved(PRIM_LINES, attribs, 2, egpGetAxesVertexCount(), (vbo + axesModel), 0);

	// skybox
	attribs[0].data = egpGetCubePositions();
	attribs[1].data = egpGetCubeColors();
	attribs[2].data = egpGetCubeNormals();
	attribs[3].data = egpGetCubeTexcoords();
	vao[skyboxModel] = egpCreateVAOInterleaved(PRIM_TRIANGLES, attribs, 4, egpGetCubeVertexCount(), (vbo + skyboxModel), 0);

	// low-res sphere
	attribs[0].data = egpGetSphere8x6Positions();
	attribs[1].data = egpGetSphere8x6Colors();
	attribs[2].data = egpGetSphere8x6Normals();
	attribs[3].data = egpGetSphere8x6Texcoords();
	vao[sphere8x6Model] = egpCreateVAOInterleaved(PRIM_TRIANGLES, attribs, 4, egpGetSphere8x6VertexCount(), (vbo + sphere8x6Model), 0);

	// high-res sphere
	attribs[0].data = egpGetSphere32x24Positions();
	attribs[1].data = egpGetSphere32x24Colors();
	attribs[2].data = egpGetSphere32x24Normals();
	attribs[3].data = egpGetSphere32x24Texcoords();
	vao[sphere32x24Model] = egpCreateVAOInterleaved(PRIM_TRIANGLES, attribs, 4, egpGetSphere32x24VertexCount(), (vbo + sphere32x24Model), 0);

	// full-screen quad (positions and texcoords only!)
	attribs[1] = attribs[3];
	attribs[0].data = egpfwGetUnitQuadPositions();
	attribs[1].data = egpfwGetUnitQuadTexcoords();
	vao[fsqModel] = egpCreateVAOInterleaved(PRIM_TRIANGLE_STRIP, attribs, 2, 4, (vbo + fsqModel), 0);


	// loaded models
	// attempt to load binary first; if failed, load object and save binary
	// binary save/load is not necessary, but it is very fast
	egpTriOBJDescriptor obj[1];

	// low-res sphere
	*obj = egpfwLoadBinaryOBJ("sphere8x6_bin.txt");
	if (!obj->data)
	{
		*obj = egpfwLoadTriangleOBJ("../../../../resource/obj/sphere8x6.obj", NORMAL_LOAD, 1.0);
		egpfwSaveBinaryOBJ(obj, "sphere8x6_bin.txt");
	}
	egpfwCreateVAOFromOBJ(obj, vao + sphereLowResObjModel, vbo + sphereLowResObjModel);
	egpfwReleaseOBJ(obj);

	// high-res sphere
	*obj = egpfwLoadBinaryOBJ("sphere32x24_bin.txt");
	if (!obj->data)
	{
		*obj = egpfwLoadTriangleOBJ("../../../../resource/obj/sphere32x24.obj", NORMAL_LOAD, 1.0);
		egpfwSaveBinaryOBJ(obj, "sphere32x24_bin.txt");
	}
	egpfwCreateVAOFromOBJ(obj, vao + sphereHiResObjModel, vbo + sphereHiResObjModel);
	egpfwReleaseOBJ(obj);
}

void deleteGeometry()
{
	unsigned int i;
	for (i = 0; i < modelCount; ++i)
	{
		egpReleaseVAO(vao + i);
		egpReleaseVBO(vbo + i);
	}
}


// setup and delete textures
void setupTextures()
{
	// here we can change the texture settings: 
	//	1. smoothing: do we want it to appear 
	//		rough/pixelated (GL_NEAREST) or smooth (GL_LINEAR)?
	//	2. do we want it to repeat (GL_REPEAT) 
	//		or clamp to the edges (GL_CLAMP) if texcoords are 
	//		less than 0 or greater than 1?

	// files
	char *imgFiles[] = {
		(char *)("../../../../resource/tex/bg/sky_clouds.png"),
		(char *)("../../../../resource/tex/earth/2k/earth_dm_2k.png"),
		(char *)("../../../../resource/tex/earth/2k/earth_sm_2k.png"),
		(char *)("../../../../resource/tex/moon/2k/moon_dm_2k.png"),
		(char *)("../../../../resource/tex/cel/test_cel_gradient.png"),
	};

	// load
	unsigned int ilHandle[textureCount] = { 0 }, status = 0, imgW = 0, imgH = 0;
	const unsigned char *data = 0;
	ilGenImages(textureCount, ilHandle);
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
	glGenTextures(textureCount, tex);
	for (unsigned int i = 0; i < textureCount; ++i)
	{
		ilBindImage(ilHandle[i]);
		status = ilLoadImage(imgFiles[i]);
		if (status)
		{
			ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
			imgW = ilGetInteger(IL_IMAGE_WIDTH);
			imgH = ilGetInteger(IL_IMAGE_HEIGHT);
			data = ilGetData();

			glBindTexture(GL_TEXTURE_2D, tex[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, imgW, imgH, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
	}
	ilDeleteImages(textureCount, ilHandle);


	// skybox
	glBindTexture(GL_TEXTURE_2D, tex[skyboxTexHandle]);					// activate 2D texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// texture gets small/large, smooth
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);		// texture repeats on horiz axis
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


	// earth textures
	glBindTexture(GL_TEXTURE_2D, tex[earthTexHandle_dm]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);		// these two are deliberately different
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, tex[earthTexHandle_sm]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	// moon textures
	glBindTexture(GL_TEXTURE_2D, tex[moonTexHandle_dm]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//cel ramp texture
	glBindTexture(GL_TEXTURE_2D, tex[celshadeRamp]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// disable textures
	glBindTexture(GL_TEXTURE_2D, 0);
}

void deleteTextures()
{
	// delete all textures at once
	glDeleteTextures(textureCount, tex);
}


// setup and delete shaders
void setupShaders()
{
	unsigned int u = 0;
	egpFileInfo files[4];
	egpShader shaders[4];

	// array of common uniform names
	const char *commonUniformName[] = {
		(const char *)("mvp"),
		(const char *)("lightPos"),
		(const char *)("eyePos"),
		(const char *)("tex_dm"),
		(const char *)("tex_sm"),
		(const char *)("pixelSizeInv"),
		(const char *)("img"),
		(const char *)("img1"),
		(const char *)("img2"),
		(const char *)("img3"),
	};

	const int imageLocations[] = {
		0, 1, 2, 3, 4, 5, 6, 7
	};


	// activate a VAO for validation
	egpActivateVAO(vao + sphere8x6Model);


	// test color program
	{
		// load files
		files[0] = egpLoadFileContents("../../../../resource/glsl/4x/vs/passColor_vs4x.glsl");
		files[1] = egpLoadFileContents("../../../../resource/glsl/4x/fs/drawColor_fs4x.glsl");

		// create shaders
		shaders[0] = egpCreateShaderFromSource(EGP_SHADER_VERTEX, files[0].contents);
		shaders[1] = egpCreateShaderFromSource(EGP_SHADER_FRAGMENT, files[1].contents);

		currentProgramIndex = testColorProgramIndex;
		currentProgram = glslPrograms + currentProgramIndex;

		// create, link and validate program
		*currentProgram = egpCreateProgram();
		egpAttachShaderToProgram(currentProgram, shaders + 0);
		egpAttachShaderToProgram(currentProgram, shaders + 1);
		egpLinkProgram(currentProgram);
		egpValidateProgram(currentProgram);

		// release shaders and files
		egpReleaseShader(shaders + 0);
		egpReleaseShader(shaders + 1);
		egpReleaseFileContents(files + 0);
		egpReleaseFileContents(files + 1);
	}

	// test texture program
	// example of shared shader
	{
		// load shared shader
		files[1] = egpLoadFileContents("../../../../resource/glsl/4x/fs/drawTexture_fs4x.glsl");
		shaders[1] = egpCreateShaderFromSource(EGP_SHADER_FRAGMENT, files[1].contents);

		// pass texcoord
		{
			files[0] = egpLoadFileContents("../../../../resource/glsl/4x/vs/passTexcoord_vs4x.glsl");
			shaders[0] = egpCreateShaderFromSource(EGP_SHADER_VERTEX, files[0].contents);

			// same as previous
			currentProgramIndex = testTextureProgramIndex;
			currentProgram = glslPrograms + currentProgramIndex;

			*currentProgram = egpCreateProgram();
			egpAttachShaderToProgram(currentProgram, shaders + 0);
			egpAttachShaderToProgram(currentProgram, shaders + 1);
			egpLinkProgram(currentProgram);
			egpValidateProgram(currentProgram);

			// release unique shaders and files
			egpReleaseShader(shaders + 0);
			egpReleaseFileContents(files + 0);
		}

		// pass texcoord, passthru position
		{
			files[0] = egpLoadFileContents("../../../../resource/glsl/4x/vs/passTexcoord_passthruPosition_vs4x.glsl");
			shaders[0] = egpCreateShaderFromSource(EGP_SHADER_VERTEX, files[0].contents);

			currentProgramIndex = testTexturePassthruProgramIndex;
			currentProgram = glslPrograms + currentProgramIndex;

			*currentProgram = egpCreateProgram();
			egpAttachShaderToProgram(currentProgram, shaders + 0);
			egpAttachShaderToProgram(currentProgram, shaders + 1);
			egpLinkProgram(currentProgram);
			egpValidateProgram(currentProgram);

			// bloom should use this vertex shader, so let's keep nesting!
			{
				{
					files[2] = egpLoadFileContents("../../../../resource/glsl/4x/fs_bloom/brightpass_fs4x.glsl");
					shaders[2] = egpCreateShaderFromSource(EGP_SHADER_FRAGMENT, files[2].contents);

					currentProgramIndex = bloomBrightProgramIndex;
					currentProgram = glslPrograms + currentProgramIndex;

					*currentProgram = egpCreateProgram();
					egpAttachShaderToProgram(currentProgram, shaders + 0);
					egpAttachShaderToProgram(currentProgram, shaders + 2);
					egpLinkProgram(currentProgram);
					egpValidateProgram(currentProgram);

					egpReleaseShader(shaders + 2);
					egpReleaseFileContents(files + 2);
				}
				{
					files[2] = egpLoadFileContents("../../../../resource/glsl/4x/fs_bloom/blur_gaussian_fs4x.glsl");
					shaders[2] = egpCreateShaderFromSource(EGP_SHADER_FRAGMENT, files[2].contents);

					currentProgramIndex = bloomBlurProgramIndex;
					currentProgram = glslPrograms + currentProgramIndex;

					*currentProgram = egpCreateProgram();
					egpAttachShaderToProgram(currentProgram, shaders + 0);
					egpAttachShaderToProgram(currentProgram, shaders + 2);
					egpLinkProgram(currentProgram);
					egpValidateProgram(currentProgram);

					egpReleaseShader(shaders + 2);
					egpReleaseFileContents(files + 2);
				}
				{
					files[2] = egpLoadFileContents("../../../../resource/glsl/4x/fs_bloom/blend_screen_fs4x.glsl");
					shaders[2] = egpCreateShaderFromSource(EGP_SHADER_FRAGMENT, files[2].contents);

					currentProgramIndex = bloomBlendProgramIndex;
					currentProgram = glslPrograms + currentProgramIndex;

					*currentProgram = egpCreateProgram();
					egpAttachShaderToProgram(currentProgram, shaders + 0);
					egpAttachShaderToProgram(currentProgram, shaders + 2);
					egpLinkProgram(currentProgram);
					egpValidateProgram(currentProgram);

					egpReleaseShader(shaders + 2);
					egpReleaseFileContents(files + 2);
				}
			}

			// done with passthru vs
			egpReleaseShader(shaders + 0);
			egpReleaseFileContents(files + 0);
		}

		// release shared
		egpReleaseShader(shaders + 1);
		egpReleaseFileContents(files + 1);
	}

	// lighting
	{
		currentProgramIndex = phongProgramIndex;
		currentProgram = glslPrograms + currentProgramIndex;

		files[0] = egpLoadFileContents("../../../../resource/glsl/4x/vs/phong_vs4x.glsl");
		files[1] = egpLoadFileContents("../../../../resource/glsl/4x/fs/phong_fs4x.glsl");
		shaders[0] = egpCreateShaderFromSource(EGP_SHADER_VERTEX, files[0].contents);
		shaders[1] = egpCreateShaderFromSource(EGP_SHADER_FRAGMENT, files[1].contents);

		*currentProgram = egpCreateProgram();
		egpAttachShaderToProgram(currentProgram, shaders + 0);
		egpAttachShaderToProgram(currentProgram, shaders + 1);
		egpLinkProgram(currentProgram);
		egpValidateProgram(currentProgram);

		egpReleaseShader(shaders + 0);
		egpReleaseShader(shaders + 1);
		egpReleaseFileContents(files + 0);
		egpReleaseFileContents(files + 1);
	}
	{
		currentProgramIndex = celshadeProgramIndex;
		currentProgram = glslPrograms + currentProgramIndex;

		files[0] = egpLoadFileContents("../../../../resource/glsl/4x/vs/celshade_vs4x.glsl");
		files[1] = egpLoadFileContents("../../../../resource/glsl/4x/fs/celshade_fs4x.glsl");
		shaders[0] = egpCreateShaderFromSource(EGP_SHADER_VERTEX, files[0].contents);
		shaders[1] = egpCreateShaderFromSource(EGP_SHADER_FRAGMENT, files[1].contents);

		*currentProgram = egpCreateProgram();
		egpAttachShaderToProgram(currentProgram, shaders + 0);
		egpAttachShaderToProgram(currentProgram, shaders + 1);
		egpLinkProgram(currentProgram);
		egpValidateProgram(currentProgram);

		egpReleaseShader(shaders + 0);
		egpReleaseShader(shaders + 1);
		egpReleaseFileContents(files + 0);
		egpReleaseFileContents(files + 1);
	}

	// configure all uniforms at once
	for (currentProgramIndex = 0; currentProgramIndex < GLSLProgramCount; ++currentProgramIndex)
	{
		// get location of every uniform
		currentProgram = glslPrograms + currentProgramIndex;
		currentUniformSet = glslCommonUniforms[currentProgramIndex];
		egpActivateProgram(currentProgram);
		for (u = 0; u < GLSLCommonUniformCount; ++u)
			currentUniformSet[u] = egpGetUniformLocation(currentProgram, commonUniformName[u]);

		// bind constant uniform locations, if they exist, because they never change
		// e.g. image bindings
		egpSendUniformInt(currentUniformSet[unif_dm], UNIF_INT, 1, imageLocations);
		egpSendUniformInt(currentUniformSet[unif_sm], UNIF_INT, 1, imageLocations + 1);

		egpSendUniformInt(currentUniformSet[unif_img], UNIF_INT, 1, imageLocations);
		egpSendUniformInt(currentUniformSet[unif_img1], UNIF_INT, 1, imageLocations + 1);
		egpSendUniformInt(currentUniformSet[unif_img2], UNIF_INT, 1, imageLocations + 2);
		egpSendUniformInt(currentUniformSet[unif_img3], UNIF_INT, 1, imageLocations + 3);
	}


	// disable all
	egpActivateProgram(0);
	egpActivateVAO(0);
}

void deleteShaders()
{
	// convenient way to release all programs
	unsigned int i;
	for (i = 0; i < GLSLProgramCount; ++i)
		egpReleaseProgram(glslPrograms + i);
}


// setup and delete framebuffers
void setupFramebuffers(unsigned int frameWidth, unsigned int frameHeight)
{
	// prepare framebuffers in one simple call
	const unsigned int frameWidth_d2 = frameWidth / 2;
	const unsigned int frameWidth_d4 = frameWidth / 4;
	const unsigned int frameWidth_d8 = frameWidth / 8;
	const unsigned int frameHeight_d2 = frameHeight / 2;
	const unsigned int frameHeight_d4 = frameHeight / 4;
	const unsigned int frameHeight_d8 = frameHeight / 8;
	unsigned int i;

	// one for the scene
	fbo[sceneFBO] = egpfwCreateFBO(frameWidth, frameHeight, 1, COLOR_RGBA16, DEPTH_D32, SMOOTH_NOWRAP);

	// bright pass
	fbo[brightFBO_d2] = egpfwCreateFBO(frameWidth_d2, frameHeight_d2, 1, COLOR_RGBA16, DEPTH_DISABLE, SMOOTH_NOWRAP);

	// blur passes
	fbo[hblurFBO_d2] = egpfwCreateFBO(frameWidth_d2, frameHeight_d2, 1, COLOR_RGBA16, DEPTH_DISABLE, SMOOTH_NOWRAP);
	fbo[vblurFBO_d2] = egpfwCreateFBO(frameWidth_d2, frameHeight_d2, 1, COLOR_RGBA16, DEPTH_DISABLE, SMOOTH_NOWRAP);
	fbo[hblurFBO_d4] = egpfwCreateFBO(frameWidth_d4, frameHeight_d4, 1, COLOR_RGBA16, DEPTH_DISABLE, SMOOTH_NOWRAP);
	fbo[vblurFBO_d4] = egpfwCreateFBO(frameWidth_d4, frameHeight_d4, 1, COLOR_RGBA16, DEPTH_DISABLE, SMOOTH_NOWRAP);
	fbo[hblurFBO_d8] = egpfwCreateFBO(frameWidth_d8, frameHeight_d8, 1, COLOR_RGBA16, DEPTH_DISABLE, SMOOTH_NOWRAP);
	fbo[vblurFBO_d8] = egpfwCreateFBO(frameWidth_d8, frameHeight_d8, 1, COLOR_RGBA16, DEPTH_DISABLE, SMOOTH_NOWRAP);

	// composite pass
	fbo[compositeFBO] = egpfwCreateFBO(frameWidth, frameHeight, 1, COLOR_RGBA16, DEPTH_DISABLE, SMOOTH_NOWRAP);


	// get inverted frame sizes
	// this represents the size of one pixel within SCREEN SPACE
	// since screen space is within [0, 1], one pixel = 1/size
	for (i = 0; i < fboCount; ++i)
		pixelSizeInv[i].set(
			1.0f / (float)(fbo + i)->frameWidth, 
			1.0f / (float)(fbo + i)->frameHeight
		);
}

void deleteFramebuffers()
{
	// how convenient
	for (unsigned int i = 0; i < fboCount; ++i)
		egpfwReleaseFBO(fbo + i);
}


// restart all timers and time counters
void resetTimers()
{
	const double renderFPS = 30.0;
	const double secondFPS = 1.0;

	egpTimerSet(renderTimer, renderFPS);
	egpTimerStart(renderTimer);

	egpTimerSet(secondTimer, secondFPS);
	egpTimerStart(secondTimer);
}


// configure viewport for correctly drawing to the back buffer
void drawToBackBuffer(int x, int y, unsigned int w, unsigned int h)
{
	// tell OpenGL to draw directly to back buffer
	glDrawBuffer(GL_BACK);

	// reset buffer tests
	// assume only FBOs are using depth
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);

	// reset viewport with borders clipped
	glViewport(x, y, w, h);
}


//-----------------------------------------------------------------------------

// update camera
void updateCameraControlled(float dt, egpMouse *mouse)
{
	// TRACK MOUSE AND KEYS TO CONTROL CAMERA: 
	// remember, the view transform is the INVERSE of the camera's model transform, 
	//	so we can start by making a proper camera transform then invert it
	// 1. compute deltas (velocities)
	// 2. integrate!
	// 3. update view matrix (camera's transformation)
	if (egpMouseIsButtonDown(mouse, 0))
	{
		cameraElevation -= (float)egpMouseDeltaY(mouse) * dt * cameraRotateSpeed;
		cameraAzimuth -= (float)egpMouseDeltaX(mouse) * dt * cameraRotateSpeed;
	}

	viewMatrix = cbtk::cbmath::makeRotationEuler4ZYX(cameraElevation, cameraAzimuth, 0.0f);

	// apply current rotation to our movement vector so that "forward" is the direction the camera is facing
	deltaCamPos.set(
		(float)egpKeyboardDifference(keybd, 'd', 'a'),
		(float)egpKeyboardDifference(keybd, 'e', 'q'),
		(float)egpKeyboardDifference(keybd, 's', 'w'),
		0.0f);
	deltaCamPos = viewMatrix * deltaCamPos;

	cameraPosWorld += cbtk::cbmath::normalize(deltaCamPos) * dt * cameraMoveSpeed;
}

void updateCameraOrbit(float dt)
{
	// force camera to orbit around center of world
	cameraAzimuth += dt * cameraRotateSpeed;

	viewMatrix = cbtk::cbmath::makeRotationEuler4ZYX(0.0f, cameraAzimuth, 0.0f);

	cameraPosWorld.set(sinf(cameraAzimuth)*cameraDistance, 0.0f, cosf(cameraAzimuth)*cameraDistance, 1.0f);
}


//-----------------------------------------------------------------------------

// initialize game objects
// mainly for good memory management, handling ram and vram
int initGame()
{
	// setup framebuffers
	// don't bother with this here actually... see window resize callback
	//	setupFramebuffers();

	// setup geometry
	setupGeometry();

	// setup textures
	setupTextures();

	// setup shaders
	setupShaders();


	// other
	egpSetActiveMouse(mouse);
	egpSetActiveKeyboard(keybd);

	// done
	return 1;
}

// destroy game objects (mem)
int termGame()
{
	// good practice to do this in reverse order of creation
	//	in case something is referencing something else

	// delete fbos
	deleteFramebuffers();

	// delete shaders
	deleteShaders();

	// delete textures
	deleteTextures();

	// delete geometry
	deleteGeometry();

	// done
	return 1;
}


// output controls
void displayControls()
{
	printf("\n-------------------------------------------------------");
	printf("\n CONTROLS: \n");
	printf("\n wasdqe = camera movement");
	printf("\n ~` = display controls");
	printf("\n o = toggle slow-mo playback for all");
	printf("\n p = toggle play/pause for all");

	printf("\n l = real-time reload all shaders");
	printf("\n x = toggle coordinate axes post-draw");

	printf("\n 0-9 = toggle pipeline stage to be displayed");

	printf("\n-------------------------------------------------------");
}


// process input each frame
void handleInputState()
{
	// display controls
	if (egpKeyboardIsKeyPressed(keybd, '`') ||
		egpKeyboardIsKeyPressed(keybd, '~'))
		displayControls();

	// slowmo: play back at 25%
	if (egpKeyboardIsKeyPressed(keybd, 'o'))
		playrate = (playrate != 100) ? 100 : 25;

	// pause/play
	if (egpKeyboardIsKeyPressed(keybd, 'p'))
		playing = 1 - playing;


	// reload shaders
	if (egpKeyboardIsKeyPressed(keybd, 'l'))
	{
		deleteShaders();
		setupShaders();
	}

	// toggle axes
	if (egpKeyboardIsKeyPressed(keybd, 'x'))
		testDrawAxes = 1 - testDrawAxes;


	// toggle pipeline stage
	if (egpKeyboardIsKeyPressed(keybd, '0'))
	{
		displayMode = fboCount;
		fboFinalDisplay = fbo;
	}
	else
	{
		for (unsigned char c = '1', i = 0; c <= '9'; ++c, ++i)
			if (egpKeyboardIsKeyPressed(keybd, c))
			{
				displayMode = i;
				fboFinalDisplay = fbo + i;
			}
	}


	// finish by updating input state
	egpMouseUpdate(mouse);
	egpKeyboardUpdate(keybd);
}

// game update to perform each frame
// update game state using the time since the last update (dt) in seconds
void updateGameState(float dt)
{
	// update camera
	updateCameraControlled(dt, mouse);
//	updateCameraOrbit(dt);

	// update view matrix
	// 'c3' in a 4x4 matrix is the translation part
	viewMatrix.c3 = cameraPosWorld;
	viewMatrix = cbtk::cbmath::transformInverseNoScale(viewMatrix);
	viewProjMat = projectionMatrix * viewMatrix;


	// update game objects
	// scale time first
	dt *= (float)(playrate * playing) * 0.01f;

	// update objects here

	// SKYBOX TRANSFORM: 
	{
		// easy way: exact same as the camera's transform (view), but 
		//	we remove the translation (which makes the background seem 
		//	infinitely far away), and scale up instead

		// multiply view matrix with scale matrix to make modelview
		skyboxModelViewMatrix = viewMatrix * cbtk::cbmath::makeScale4(minClipDist);

		// remove translation component (by setting 4th column to 0,0,0,1)
		skyboxModelViewMatrix.c3 = cbmath::v4w;

		// concatenate with proj to get mvp
		skyboxModelViewProjectionMatrix = projectionMatrix * skyboxModelViewMatrix;
	}

	// earth: 
	{
		// animate daytime and orbit
		earthDaytime += dt * earthDaytimePeriod;
		earthOrbit += dt * earthOrbitPeriod;

		// calculate model matrix
		earthModelMatrix = cbmath::makeRotationZ4(earthTilt) * cbmath::makeRotationY4(earthDaytime);
		earthModelMatrix.c3.x = cosf(earthOrbit) * earthDistance;
		earthModelMatrix.c3.z = -sinf(earthOrbit) * earthDistance;

		// update mvp
		earthModelViewProjectionMatrix = viewProjMat * earthModelMatrix;

		// update inverse
		earthModelInverseMatrix = cbmath::transformInverseNoScale(earthModelMatrix);
	}

	// moon: 
	{
		moonOrbit += dt * moonOrbitPeriod;

		moonModelMatrix = cbmath::makeRotationZ4(moonTilt) * cbmath::makeRotationY4(moonOrbit) * cbmath::makeScale4(moonSize);
		moonModelMatrix.c3.x = earthModelMatrix.c3.x + cosf(moonOrbit) * moonDistance;
		moonModelMatrix.c3.z = earthModelMatrix.c3.z - sinf(moonOrbit) * moonDistance;
		moonModelViewProjectionMatrix = viewProjMat * moonModelMatrix;
		moonModelInverseMatrix = cbmath::transformInverseNoScale(moonModelMatrix);
	}
}


// skybox clear
void renderSkybox()
{
	currentProgramIndex = testTextureProgramIndex;
	currentProgram = glslPrograms + currentProgramIndex;
	currentUniformSet = glslCommonUniforms[currentProgramIndex];
	egpActivateProgram(currentProgram);

	// draw skybox instead of clearing
	glCullFace(GL_FRONT);
	glDepthFunc(GL_ALWAYS);
	glBindTexture(GL_TEXTURE_2D, tex[skyboxTexHandle]);

	egpSendUniformFloatMatrix(currentUniformSet[unif_mvp], UNIF_MAT4, 1, 0, skyboxModelViewProjectionMatrix.m);
	egpActivateVAO(vao + skyboxModel);
	egpDrawActiveVAO();

	glDepthFunc(GL_LESS);
	glCullFace(GL_BACK);
}

// draw scene objects
void renderSceneObjects()
{
	// draw textured moon
	{
		currentProgramIndex = testTextureProgramIndex;
		currentProgram = glslPrograms + currentProgramIndex;
		currentUniformSet = glslCommonUniforms[currentProgramIndex];
		egpActivateProgram(currentProgram);

		glBindTexture(GL_TEXTURE_2D, tex[moonTexHandle_dm]);

		// retained
		egpSendUniformFloatMatrix(currentUniformSet[unif_mvp], UNIF_MAT4, 1, 0, moonModelViewProjectionMatrix.m);
		egpActivateVAO(vao + sphere8x6Model);
		egpDrawActiveVAO();
	}

	// draw shaded earth
	{
		currentProgramIndex = celshadeProgramIndex;
		currentProgram = glslPrograms + currentProgramIndex;
		currentUniformSet = glslCommonUniforms[currentProgramIndex];
		egpActivateProgram(currentProgram);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex[earthTexHandle_dm]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, tex[celshadeRamp]);

		eyePos_object = earthModelInverseMatrix * cameraPosWorld;
		lightPos_object = earthModelInverseMatrix * lightPos_world;
		egpSendUniformFloat(currentUniformSet[unif_eyePos], UNIF_VEC4, 1, eyePos_object.v);
		egpSendUniformFloat(currentUniformSet[unif_lightPos], UNIF_VEC4, 1, lightPos_object.v);
		egpSendUniformFloatMatrix(currentUniformSet[unif_mvp], UNIF_MAT4, 1, 0, earthModelViewProjectionMatrix.m);
		egpActivateVAO(vao + sphereHiResObjModel);
		egpDrawActiveVAO();
	}
}


// draw frame
// DRAWING AND UPDATING SHOULD BE SEPARATE (good practice)
void renderGameState()
{
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
	// BLOOM ALGORITHM: 
	//	1. draw scene
	//	2. bright pass scene output
	//	3. blur bright pass output
	//		(repeat h/v, repeat for more bloom)
	//	4. composite

	cbmath::vec2 pixSzInv;
	int currentPipelineStage, lastPipelineStage;


	// first pass: scene
	// draw scene objects off-screen
	currentPipelineStage = sceneFBO;
	egpfwActivateFBO(fbo + currentPipelineStage);
	renderSkybox();
	renderSceneObjects();


	// only drawing full-screen now
	egpActivateVAO(vao + fsqModel);


	// second pass: bright pass/tone map the result of the scene pass
	// free box-blur occurs because the target is smaller than the screen
	currentProgramIndex = bloomBrightProgramIndex;
	currentProgram = glslPrograms + currentProgramIndex;
	egpActivateProgram(currentProgram);

	lastPipelineStage = currentPipelineStage;
	currentPipelineStage = brightFBO_d2;
	egpfwActivateFBO(fbo + currentPipelineStage);
	egpfwBindColorTargetTexture(fbo + lastPipelineStage, 0, 0);
	egpDrawActiveVAO();


	// third pass: Gaussian blur
	currentProgramIndex = bloomBlurProgramIndex;
	currentProgram = glslPrograms + currentProgramIndex;
	currentUniformSet = glslCommonUniforms[currentProgramIndex];
	egpActivateProgram(currentProgram);

	// horizontal
	lastPipelineStage = currentPipelineStage;
	currentPipelineStage = hblurFBO_d2;
	egpfwActivateFBO(fbo + currentPipelineStage);
	egpfwBindColorTargetTexture(fbo + lastPipelineStage, 0, 0);
	egpSendUniformFloat(currentUniformSet[unif_pixelSizeInv], UNIF_VEC2, 1, 
		pixSzInv.set(pixelSizeInv[lastPipelineStage].x, 0.0f).v);
	egpDrawActiveVAO();

	// repeat for vertical
	lastPipelineStage = currentPipelineStage;
	currentPipelineStage = vblurFBO_d2;
	egpfwActivateFBO(fbo + currentPipelineStage);
	egpfwBindColorTargetTexture(fbo + lastPipelineStage, 0, 0);
	egpSendUniformFloat(currentUniformSet[unif_pixelSizeInv], UNIF_VEC2, 1,
		pixSzInv.set(0.0f, pixelSizeInv[lastPipelineStage].x).v);
	egpDrawActiveVAO();

	// iterate on blurring for a more intense bloom effect
	{
		// repeat for horizontal
		lastPipelineStage = currentPipelineStage;
		currentPipelineStage = hblurFBO_d4;
		egpfwActivateFBO(fbo + currentPipelineStage);
		egpfwBindColorTargetTexture(fbo + lastPipelineStage, 0, 0);
		egpSendUniformFloat(currentUniformSet[unif_pixelSizeInv], UNIF_VEC2, 1,
			pixSzInv.set(pixelSizeInv[lastPipelineStage].x, 0.0f).v);
		egpDrawActiveVAO();

		// repeat for vertical
		lastPipelineStage = currentPipelineStage;
		currentPipelineStage = vblurFBO_d4;
		egpfwActivateFBO(fbo + currentPipelineStage);
		egpfwBindColorTargetTexture(fbo + lastPipelineStage, 0, 0);
		egpSendUniformFloat(currentUniformSet[unif_pixelSizeInv], UNIF_VEC2, 1,
			pixSzInv.set(0.0f, pixelSizeInv[lastPipelineStage].x).v);
		egpDrawActiveVAO();

		// ...moar bloom!
		{
			// repeat once more for horizontal
			lastPipelineStage = currentPipelineStage;
			currentPipelineStage = hblurFBO_d8;
			egpfwActivateFBO(fbo + currentPipelineStage);
			egpfwBindColorTargetTexture(fbo + lastPipelineStage, 0, 0);
			egpSendUniformFloat(currentUniformSet[unif_pixelSizeInv], UNIF_VEC2, 1,
				pixSzInv.set(pixelSizeInv[lastPipelineStage].x, 0.0f).v);
			egpDrawActiveVAO();

			// ditto for vertical
			lastPipelineStage = currentPipelineStage;
			currentPipelineStage = vblurFBO_d8;
			egpfwActivateFBO(fbo + currentPipelineStage);
			egpfwBindColorTargetTexture(fbo + lastPipelineStage, 0, 0);
			egpSendUniformFloat(currentUniformSet[unif_pixelSizeInv], UNIF_VEC2, 1,
				pixSzInv.set(0.0f, pixelSizeInv[lastPipelineStage].x).v);
			egpDrawActiveVAO();
		}
	}


	// fourth pass: composite
	currentProgramIndex = bloomBlendProgramIndex;
	currentProgram = glslPrograms + currentProgramIndex;
	currentUniformSet = glslCommonUniforms[currentProgramIndex];
	egpActivateProgram(currentProgram);

	currentPipelineStage = compositeFBO;
	egpfwActivateFBO(fbo + currentPipelineStage);
	egpfwBindColorTargetTexture(fbo + sceneFBO, 0, 0);
	egpfwBindColorTargetTexture(fbo + vblurFBO_d2, 1, 0);
	egpfwBindColorTargetTexture(fbo + vblurFBO_d4, 2, 0);
	egpfwBindColorTargetTexture(fbo + vblurFBO_d8, 3, 0);
	egpDrawActiveVAO();


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
	// FINAL DISPLAY: RENDER FINAL IMAGE ON FSQ TO BACK BUFFER

	// use back buffer
	egpfwActivateFBO(0);
	drawToBackBuffer(viewport_nb, viewport_nb, viewport_tw, viewport_th);

	// final pass: draw FSQ with user-selected FBO on it
	{
		// use texturing program, no mvp
		currentProgramIndex = testTexturePassthruProgramIndex;
		currentProgram = glslPrograms + currentProgramIndex;
		egpActivateProgram(currentProgram);

		// bind scene texture
		if (displayMode != fboCount)
			egpfwBindColorTargetTexture(fboFinalDisplay, 0, 0);
		else
			egpfwBindDepthTargetTexture(fboFinalDisplay, 0);
		egpDrawActiveVAO();

		// done with textures
		glBindTexture(GL_TEXTURE_2D, 0);

		// TEST DRAW: coordinate axes at center of spaces
		//	and other line objects
		if (testDrawAxes)
		{
			currentProgramIndex = testColorProgramIndex;
			currentProgram = glslPrograms + currentProgramIndex;
			currentUniformSet = glslCommonUniforms[currentProgramIndex];
			egpActivateProgram(currentProgram);
			egpSendUniformFloatMatrix(currentUniformSet[unif_mvp], UNIF_MAT4, 1, 0, viewProjMat.m);

			// center of world
			// (this is useful to see where the origin is and how big one unit is)
			egpActivateVAO(vao + axesModel);
			egpDrawActiveVAO();
		}

		// done
		glEnable(GL_DEPTH_TEST);
	}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

	// disable all renderables, shaders
	egpActivateProgram(0);
	egpActivateVAO(0);
}



//-----------------------------------------------------------------------------
// callbacks

// process everything
int idle()
{
	int ret = 0;
	// timer checks
	// pro tip: see what happens if you don't do the 'if'  ;)

	// trigger render if it is time
	if (egpTimerUpdate(renderTimer))
	{
		///////////////////////////////////////////////////////////////////////
		updateGameState(renderTimer->dt);
		handleInputState();
		renderGameState();
		///////////////////////////////////////////////////////////////////////
		ret = 1;
	}

//	if (egpTimerUpdate(secondTimer))
//	{
//		printf("\n %u frames rendered over %u seconds", renderTimer->ticks, secondTimer->ticks);
//	}

	return ret;
}

// window closed
void onCloseWindow()
{
	// clean up
	termGame();
}

// window resized
void onResizeWindow(int w, int h)
{
	// set new sizes
	win_w = w;
	win_h = h;
	win_aspect = ((float)w) / ((float)h);

	// calculate total viewport size
	viewport_tw = w + viewport_tb;
	viewport_th = h + viewport_tb;

	// update projection matrix
	projectionMatrix = cbtk::cbmath::makePerspective(fovy, win_aspect, znear, zfar);
	viewProjMat = projectionMatrix * viewMatrix;

	// update max clipping distance using pythagorean theorem 
	//	to far corner of frustum; the purpose of this is to 
	//	know how much space there is make the most of (skybox)
	{
		const float tf = zfar*tanf(fovy*0.5f);
		const float rf = tf*win_aspect;
		maxClipDist = (float)((unsigned int)(sqrtf(rf*rf + tf*tf + zfar*zfar)));
		minClipDist = (float)((unsigned int)(sqrtf(zfar*zfar / 3.0f)));
	}

	// if framebuffers are backed against the size of the main window, then 
	//	it's probably a good idea to tear down and remake the framebuffers...
	deleteFramebuffers();
	setupFramebuffers(viewport_tw, viewport_th);
}

// window moved
void onPositionWindow(int x, int y)
{
	win_x = x;
	win_y = y;
}
