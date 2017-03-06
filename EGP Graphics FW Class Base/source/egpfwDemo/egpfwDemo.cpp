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
	boxModel, sphere8x6Model, sphere32x24Model,

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
	booleanTexHandle, 

	atlas_diffuse,
	atlas_specular,

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
	testTransformProgramIndex, 

	// shadow mapping and projective texturing
	projectiveTextureProgram, 
	shadowMapProgram, 

//-----------------------------
	GLSLProgramCount
};
enum GLSLCommonUniformIndex
{
	unif_mvp,

	unif_lightColor,
	unif_lightPos,
	unif_eyePos,

	unif_atlasMat,
	unif_normalScale,

	unif_dm,
	unif_sm,
	unif_bool, 

	// shadow mapping
	unif_mvp_projector, 
	unif_projtex, 
	unif_shadowmap, 

//-----------------------------
	GLSLCommonUniformCount
};
const char *commonUniformName[] = {
	(const char *)("mvp"),
	(const char *)("lightColor"),
	(const char *)("lightPos"),
	(const char *)("eyePos"),
	(const char *)("atlasMat"),
	(const char *)("normalScale"),
	(const char *)("tex_dm"),
	(const char *)("tex_sm"),
	(const char *)("tex_bool"),
	(const char *)("mvp_projector"),
	(const char *)("projtex"),
	(const char *)("shadowmap"),
};
egpProgram glslPrograms[GLSLProgramCount] = { 0 }, *currentProgram = 0;
int glslCommonUniforms[GLSLProgramCount][GLSLCommonUniformCount] = { -1 }, *currentUniformSet = 0;
int currentProgramIndex = 0;


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
enum DisplayMode
{
	displayScene, 
	displaySceneDepth, 
	displaySceneProjTex, 
	displaySceneShadowMap, 
	displayShadowMap, 
};
egpFrameBufferObjectDescriptor fbo[fboCount], *fboFinalDisplay = fbo;
int displayMode = displayScene;
int displayColor = 1;
int testDrawAxes = 0;



//-----------------------------------------------------------------------------
// our game objects

// cameras
const unsigned int numCameras = 1, activeCamera = 0;
cbmath::mat4 projectionMatrix[numCameras];
// camera controls
float cameraElevation = 0.0f, cameraAzimuth = 0.0f;
float cameraRotateSpeed = 0.25f, cameraMoveSpeed = 4.0f, cameraDistance = 8.0f;
cbmath::vec4 cameraPos_world[numCameras] = {
	cbmath::vec4(0.0f, 0.0f, cameraDistance, 1.0f)
}, cameraPos_obj[numCameras], deltaCamPos;


// light positions and colors
// for the colors, let the xyz components represent color in rgb, and 
//	the w component represent the radius of the light volume
const unsigned int numLights = 1;
cbmath::vec4 lightPos_world[numLights] = {
	cbmath::vec4(10.0f, 20.0f, 5.0f, 1.0f),
}, lightPos_obj[numLights];
cbmath::vec4 lightColor[numLights] = {
	cbmath::vec4(1.0f, 1.0f, 1.0f, 100.0f),
};

// bias matrix for using light as viewer
const cbmath::mat4 bias(0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f);

// projection matrix for light as a viewer
const float lightNearZ = 10.0f, lightFarZ = 100.0f;
const cbmath::mat4 lightProjMat = cbmath::makePerspective(-lightNearZ, lightNearZ, -lightNearZ, lightNearZ, lightNearZ, lightFarZ);


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



// list of scene objects and respective matrices
enum SceneObjects
{
	skyboxObject,
	groundObject,
	moonObject,
	marsObject,
	earthObject,
//-----------------------------
	sceneObjectCount,
//-----------------------------
	cameraObjects = sceneObjectCount,
	lightObjects = cameraObjects + numCameras,
//-----------------------------
	sceneObjectTotalCount = lightObjects + numLights
};

// transformation matrices
cbmath::mat4 modelMatrix[sceneObjectTotalCount];
cbmath::mat4 modelMatrixInv[sceneObjectTotalCount];
cbmath::mat4 viewProjectionMatrix[sceneObjectTotalCount];
cbmath::mat4 atlasMatrix[sceneObjectCount];

