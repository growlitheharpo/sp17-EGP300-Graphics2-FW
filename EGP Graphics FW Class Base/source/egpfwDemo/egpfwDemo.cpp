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
const unsigned int viewport_b = 0;
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

	// morph targets
	morphModel, 

//-----------------------------
	modelCount
};
egpVertexArrayObjectDescriptor vao[modelCount] = { 0 };
egpVertexBufferObjectDescriptor vbo[modelCount] = { 0 };


// loaded textures
enum TextureIndex
{
	skyboxTexHandle,

//-----------------------------
	textureCount
};
unsigned int tex[textureCount] = { 0 };


// GLSL program handle objects: 
// very convenient way to organize programs and their uniforms!
enum GLSLProgramIndex
{
	testColorProgramIndex,
	testSolidColorProgramIndex, 
	testTextureProgramIndex,
	testTexturePassthruProgramIndex,

	// morph targets programs
	morphTargetsProgram, 
	morphTargetsDrawCurveProgram, 

//-----------------------------
	GLSLProgramCount
};
enum GLSLCommonUniformIndex
{
	unif_mvp,
	unif_color, 
	unif_dm,

	// morph target uniforms
	unif_param, 
	unif_index, 

//-----------------------------
	GLSLCommonUniformCount
};
const char *commonUniformName[] = {
	(const char *)("mvp"),
	(const char *)("color"),
	(const char *)("tex_dm"),

	// morph targets
	(const char *)("param"),
	(const char *)("index"),
};
egpProgram glslPrograms[GLSLProgramCount] = { 0 }, *currentProgram = 0;
int glslCommonUniforms[GLSLProgramCount][GLSLCommonUniformCount] = { -1 }, *currentUniformSet = 0;
int currentProgramIndex = 0;


// framebuffer objects (FBOs)
enum FBOIndex
{
	// scene
	sceneFBO, 

//-----------------------------
	fboCount
};
enum DisplayMode
{
	displayScene, 
	displaySceneDepth, 
};
egpFrameBufferObjectDescriptor fbo[fboCount], *fboFinalDisplay = fbo;
int displayMode = displayScene;
int displayColor = 1;
int testDrawAxes = 0;
int testDrawCurves = 0;



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


// list of scene objects and respective matrices
enum SceneObjects
{
	skyboxObject,
	morphTargetObject, 
//-----------------------------
	sceneObjectCount,
//-----------------------------
	cameraObjects = sceneObjectCount,
//-----------------------------
	sceneObjectTotalCount = sceneObjectCount + numCameras
};

// transformation matrices
cbmath::mat4 modelMatrix[sceneObjectTotalCount];
cbmath::mat4 modelMatrixInv[sceneObjectTotalCount];
cbmath::mat4 viewProjectionMatrix[sceneObjectTotalCount];

cbmath::mat4 boxModelViewProjectionMatrix;



