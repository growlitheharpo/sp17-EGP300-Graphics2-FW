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

	// skinning mesh & skeleton drawing bone model
	skinningModel, 
	boneModel, 

//-----------------------------
	modelCount
};
enum IndexedModelIndex
{
	skinningIndexedModel, 
	boneIndexedModel, 
	
//-----------------------------
	indexedModelCount
};
egpVertexArrayObjectDescriptor vao[modelCount] = { 0 };
egpVertexBufferObjectDescriptor vbo[modelCount] = { 0 };
egpIndexBufferObjectDescriptor ibo[indexedModelCount] = { 0 };


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

	// skinning program
	skinningProgram, 
	drawBoneProgram, 

//-----------------------------
	GLSLProgramCount
};
enum GLSLCommonUniformIndex
{
	unif_mvp,
	unif_color, 
	unif_dm,

	// skinning & skeletal uniforms
	unif_bones_skinning, 
	unif_bones_world, 
	unif_boneLengths, 

//-----------------------------
	GLSLCommonUniformCount
};
const char *commonUniformName[] = {
	(const char *)("mvp"),
	(const char *)("color"),
	(const char *)("tex_dm"),

	// skeletal
	(const char *)("bones_skinning"),
	(const char *)("bones_world"),
	(const char *)("boneLengths"),
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
	skinningObject, 
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



// simple skeletal animation
#define BONES_MAX 64
struct SimpleBone
{
	int parentBoneIndex;			// -1 for root
	cbmath::mat4 transform_local;	// transformation relative to parent
};
struct SimpleSkeleton
{
	// assumes first bone is root
	unsigned int boneCount;
	SimpleBone bones[BONES_MAX];
	float boneLength[BONES_MAX];
	cbmath::mat4 transform_root[BONES_MAX];					// transformation relative to root/model
	cbmath::mat4 transform_root_bind[BONES_MAX];			// root transform at bind time (base pose)
	cbmath::mat4 transform_root_bind_inverse[BONES_MAX];	// bind pose inverse
	cbmath::mat4 transform_bindToCurr[BONES_MAX];			// skinning matrix: transform from bind pose to current pose
};

SimpleSkeleton testSkeleton[1];
float testSkeletonTime = 0.0f;


// ****
// forward kinematics update for all bones
void updateSkeletonForwardKinematics(SimpleSkeleton *skel)
{
	unsigned int i;

	// update root joint: no parent, so copy local transform to root transform


	// local to root = parent's local to root * local to parent
	for (i = 1; i < skel->boneCount; ++i)
	{

	}
}

// ****
// update skinning matrix for all bones
void updateSkeletonSkinningMatrices(SimpleSkeleton *skel)
{
	unsigned int i;

	// skinning matrix = local to root * inverse local to root bind
	for (i = 0; i < skel->boneCount; ++i)
	{

	}
}

// calculate bind pose
void calculateSkeletonBindPose(SimpleSkeleton *skel)
{
	unsigned int i;

	// do FK update, use resulting state as bind pose
	updateSkeletonForwardKinematics(skel);
	for (i = 0; i < skel->boneCount; ++i)
	{
		skel->transform_root_bind[i] = skel->transform_root[i];
		skel->transform_root_bind_inverse[i] = cbmath::transformInverseNoScale(skel->transform_root_bind[i]);
	}
}



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


	// ****
	// skeletal: skinning object is just a tessellated tube
	// skeleton will be drawn using a geometry shader again, single point converted into lines
	{
		// skinning mesh: just make a flat thingimagig for simplicity's sake
		{
			// raw mesh positions - unskinned ("bind pose")
			const unsigned int numSkinningVertices = 16;
			const float skinningVertices[numSkinningVertices * 3] = {
				// right pillar
				+0.5f,  0.0f,  0.0f, 
				+0.5f, +1.0f,  0.0f,
				+0.5f, +2.0f,  0.0f,
				+0.5f, +3.0f,  0.0f,

				// back pillar
				 0.0f,  0.0f, -0.5f,
				 0.0f, +1.0f, -0.5f,
				 0.0f, +2.0f, -0.5f,
				 0.0f, +3.0f, -0.5f,

				// left pillar
				-0.5f,  0.0f,  0.0f,
				-0.5f, +1.0f,  0.0f,
				-0.5f, +2.0f,  0.0f,
				-0.5f, +3.0f,  0.0f,

				// front pillar
				 0.0f,  0.0f, +0.5f,
				 0.0f, +1.0f, +0.5f,
				 0.0f, +2.0f, +0.5f,
				 0.0f, +3.0f, +0.5f,
			};

			// raw mesh normals - unskinned
			const float skinningNormals[numSkinningVertices * 3] = {
				+1.0f,  0.0f,  0.0f,
				+1.0f,  0.0f,  0.0f,
				+1.0f,  0.0f,  0.0f,
				+1.0f,  0.0f,  0.0f,

				 0.0f,  0.0f, -1.0f,
				 0.0f,  0.0f, -1.0f,
				 0.0f,  0.0f, -1.0f,
				 0.0f,  0.0f, -1.0f,
				 
				-1.0f,  0.0f,  0.0f,
				-1.0f,  0.0f,  0.0f,
				-1.0f,  0.0f,  0.0f,
				-1.0f,  0.0f,  0.0f,

				 0.0f,  0.0f, +1.0f,
				 0.0f,  0.0f, +1.0f,
				 0.0f,  0.0f, +1.0f,
				 0.0f,  0.0f, +1.0f,
			};

			// skinning weights (w): max 4 per vertex - each set adds up to 1
			const float skinningBlendWeights[numSkinningVertices * 4] = {
				1.00f, 0.00f, 0.00f, 0.00f,
				1.00f, 0.00f, 0.00f, 0.00f,
				1.00f, 0.00f, 0.00f, 0.00f,
				1.00f, 0.00f, 0.00f, 0.00f,

				1.00f, 0.00f, 0.00f, 0.00f,
				1.00f, 0.00f, 0.00f, 0.00f,
				1.00f, 0.00f, 0.00f, 0.00f,
				1.00f, 0.00f, 0.00f, 0.00f,

				1.00f, 0.00f, 0.00f, 0.00f,
				1.00f, 0.00f, 0.00f, 0.00f,
				1.00f, 0.00f, 0.00f, 0.00f,
				1.00f, 0.00f, 0.00f, 0.00f,

				1.00f, 0.00f, 0.00f, 0.00f,
				1.00f, 0.00f, 0.00f, 0.00f,
				1.00f, 0.00f, 0.00f, 0.00f,
				1.00f, 0.00f, 0.00f, 0.00f,
			};

			// skinning INFLUENCE indices (i): max 4 per vertex
			// corresponds to the bone that influences the vertex 
			//	by a factor of w (weight)
			// NOTE: they are only "floats" because that is the supported 
			//	GLSL attribute data type, but they will be used as integers!
			const float skinningBlendIndices[numSkinningVertices * 4] = {
				0, 0, 0, 0, 
				1, 0, 0, 0,
				2, 0, 0, 0,
				3, 0, 0, 0,

				0, 0, 0, 0,
				1, 0, 0, 0,
				2, 0, 0, 0,
				3, 0, 0, 0,

				0, 0, 0, 0,
				1, 0, 0, 0,
				2, 0, 0, 0,
				3, 0, 0, 0,

				0, 0, 0, 0,
				1, 0, 0, 0,
				2, 0, 0, 0,
				3, 0, 0, 0,
			};

			// indices for DRAWING a layered "tower-like" object: 
			// these are for the IBO, nothing to do with skinning
			// 3 verts per tri, 6 tris per facet * 4 facets, + 4 cap tris
			const unsigned int numSkinningIndices = 3 * (6 * 4 + 4);
			const unsigned int skinningIndices[numSkinningIndices] = {
				0, 4, 5, 5, 1, 0, 
				1, 5, 6, 6, 2, 1, 
				2, 6, 7, 7, 3, 2,

				4, 8, 9, 9, 5, 4,
				5, 9, 10, 10, 6, 5,
				6, 10, 11, 11, 7, 6,

				8, 12, 13, 13, 9, 8,
				9, 13, 14, 14, 10, 9,
				10, 14, 15, 15, 11, 10,

				12, 0, 1, 1, 13, 12,
				13, 1, 2, 2, 14, 13,
				14, 2, 3, 3, 15, 14,

				0, 12, 8, 8, 4, 0, 
				3, 7, 11, 11, 15, 3, 
			};


			// build object VAO & VBO
			egpAttributeDescriptor attribs_skinning[] = {
				egpCreateAttributeDescriptor(ATTRIB_POSITION, ATTRIB_VEC3, skinningVertices),
				egpCreateAttributeDescriptor(ATTRIB_NORMAL, ATTRIB_VEC3, skinningNormals),
				egpCreateAttributeDescriptor(ATTRIB_BLEND_WEIGHTS, ATTRIB_VEC4, skinningBlendWeights),
				egpCreateAttributeDescriptor(ATTRIB_BLEND_INDICES, ATTRIB_VEC4, skinningBlendIndices),
			};
			vao[skinningModel] = egpCreateVAOInterleavedIndexed(PRIM_TRIANGLES, 
				attribs_skinning, 4, numSkinningVertices, (vbo + skinningModel), 
				INDEX_UINT, numSkinningIndices, skinningIndices, (ibo + skinningIndexedModel));
		}

		// bone
		{
			// custom bone primitive: pyramid shape pointing along Z
			const float boneSz = 0.05f;
			const unsigned int numBoneVertices = 5;
			const float boneVertices[numBoneVertices * 3] = {
				+boneSz, 0.0f, 0.0f, 
				0.0f, +boneSz, 0.0f, 
				-boneSz, 0.0f, 0.0f, 
				0.0f, -boneSz, 0.0f, 
				0.0f, 0.0f, 1.0f, 
			};

			// indexed rendering vertices
			const unsigned int numBoneIndices = 10;
			const unsigned int boneIndices[numBoneIndices] = {
				0, 1, 2, 3, // base
				0, 4, 1, 2, 4, 3, // point
			};

			// create bone object
			egpAttributeDescriptor attribs_bone[] = {
				egpCreateAttributeDescriptor(ATTRIB_POSITION, ATTRIB_VEC3, boneVertices),
			};
			vao[boneModel] = egpCreateVAOInterleavedIndexed(PRIM_LINE_STRIP,
				attribs_bone, 1, numBoneVertices, (vbo + boneModel), 
				INDEX_UINT, numBoneIndices, boneIndices, (ibo + boneIndexedModel));
		}

		// setup skeleton bone poses
		{
			unsigned int parentIndex;
			testSkeleton->boneCount = 4;

			// root: rotate -90 on X so that Z points upwards
			testSkeleton->bones[0].parentBoneIndex = -1;
			testSkeleton->bones[0].transform_local = cbmath::makeRotationX4(Deg2Rad(-90.0f));
			testSkeleton->boneLength[0] = 1.0f;

			// subsequent bones: just translate on Z by parent's bone length
			//	and set new bone's length
			testSkeleton->bones[1].parentBoneIndex = parentIndex = 0;	// child of root node
			testSkeleton->bones[1].transform_local.m32 = testSkeleton->boneLength[parentIndex];
			testSkeleton->boneLength[1] = 1.0f;

			testSkeleton->bones[2].parentBoneIndex = parentIndex = 1;
			testSkeleton->bones[2].transform_local.m32 = testSkeleton->boneLength[parentIndex];
			testSkeleton->boneLength[2] = 1.0f;

			testSkeleton->bones[3].parentBoneIndex = parentIndex = 2;
			testSkeleton->bones[3].transform_local.m32 = testSkeleton->boneLength[parentIndex];
			testSkeleton->boneLength[3] = 0.0f;	// end of chain

			// set bind pose matrices
			calculateSkeletonBindPose(testSkeleton);
		}
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
	for (i = 0; i < indexedModelCount; ++i)
		egpReleaseIBO(ibo + i);
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

		// draw skinning object
		{
			files[0] = egpLoadFileContents("../../../../resource/glsl/4x/vs_animation/skinning_passNormalAsColor_vs4x.glsl");
			shaders[0] = egpCreateShaderFromSource(EGP_SHADER_VERTEX, files[0].contents);

			currentProgramIndex = skinningProgram;
			currentProgram = glslPrograms + currentProgramIndex;
			*currentProgram = egpCreateProgram();
			egpAttachShaderToProgram(currentProgram, shaders + 0);
			egpAttachShaderToProgram(currentProgram, shaders + 2);
			egpLinkProgram(currentProgram);
			egpValidateProgram(currentProgram);

			egpReleaseShader(shaders + 0);
			egpReleaseFileContents(files + 0);
		}

		// done with varying color fs
		egpReleaseShader(shaders + 2);
		egpReleaseFileContents(files + 2);


		// draw solid color (uniform)
		{
			files[1] = egpLoadFileContents("../../../../resource/glsl/4x/fs/drawSolid_fs4x.glsl");
			shaders[1] = egpCreateShaderFromSource(EGP_SHADER_FRAGMENT, files[1].contents);

			// basic draw solid color
			{
				files[0] = egpLoadFileContents("../../../../resource/glsl/4x/vs/transform_vs4x.glsl");
				shaders[0] = egpCreateShaderFromSource(EGP_SHADER_VERTEX, files[0].contents);

				currentProgramIndex = testSolidColorProgramIndex;
				currentProgram = glslPrograms + currentProgramIndex;
				*currentProgram = egpCreateProgram();
				egpAttachShaderToProgram(currentProgram, shaders + 0);
				egpAttachShaderToProgram(currentProgram, shaders + 1);
				egpLinkProgram(currentProgram);
				egpValidateProgram(currentProgram);

				egpReleaseShader(shaders + 0);
				egpReleaseFileContents(files + 0);
			}

			// bone with solid color
			{
				files[0] = egpLoadFileContents("../../../../resource/glsl/4x/vs_animation/drawBone_vs4x.glsl");
				shaders[0] = egpCreateShaderFromSource(EGP_SHADER_VERTEX, files[0].contents);

				currentProgramIndex = drawBoneProgram;
				currentProgram = glslPrograms + currentProgramIndex;
				*currentProgram = egpCreateProgram();
				egpAttachShaderToProgram(currentProgram, shaders + 0);
				egpAttachShaderToProgram(currentProgram, shaders + 1);
				egpLinkProgram(currentProgram);
				egpValidateProgram(currentProgram);

				egpReleaseShader(shaders + 0);
				egpReleaseFileContents(files + 0);
			}

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


		// skeletal animation
		{
			// procedural animation for debugging purposes: 
			// just make the bones do some snake-like movement
			unsigned int parentIndex;

			// time step and value used for animation
			float s;
			testSkeletonTime += dt;
			s = sinf(testSkeletonTime);

			// root: make it sway about X
			testSkeleton->bones[0].transform_local = cbmath::makeRotationX4(Deg2Rad(-90.0f) + s);

			// make each bone sway about some local axis/axes
			s *= 0.5f;
			parentIndex = testSkeleton->bones[1].parentBoneIndex;
			testSkeleton->bones[1].transform_local = cbmath::makeRotationEuler4XYZ(s, s, s);
			testSkeleton->bones[1].transform_local.m32 = testSkeleton->boneLength[parentIndex];

			s *= 0.5f;
			parentIndex = testSkeleton->bones[2].parentBoneIndex;
			testSkeleton->bones[2].transform_local = cbmath::makeRotationEuler4XYZ(s, s, s);
			testSkeleton->bones[2].transform_local.m32 = testSkeleton->boneLength[parentIndex];

			s *= 0.5f;
			parentIndex = testSkeleton->bones[3].parentBoneIndex;
			testSkeleton->bones[3].transform_local = cbmath::makeRotationEuler4XYZ(s, s, s);
			testSkeleton->bones[3].transform_local.m32 = testSkeleton->boneLength[parentIndex];

			// do forward kin and update skinning
			updateSkeletonForwardKinematics(testSkeleton);
			updateSkeletonSkinningMatrices(testSkeleton);
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
void renderSkinningObject()
{
	const cbmath::vec4 BLUE(0.0f, 0.5f, 1.0f, 1.0f), ORANGE(1.0f, 0.5f, 0.0f, 1.0f);
	cbmath::mat4 mvp = viewProjectionMatrix[cameraObjects + activeCamera] * modelMatrix[skinningObject];

	// draw skinning object
	currentProgramIndex = skinningProgram;
	currentProgram = glslPrograms + currentProgramIndex;
	currentUniformSet = glslCommonUniforms[currentProgramIndex];
	egpActivateProgram(currentProgram);

	// send MVP and bone matrices
	egpSendUniformFloatMatrix(currentUniformSet[unif_mvp], UNIF_MAT4, 1, 0, mvp.m);
	egpSendUniformFloatMatrix(currentUniformSet[unif_bones_skinning], UNIF_MAT4,
		testSkeleton->boneCount, 0, testSkeleton->transform_bindToCurr->m);

	// draw skinned object
	egpActivateVAO(vao + skinningModel);
	egpDrawActiveVAO();


	// draw skeleton as a series of bones
	currentProgramIndex = drawBoneProgram;
	currentProgram = glslPrograms + currentProgramIndex;
	currentUniformSet = glslCommonUniforms[currentProgramIndex];
	egpActivateProgram(currentProgram);

	// send MVP and bone info
	egpSendUniformFloatMatrix(currentUniformSet[unif_mvp], UNIF_MAT4, 1, 0, mvp.m);
	egpSendUniformFloatMatrix(currentUniformSet[unif_bones_world], UNIF_MAT4, 
		testSkeleton->boneCount, 0, testSkeleton->transform_root->m);
	egpSendUniformFloat(currentUniformSet[unif_boneLengths], UNIF_FLOAT,
		testSkeleton->boneCount, testSkeleton->boneLength);


	glDisable(GL_DEPTH_TEST);

	// draw bone model in blue
	egpSendUniformFloat(currentUniformSet[unif_color], UNIF_VEC4, 1, BLUE.v);
	egpActivateVAO(vao + boneModel);
	egpDrawActiveVAOInstanced(testSkeleton->boneCount);

	// draw coordinate axis for each bone in orange
	if (testDrawAxes)
	{
		egpSendUniformFloat(currentUniformSet[unif_color], UNIF_VEC4, 1, ORANGE.v);
		egpActivateVAO(vao + axesModel);
		egpDrawActiveVAOInstanced(testSkeleton->boneCount);
	}

	glEnable(GL_DEPTH_TEST);
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
	renderSkinningObject();


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