cbmath::mat4 boxModelViewProjectionMatrix;



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
	vao[boxModel] = egpCreateVAOInterleaved(PRIM_TRIANGLES, attribs, 4, egpGetCubeVertexCount(), (vbo + boxModel), 0);

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

	// full-screen quad
	{
		const float tmpNormalData[] = {
			0.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f,
		};

		attribs[0].data = egpfwGetUnitQuadPositions();
		attribs[1].data = egpfwGetUnitQuadColors();
		attribs[2].data = tmpNormalData;
		attribs[3].data = egpfwGetUnitQuadTexcoords();
		vao[fsqModel] = egpCreateVAOInterleaved(PRIM_TRIANGLE_STRIP, attribs, 4, 4, (vbo + fsqModel), 0);
	}


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


	// geometry-related constants
	modelMatrix[groundObject] = cbmath::makeTranslation4(0.0f, -5.0f, 0.0f) * cbmath::makeScale4(10.0f, 0.1f, 10.0f);
	modelMatrixInv[groundObject] = cbmath::transformInverse(modelMatrix[groundObject]);

	atlasMatrix[skyboxObject] = atlasMatrix[groundObject] = cbmath::makeScale4(0.125f, 0.125f, 0.0f);
	atlasMatrix[earthObject] = atlasMatrix[moonObject] = atlasMatrix[marsObject] = cbmath::makeScale4(0.5f, 0.25f, 0.0f);
	atlasMatrix[earthObject].c3.y = 0.75f;
	atlasMatrix[moonObject].c3.y = 0.5f;
	atlasMatrix[marsObject].c3.y = 0.25f;
	atlasMatrix[groundObject].c3.y = 0.125f;

	// lights
	for (unsigned int i = 0, j = lightObjects; i < numLights; ++i, ++j)
	{
		modelMatrix[j] = cbmath::makeFrenet4(lightPos_world[i].xyz, cbmath::v3zero, cbmath::v3y);
		modelMatrixInv[j] = cbmath::transformInverseNoScale(modelMatrix[j]);
		viewProjectionMatrix[j] = lightProjMat * modelMatrixInv[j];
	}
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
		(char *)("../../../../resource/tex/logic/bool.png"),
		(char *)("../../../../resource/tex/atlas/atlas_diffuse.png"),
		(char *)("../../../../resource/tex/atlas/atlas_specular.png"),
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

	// boolean ramp
	glBindTexture(GL_TEXTURE_2D, tex[booleanTexHandle]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	// atlas textures
	glBindTexture(GL_TEXTURE_2D, tex[atlas_diffuse]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	// pixelated for demonstration purposes
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, tex[atlas_specular]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


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

			// done with passthru vs
			egpReleaseShader(shaders + 0);
			egpReleaseFileContents(files + 0);
		}

		// release shared
		egpReleaseShader(shaders + 1);
		egpReleaseFileContents(files + 1);
	}

	// transform, no FS
	{
		files[0] = egpLoadFileContents("../../../../resource/glsl/4x/vs/transform_vs4x.glsl");
		shaders[0] = egpCreateShaderFromSource(EGP_SHADER_VERTEX, files[0].contents);

		currentProgramIndex = testTransformProgramIndex;
		currentProgram = glslPrograms + currentProgramIndex;

		*currentProgram = egpCreateProgram();
		egpAttachShaderToProgram(currentProgram, shaders + 0);
		egpLinkProgram(currentProgram);
		egpValidateProgram(currentProgram);

		egpReleaseShader(shaders + 0);
		egpReleaseFileContents(files + 0);
	}

	// shadow mapping and projective texturing
	{
		// load shared shader
		files[0] = egpLoadFileContents("../../../../resource/glsl/4x/vs_shadow/phong_passAltClip_vs4x.glsl");
		shaders[0] = egpCreateShaderFromSource(EGP_SHADER_VERTEX, files[0].contents);

		// projective
		{
			files[1] = egpLoadFileContents("../../../../resource/glsl/4x/fs_shadow/phong_projtex_fs4x.glsl");
			shaders[1] = egpCreateShaderFromSource(EGP_SHADER_FRAGMENT, files[1].contents);

			currentProgramIndex = projectiveTextureProgram;
			currentProgram = glslPrograms + currentProgramIndex;

			*currentProgram = egpCreateProgram();
			egpAttachShaderToProgram(currentProgram, shaders + 0);
			egpAttachShaderToProgram(currentProgram, shaders + 1);
			egpLinkProgram(currentProgram);
			egpValidateProgram(currentProgram);

			egpReleaseShader(shaders + 1);
			egpReleaseFileContents(files + 1);
		}

		// shadow
		{
			files[1] = egpLoadFileContents("../../../../resource/glsl/4x/fs_shadow/phong_shadowmap_fs4x.glsl");
			shaders[1] = egpCreateShaderFromSource(EGP_SHADER_FRAGMENT, files[1].contents);

			currentProgramIndex = shadowMapProgram;
			currentProgram = glslPrograms + currentProgramIndex;

			*currentProgram = egpCreateProgram();
			egpAttachShaderToProgram(currentProgram, shaders + 0);
			egpAttachShaderToProgram(currentProgram, shaders + 1);
			egpLinkProgram(currentProgram);
			egpValidateProgram(currentProgram);

			egpReleaseShader(shaders + 1);
			egpReleaseFileContents(files + 1);
		}

		// release shared
		egpReleaseShader(shaders + 0);
		egpReleaseFileContents(files + 0);
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
		egpSendUniformInt(currentUniformSet[unif_bool], UNIF_INT, 1, imageLocations + 2);
		egpSendUniformInt(currentUniformSet[unif_shadowmap], UNIF_INT, 1, imageLocations + 3);
		egpSendUniformInt(currentUniformSet[unif_projtex], UNIF_INT, 1, imageLocations + 4);
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
	const egpColorFormat colorFormat = COLOR_RGBA16;
	const unsigned int shadowMapSize = 2048;

	// one for the scene
	fbo[sceneFBO] = egpfwCreateFBO(frameWidth, frameHeight, 1, colorFormat, DEPTH_D32, SMOOTH_NOWRAP);

	// one for the shadow map, depth only!
	fbo[shadowFBO] = egpfwCreateFBO(shadowMapSize, shadowMapSize, 0, COLOR_DISABLE, DEPTH_D32, WRAP_DISABLE);
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
void updateCameraControlled(float dt, unsigned int controlCamera, unsigned int controlCameraIndex, egpMouse *mouse)
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

	modelMatrix[controlCamera] = cbtk::cbmath::makeRotationEuler4ZYX(cameraElevation, cameraAzimuth, 0.0f);

	// apply current rotation to our movement vector so that "forward" is the direction the camera is facing
	deltaCamPos.set(
		(float)egpKeyboardDifference(keybd, 'd', 'a'),
		(float)egpKeyboardDifference(keybd, 'e', 'q'),
		(float)egpKeyboardDifference(keybd, 's', 'w'),
		0.0f);
	deltaCamPos = modelMatrix[controlCamera] * deltaCamPos;

	cameraPos_world[controlCameraIndex] += cbtk::cbmath::normalize(deltaCamPos) * dt * cameraMoveSpeed;
}