// morph targets
const int morphKeyframeCount = 4;
int morphKeyframeIndex[morphKeyframeCount] = { 0, 1, 2, 3 };
float morphInterpolationParam = 0.0f;
float morphInterpolationParamRate = 1.0f;



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
	attribs[0].data = egpfwGetUnitQuadPositions();
	attribs[1] = egpCreateAttributeDescriptor(ATTRIB_TEXCOORD, ATTRIB_VEC2, egpfwGetUnitQuadTexcoords());
	vao[fsqModel] = egpCreateVAOInterleaved(PRIM_TRIANGLE_STRIP, attribs, 2, 4, (vbo + fsqModel), 0);


	// morphing: test shape will just be a quad with 4 built-in keyframe poses
	{
		// we will have 4 positions; if you want normals you need to add more
		const unsigned int totalAttribs = morphKeyframeCount;
		const unsigned int numVertices = 4;

		// ****
		// positions for pose 0: default quad
		const float testMorphShapePosition0[numVertices * 3] = {
			-1.0f, -1.0f,  0.0f, 
			+1.0f, -1.0f,  0.0f, 
			-1.0f, +1.0f,  0.0f, 
			+1.0f, +1.0f,  0.0f, 
		};
		// positions for pose 1: move the bottom-left corner forward
		const float testMorphShapePosition1[numVertices * 3] = {
			-1.0f, -1.0f, +1.0f,
			+1.0f, -1.0f,  0.0f,
			-1.0f, +1.0f,  0.0f,
			+1.0f, +1.0f,  0.0f,
		};
		// positions for pose 2: move the top-right corner backward
		const float testMorphShapePosition2[numVertices * 3] = {
			-1.0f, -1.0f,  0.0f,
			+1.0f, -1.0f,  0.0f,
			-1.0f, +1.0f,  0.0f,
			+1.0f, +1.0f, -1.0f,
		};
		// positions for pose 3: stretch the bottom-right and top-left corners
		const float testMorphShapePosition3[numVertices * 3] = {
			-1.0f, -1.0f,  0.0f,
			+2.0f, -2.0f,  0.0f,
			-2.0f, +2.0f,  0.0f,
			+1.0f, +1.0f,  0.0f,
		};

		// create morph target object: 
		//	-> stuff all poses for position and other 
		//		morphable attributes into a single VBO!
		// demo case: let's just use the positions for now...
		const egpAttributeDescriptor morphTargetModelAttribs[] = {
			egpCreateAttributeDescriptor((egpAttributeName)0, ATTRIB_VEC3, testMorphShapePosition0),
			egpCreateAttributeDescriptor((egpAttributeName)1, ATTRIB_VEC3, testMorphShapePosition1),
			egpCreateAttributeDescriptor((egpAttributeName)2, ATTRIB_VEC3, testMorphShapePosition2),
			egpCreateAttributeDescriptor((egpAttributeName)3, ATTRIB_VEC3, testMorphShapePosition3),
		};
		vao[morphModel] = egpCreateVAOInterleaved(PRIM_TRIANGLE_STRIP, 
			morphTargetModelAttribs, totalAttribs, numVertices, (vbo + morphModel), 0);
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


	// color programs
	{
		// shared: varying color fragment shader
		files[2] = egpLoadFileContents("../../../../resource/glsl/4x/fs/drawColor_fs4x.glsl");
		shaders[2] = egpCreateShaderFromSource(EGP_SHADER_FRAGMENT, files[2].contents);

		// pass from attribute
		{
			files[0] = egpLoadFileContents("../../../../resource/glsl/4x/vs/passColor_vs4x.glsl");
			shaders[0] = egpCreateShaderFromSource(EGP_SHADER_VERTEX, files[0].contents);

			currentProgramIndex = testColorProgramIndex;
			currentProgram = glslPrograms + currentProgramIndex;
			*currentProgram = egpCreateProgram();
			egpAttachShaderToProgram(currentProgram, shaders + 0);
			egpAttachShaderToProgram(currentProgram, shaders + 2);
			egpLinkProgram(currentProgram);
			egpValidateProgram(currentProgram);

			egpReleaseShader(shaders + 0);
			egpReleaseFileContents(files + 0);
		}

		// draw morphing object
		{
			files[0] = egpLoadFileContents("../../../../resource/glsl/4x/vs_animation/morphing_vs4x.glsl");
			shaders[0] = egpCreateShaderFromSource(EGP_SHADER_VERTEX, files[0].contents);

			currentProgramIndex = morphTargetsProgram;
			currentProgram = glslPrograms + currentProgramIndex;
			*currentProgram = egpCreateProgram();
			egpAttachShaderToProgram(currentProgram, shaders + 0);
			egpAttachShaderToProgram(currentProgram, shaders + 2);
			egpLinkProgram(currentProgram);
			egpValidateProgram(currentProgram);

			egpReleaseShader(shaders + 0);
			egpReleaseFileContents(files + 0);
		}

		// draw morphing object curves
		{
			files[0] = egpLoadFileContents("../../../../resource/glsl/4x/vs_animation/morphing_passKeyframes_vs4x.glsl");
			shaders[0] = egpCreateShaderFromSource(EGP_SHADER_VERTEX, files[0].contents);
			files[1] = egpLoadFileContents("../../../../resource/glsl/4x/gs/drawCurve_morphing_gs4x.glsl");
			shaders[1] = egpCreateShaderFromSource(EGP_SHADER_GEOMETRY, files[1].contents);

			currentProgramIndex = morphTargetsDrawCurveProgram;
			currentProgram = glslPrograms + currentProgramIndex;
			*currentProgram = egpCreateProgram();
			egpAttachShaderToProgram(currentProgram, shaders + 0);
			egpAttachShaderToProgram(currentProgram, shaders + 1);
			egpAttachShaderToProgram(currentProgram, shaders + 2);
			egpLinkProgram(currentProgram);
			egpValidateProgram(currentProgram);

			egpReleaseShader(shaders + 0);
			egpReleaseFileContents(files + 0);
			egpReleaseShader(shaders + 1);
			egpReleaseFileContents(files + 1);
		}

		// done with varying color fs
		egpReleaseShader(shaders + 2);
		egpReleaseFileContents(files + 2);


		// draw solid color (uniform)
		{
			files[0] = egpLoadFileContents("../../../../resource/glsl/4x/vs/transform_vs4x.glsl");
			shaders[0] = egpCreateShaderFromSource(EGP_SHADER_VERTEX, files[0].contents);
			files[1] = egpLoadFileContents("../../../../resource/glsl/4x/fs/drawSolid_fs4x.glsl");
			shaders[1] = egpCreateShaderFromSource(EGP_SHADER_FRAGMENT, files[1].contents);

			currentProgramIndex = testSolidColorProgramIndex;
			currentProgram = glslPrograms + currentProgramIndex;
			*currentProgram = egpCreateProgram();
			egpAttachShaderToProgram(currentProgram, shaders + 0);
			egpAttachShaderToProgram(currentProgram, shaders + 1);
			egpLinkProgram(currentProgram);
			egpValidateProgram(currentProgram);

			egpReleaseShader(shaders + 0);
			egpReleaseFileContents(files + 0);
			egpReleaseShader(shaders + 1);
			egpReleaseFileContents(files + 1);
		}
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

	// scene
	fbo[sceneFBO] = egpfwCreateFBO(frameWidth, frameHeight, 1, colorFormat, DEPTH_D32, SMOOTH_NOWRAP);
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

	printf("\n c = draw morphing curves for vertices");
	printf("\n 1-4 = go to morph keyframe");

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

	
	// morphing curves
	if (egpKeyboardIsKeyPressed(keybd, 'c'))
		testDrawCurves = 1 - testDrawCurves;

	// hard-set morphing frame and pause
	if (egpKeyboardIsKeyPressed(keybd, '1'))
	{
		playing = 0;
		morphInterpolationParam = 0.0f;
		morphKeyframeIndex[0] = 0;
		morphKeyframeIndex[1] = 1;
		morphKeyframeIndex[2] = 2;
		morphKeyframeIndex[3] = 3;
	}
	else if (egpKeyboardIsKeyPressed(keybd, '2'))
	{
		playing = 0;
		morphInterpolationParam = 0.0f;
		morphKeyframeIndex[0] = 1;
		morphKeyframeIndex[1] = 2;
		morphKeyframeIndex[2] = 3;
		morphKeyframeIndex[3] = 0;
	}
	else if (egpKeyboardIsKeyPressed(keybd, '3'))
	{
		playing = 0;
		morphInterpolationParam = 0.0f;
		morphKeyframeIndex[0] = 2;
		morphKeyframeIndex[1] = 3;
		morphKeyframeIndex[2] = 0;
		morphKeyframeIndex[3] = 1;
	}
	else if (egpKeyboardIsKeyPressed(keybd, '4'))
	{
		playing = 0;
		morphInterpolationParam = 0.0f;
		morphKeyframeIndex[0] = 3;
		morphKeyframeIndex[1] = 0;
		morphKeyframeIndex[2] = 1;
		morphKeyframeIndex[3] = 2;
	}


/*
	// toggle pipeline stage
	if (egpKeyboardIsKeyPressed(keybd, '9'))
	{
		displayMode = displayScene;
		displayColor = 1;
		fboFinalDisplay = fbo + sceneFBO;
	}
	else if (egpKeyboardIsKeyPressed(keybd, '0'))
	{
		displayMode = displaySceneDepth;
		displayColor = 0;
		fboFinalDisplay = fbo + sceneFBO;
	}
*/


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
	{
		// skybox
		{
			// easy way: exact same as the camera's transform (view), but 
			//	we remove the translation (which makes the background seem 
			//	infinitely far away), and scale up instead

			// multiply view matrix with scale matrix to make modelview
			modelMatrix[skyboxObject] = cbmath::makeScaleTranslate(minClipDist, cameraPos_world[activeCamera].xyz);
			modelMatrixInv[skyboxObject] = cbmath::transformInverseUniformScale(modelMatrix[skyboxObject]);

			// concatenate with proj to get mvp
			boxModelViewProjectionMatrix = viewProjectionMatrix[controlCamera] * modelMatrix[skyboxObject];
		}

		// morphing object
		{
			// update interpolation parameter
			morphInterpolationParam += morphInterpolationParamRate*dt;
			if (morphInterpolationParam > 1.0f)
			{
				// next segment
				morphInterpolationParam -= 1.0f;

				// update keyframe indices (looping)
				for (int i = 0; i < morphKeyframeCount; ++i)
					morphKeyframeIndex[i] = (morphKeyframeIndex[i] + 1) % morphKeyframeCount;
			}
		}
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


// draw curves
void renderMorphObject()
{
	cbmath::mat4 mvp = viewProjectionMatrix[cameraObjects + activeCamera] * modelMatrix[morphTargetObject];
	egpActivateVAO(vao + morphModel);

	// draw morphing object
	currentProgramIndex = morphTargetsProgram;
	currentProgram = glslPrograms + currentProgramIndex;
	currentUniformSet = glslCommonUniforms[currentProgramIndex];
	egpActivateProgram(currentProgram);

	egpSendUniformFloatMatrix(currentUniformSet[unif_mvp], UNIF_MAT4, 1, 0, mvp.m);
	egpSendUniformFloat(currentUniformSet[unif_param], UNIF_FLOAT, 1, &morphInterpolationParam);
	egpSendUniformInt(currentUniformSet[unif_index], UNIF_INT, morphKeyframeCount, morphKeyframeIndex);
	egpDrawActiveVAO();

	// draw morphing object again with curve program
	if (testDrawCurves)
	{
		currentProgramIndex = morphTargetsDrawCurveProgram;
		currentProgram = glslPrograms + currentProgramIndex;
		currentUniformSet = glslCommonUniforms[currentProgramIndex];
		egpActivateProgram(currentProgram);

		egpSendUniformFloatMatrix(currentUniformSet[unif_mvp], UNIF_MAT4, 1, 0, mvp.m);
		egpDrawActiveVAO();
	}
}


// draw frame
// DRAWING AND UPDATING SHOULD BE SEPARATE (good practice)
void renderGameState()
{
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
	
	// draw skybox
	egpfwActivateFBO(fbo + sceneFBO);
	renderSkybox();
	renderMorphObject();


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

	//	// bind scene texture
	//	if (displayColor)
			egpfwBindColorTargetTexture(fboFinalDisplay, 0, 0);
	//	else
	//		egpfwBindDepthTargetTexture(fboFinalDisplay, 0);

		// draw fsq
		egpDrawActiveVAO();

		// TEST DRAW: coordinate axes at center of spaces
		//	and other line objects
		if (testDrawAxes)
		{
			currentProgramIndex = testColorProgramIndex;
			currentProgram = glslPrograms + currentProgramIndex;
			currentUniformSet = glslCommonUniforms[currentProgramIndex];
			egpActivateProgram(currentProgram);

			// axes
			egpActivateVAO(vao + axesModel);

			// center of world
			// (this is useful to see where the origin is and how big one unit is)
			egpSendUniformFloatMatrix(currentUniformSet[unif_mvp], UNIF_MAT4, 1, 0, viewProjectionMatrix[cameraObjects + activeCamera].m);

			// draw axes
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