void updateCameraOrbit(float dt, unsigned int controlCamera, unsigned int controlCameraIndex)
{
	// force camera to orbit around center of world
	cameraAzimuth += dt * cameraRotateSpeed;

	modelMatrix[controlCamera] = cbtk::cbmath::makeRotationEuler4ZYX(0.0f, cameraAzimuth, 0.0f);

	cameraPos_world[controlCameraIndex].set(sinf(cameraAzimuth)*cameraDistance, 0.0f, cosf(cameraAzimuth)*cameraDistance, 1.0f);
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

	printf("\n 1-5 = toggle pipeline stage to be displayed");

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
	if (egpKeyboardIsKeyPressed(keybd, '1'))
	{
		displayMode = displayScene;
		displayColor = 1;
		fboFinalDisplay = fbo + sceneFBO;
	}
	else if (egpKeyboardIsKeyPressed(keybd, '2'))
	{
		displayMode = displaySceneDepth;
		displayColor = 0;
		fboFinalDisplay = fbo + sceneFBO;
	}
	else if (egpKeyboardIsKeyPressed(keybd, '3'))
	{
		displayMode = displaySceneProjTex;
		displayColor = 1;
		fboFinalDisplay = fbo + sceneFBO;
	}
	else if (egpKeyboardIsKeyPressed(keybd, '4'))
	{
		displayMode = displaySceneShadowMap;
		displayColor = 1;
		fboFinalDisplay = fbo + sceneFBO;
	}
	else if (egpKeyboardIsKeyPressed(keybd, '5'))
	{
		displayMode = displayShadowMap;
		displayColor = 0;
		fboFinalDisplay = fbo + shadowFBO;
	}


	// finish by updating input state
	egpMouseUpdate(mouse);
	egpKeyboardUpdate(keybd);
}

// game update to perform each frame
// update game state using the time since the last update (dt) in seconds
void updateGameState(float dt)
{
	const unsigned int controlCamera = cameraObjects + activeCamera;

	// update camera
	updateCameraControlled(dt, controlCamera, activeCamera, mouse);
//	updateCameraOrbit(dt, controlCamera);

	// update view matrix
	// 'c3' in a 4x4 matrix is the translation part
	modelMatrix[controlCamera].c3 = cameraPos_world[activeCamera];
	modelMatrixInv[controlCamera] = cbtk::cbmath::transformInverseNoScale(modelMatrix[controlCamera]);
	viewProjectionMatrix[controlCamera] = projectionMatrix[activeCamera] * modelMatrixInv[controlCamera];


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
		modelMatrix[skyboxObject] = cbmath::makeScaleTranslate(minClipDist, cameraPos_world[activeCamera].xyz);
		modelMatrixInv[skyboxObject] = cbmath::transformInverseUniformScale(modelMatrix[skyboxObject]);

		// concatenate with proj to get mvp
		boxModelViewProjectionMatrix = modelMatrixInv[controlCamera] * modelMatrix[skyboxObject];
	}

	// earth: 
	{
		// animate daytime and orbit
		earthDaytime += dt * earthDaytimePeriod;
		earthOrbit += dt * earthOrbitPeriod;

		// calculate model matrix
		modelMatrix[earthObject] = cbmath::makeRotationZ4(earthTilt) * cbmath::makeRotationY4(earthDaytime);
		modelMatrix[earthObject].c3.x = cosf(earthOrbit) * earthDistance;
		modelMatrix[earthObject].c3.z = -sinf(earthOrbit) * earthDistance;
		modelMatrixInv[earthObject] = cbmath::transformInverseNoScale(modelMatrix[earthObject]);
	}

	// moon: 
	{
		moonOrbit += dt * moonOrbitPeriod;

		modelMatrix[moonObject] = cbmath::makeRotationZ4(moonTilt) * cbmath::makeRotationY4(moonOrbit) * cbmath::makeScale4(moonSize);
		modelMatrix[moonObject].c3.x = modelMatrix[earthObject].c3.x + cosf(moonOrbit) * moonDistance;
		modelMatrix[moonObject].c3.z = modelMatrix[earthObject].c3.z - sinf(moonOrbit) * moonDistance;
		modelMatrixInv[moonObject] = cbmath::transformInverseNoScale(modelMatrix[moonObject]);
	}

	// mars: 
	{
		modelMatrix[marsObject] = modelMatrix[moonObject];
		modelMatrix[marsObject].c3.y += 2.0f;
		modelMatrixInv[marsObject] = cbmath::transformInverseNoScale(modelMatrix[marsObject]);
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

	egpSendUniformFloatMatrix(currentUniformSet[unif_mvp], UNIF_MAT4, 1, 0, boxModelViewProjectionMatrix.m);
	egpActivateVAO(vao + boxModel);
	egpDrawActiveVAO();

	glDepthFunc(GL_LESS);
	glCullFace(GL_BACK);
}


// update and draw scene objects
void updateObjectMVPs(const cbmath::mat4 modelMat, const cbmath::mat4 biasMat, 
	const cbmath::mat4 *vp, cbmath::mat4 *mvp_out, const unsigned int numMatrices)
{
	unsigned int i;
	for (i = 0; i < numMatrices; ++i, ++vp, ++mvp_out)
		*mvp_out = biasMat * (*vp * modelMat);
}

void updateObjectSpacePoint(const cbmath::mat4 modelMatInv,
	const cbmath::vec4 *point_world, cbmath::vec4 *point_obj, const unsigned int numPoints)
{
	unsigned int i;
	for (i = 0; i < numPoints; ++i, ++point_world, ++point_obj)
		*point_obj = modelMatInv * *point_world;
}

void updateSendLightsCamera(const cbmath::mat4 modelInv)
{
	// light position(s) in object space
	updateObjectSpacePoint(modelInv, lightPos_world, lightPos_obj, numLights);
	egpSendUniformFloat(currentUniformSet[unif_lightPos], UNIF_VEC4, numLights, lightPos_obj->v);

	// camera position(s) in object space
	updateObjectSpacePoint(modelInv, cameraPos_world + activeCamera, cameraPos_obj + activeCamera, 1);
	egpSendUniformFloat(currentUniformSet[unif_eyePos], UNIF_VEC4, 1, cameraPos_obj[activeCamera].v);
}

void sendObjectMatrices(const cbmath::mat4 *mvp_main, const cbmath::mat4 *mvp_other, const cbmath::mat4 *atlas,
	const unsigned int numOtherMatrices, int mvpMainLocation, int mvpOtherLocation, int atlasLocation)
{
	egpSendUniformFloatMatrix(currentUniformSet[mvpMainLocation], UNIF_MAT4, 1, 0, mvp_main->m);
	if (mvp_other && numOtherMatrices)
		egpSendUniformFloatMatrix(currentUniformSet[mvpOtherLocation], UNIF_MAT4, numOtherMatrices, 0, mvp_other->m);
	if (atlas)
		egpSendUniformFloatMatrix(currentUniformSet[atlasLocation], UNIF_MAT4, 1, 0, atlas->m);
}

void renderSceneObjects(const cbmath::mat4 *mvp_main, const cbmath::mat4 *mvp_other, const cbmath::mat4 *atlas,
	const unsigned int numOtherMatrices, const unsigned int numSceneObjects, int lighting, int cullBack)
{
	// light colors for all
	if (lighting)
		egpSendUniformFloat(currentUniformSet[unif_lightColor], UNIF_VEC4, numLights, lightColor->v);

	// background
	{
		// normal scaling adjustment values
		const float normalScale[2] = { -1.0f, +1.0f };

		glCullFace(GL_FRONT);
		glDepthFunc(GL_ALWAYS);
		egpSendUniformFloat(currentUniformSet[unif_normalScale], UNIF_FLOAT, 1, normalScale);	// invert

		if (lighting)
			updateSendLightsCamera(modelMatrixInv[skyboxObject]);
		sendObjectMatrices(mvp_main, mvp_other, (atlas ? atlas++ : 0),
			numOtherMatrices, unif_mvp, unif_mvp_projector, unif_atlasMat);
		egpActivateVAO(vao + boxModel);
		egpDrawActiveVAO();

		egpSendUniformFloat(currentUniformSet[unif_normalScale], UNIF_FLOAT, 1, normalScale + 1);
		glDepthFunc(GL_LESS);

		// leave culling inverted for shadows
		if (cullBack)
			glCullFace(GL_BACK);
	}


	// ground
	{
		if (lighting)
			updateSendLightsCamera(modelMatrixInv[groundObject]);
		sendObjectMatrices(mvp_main += numSceneObjects, mvp_other += numSceneObjects, (atlas ? atlas++ : 0),
			numOtherMatrices, unif_mvp, unif_mvp_projector, unif_atlasMat);
		egpDrawActiveVAO();
	}

	// moon
	{
		if (lighting)
			updateSendLightsCamera(modelMatrixInv[moonObject]);
		sendObjectMatrices(mvp_main += numSceneObjects, mvp_other += numSceneObjects, (atlas ? atlas++ : 0),
			numOtherMatrices, unif_mvp, unif_mvp_projector, unif_atlasMat);
	//	egpActivateVAO(vao + sphereLowResObjModel);
		egpActivateVAO(vao + sphere8x6Model);
		egpDrawActiveVAO();
	}

	// mars
	{
		if (lighting)
			updateSendLightsCamera(modelMatrixInv[marsObject]);
		sendObjectMatrices(mvp_main += numSceneObjects, mvp_other += numSceneObjects, (atlas ? atlas++ : 0),
			numOtherMatrices, unif_mvp, unif_mvp_projector, unif_atlasMat);
		egpDrawActiveVAO();
	}

	// earth
	{
		if (lighting)
			updateSendLightsCamera(modelMatrixInv[earthObject]);
		sendObjectMatrices(mvp_main += numSceneObjects, mvp_other += numSceneObjects, (atlas ? atlas++ : 0),
			numOtherMatrices, unif_mvp, unif_mvp_projector, unif_atlasMat);
		//	egpActivateVAO(vao + sphereHiResObjModel);
		egpActivateVAO(vao + sphere32x24Model);
		egpDrawActiveVAO();
	}


	// done
	glCullFace(GL_BACK);
}


// draw frame
// DRAWING AND UPDATING SHOULD BE SEPARATE (good practice)
void renderGameState()
{
	const unsigned int controlCamera = cameraObjects + activeCamera;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
	// PROJECTIVE TEXTURING ALGORITHM: 
	//	1. draw scene with main and alternative clipping pipeline (mvp)
	//		-> perform shadow test in frag shader, apply texture

	// DEFERRED LIGHTING ALGORITHM: 
	//	1. draw scene to depth-only target from light's point of view
	//	2. draw scene with main and alternative clipping pipeline (mvp)
	//		-> perform shadow test in frag shader, apply shadow


	int currentPipelineStage, lastPipelineStage;

	// each object has one matrix per camera and per light
	const unsigned int numMVPsPerObject = (numCameras + numLights);
	const unsigned int numMVPs = numMVPsPerObject * sceneObjectCount;
	cbmath::mat4 mvp[numMVPs];
	cbmath::mat4 mvpb[numMVPs];

	// update all MVP matrices once
	{
		cbmath::mat4 *mvpPtr = mvp, *mvpbPtr = mvpb;
		const cbmath::mat4 *modelMatPtr = modelMatrix, *viewprojMatPtr = viewProjectionMatrix + cameraObjects;
		unsigned int i;
		for (i = 0; i < sceneObjectCount; ++i, ++modelMatPtr, mvpPtr += numMVPsPerObject, mvpbPtr += numMVPsPerObject)
		{
			updateObjectMVPs(*modelMatPtr, cbmath::m4Identity, viewprojMatPtr, mvpPtr, numMVPsPerObject);
			updateObjectMVPs(*modelMatPtr, bias, viewprojMatPtr, mvpbPtr, numMVPsPerObject);
		}
	}


	// bind atlases
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, tex[booleanTexHandle]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex[atlas_specular]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex[atlas_diffuse]);


	switch (displayMode)
	{
	case displayScene: 
	case displaySceneDepth:
		// draw scene, just output colors
		currentPipelineStage = sceneFBO;
		egpfwActivateFBO(fbo + currentPipelineStage);
		currentProgramIndex = testColorProgramIndex;
		currentProgram = glslPrograms + currentProgramIndex;
		currentUniformSet = glslCommonUniforms[currentProgramIndex];
		egpActivateProgram(currentProgram);
		renderSceneObjects(mvp, 0, 0, 0, numMVPsPerObject, 0, 1);
		break;
	case displaySceneProjTex: 
		// draw scene with projective texturing
		currentPipelineStage = sceneFBO;
		egpfwActivateFBO(fbo + currentPipelineStage);
		currentProgramIndex = projectiveTextureProgram;
		currentProgram = glslPrograms + currentProgramIndex;
		currentUniformSet = glslCommonUniforms[currentProgramIndex];
		egpActivateProgram(currentProgram);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, tex[skyboxTexHandle]);	// using texture to project
		renderSceneObjects(mvp, mvpb + numCameras, atlasMatrix, numLights, numMVPsPerObject, 1, 1);
		break;
	case displaySceneShadowMap: 
	case displayShadowMap: 
		// draw scene to shadow map target
		currentPipelineStage = shadowFBO;
		egpfwActivateFBO(fbo + currentPipelineStage);
		currentProgramIndex = testTransformProgramIndex;
		currentProgram = glslPrograms + currentProgramIndex;
		currentUniformSet = glslCommonUniforms[currentProgramIndex];
		egpActivateProgram(currentProgram);
		renderSceneObjects(mvp + numCameras, 0, 0, 0, numMVPsPerObject, 0, 0);	// reverse culling

		// draw scene with shadow mapping
		lastPipelineStage = currentPipelineStage;
		currentPipelineStage = sceneFBO;
		egpfwActivateFBO(fbo + currentPipelineStage);
		currentProgramIndex = shadowMapProgram;
		currentProgram = glslPrograms + currentProgramIndex;
		currentUniformSet = glslCommonUniforms[currentProgramIndex];
		egpActivateProgram(currentProgram);
		egpfwBindDepthTargetTexture(fbo + lastPipelineStage, 3);	// using shadow map
		renderSceneObjects(mvp, mvpb + numCameras, atlasMatrix, numLights, numMVPsPerObject, 1, 1);
		break;
	}


/*	
	// DEBUGGING: add object to final output
	{
		currentPipelineStage = sceneFBO;
		egpfwActivateFBO(fbo + currentPipelineStage);
		currentProgramIndex = testColorProgramIndex;
		currentProgram = glslPrograms + currentProgramIndex;
		currentUniformSet = glslCommonUniforms[currentProgramIndex];
		egpActivateProgram(currentProgram);

		egpSendUniformFloatMatrix(currentUniformSet[unif_mvp], UNIF_MAT4, 1, 0, viewProjectionMatrix[cameraObjects].m);
		egpActivateVAO(vao + sphere32x24Model);
		egpDrawActiveVAO();
	}
*/


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
	// FINAL DISPLAY: RENDER FINAL IMAGE ON FSQ TO BACK BUFFER
	{
		// use back buffer
		egpfwActivateFBO(0);
		drawToBackBuffer(viewport_nb, viewport_nb, viewport_tw, viewport_th);

		// draw FSQ with user-selected FBO on it
		egpActivateVAO(vao + fsqModel);

		// use texturing program, no mvp
		currentProgramIndex = testTexturePassthruProgramIndex;
		currentProgram = glslPrograms + currentProgramIndex;
		egpActivateProgram(currentProgram);

		// bind scene texture
		if (displayColor)
			egpfwBindColorTargetTexture(fboFinalDisplay, 0, 0);
		else
			egpfwBindDepthTargetTexture(fboFinalDisplay, 0);
		egpDrawActiveVAO();

		// TEST DRAW: coordinate axes at center of spaces
		//	and other line objects
		if (testDrawAxes && displayMode != displayShadowMap)
		{
			cbmath::mat4 t = viewProjectionMatrix[controlCamera];

			egpActivateVAO(vao + axesModel);
			currentProgramIndex = testColorProgramIndex;
			currentProgram = glslPrograms + currentProgramIndex;
			currentUniformSet = glslCommonUniforms[currentProgramIndex];
			egpActivateProgram(currentProgram);

			// center of world
			// (this is useful to see where the origin is and how big one unit is)
			egpSendUniformFloatMatrix(currentUniformSet[unif_mvp], UNIF_MAT4, 1, 0, t.m);
			egpDrawActiveVAO();

			// light source
			t = t * modelMatrix[lightObjects];
			egpSendUniformFloatMatrix(currentUniformSet[unif_mvp], UNIF_MAT4, 1, 0, t.m);
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

	// calculate total viewport size
	viewport_tw = w + viewport_tb;
	viewport_th = h + viewport_tb;
	win_aspect = ((float)viewport_tw) / ((float)viewport_th);

	// update projection matrix
	projectionMatrix[activeCamera] = cbtk::cbmath::makePerspective(fovy, win_aspect, znear, zfar);
	viewProjectionMatrix[cameraObjects + activeCamera] = projectionMatrix[activeCamera] * modelMatrixInv[cameraObjects + activeCamera];

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
