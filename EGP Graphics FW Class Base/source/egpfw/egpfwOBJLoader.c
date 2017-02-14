// By Dan Buckstein
// Modified by: _______________________________________________________________
#include "egpfw/egpfw/egpfwOBJLoader.h"


#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#define BUFFER_OFFSET_BYTE(p,n) ((char *)(p) + n)


// helper structures
union float2
{
	struct { float f0, f1; };
	float f[2];
};
union float3
{
	struct { float f0, f1, f2; };
	float f[3];
};
union float4
{
	struct { float f0, f1, f2, f3; };
	float f[4];
};
union int4
{
	struct { int i0, i1, i2, i3; };
	int i[4];
};
union face
{
	struct { unsigned int v0, v1, v2, vt0, vt1, vt2, vn0, vn1, vn2; };
	struct { unsigned int v[3], vt[3], vn[3]; };
};

struct uniqueTriFace;
struct uniqueVertex;
struct uniqueTexcoord;
struct uniqueNormal;

#ifndef __cplusplus
typedef union float2			float2;
typedef union float3			float3;
typedef union float4			float4;
typedef union int4				int4;
typedef union face				face;
typedef struct uniqueTriFace	uniqueTriFace;
typedef struct uniqueVertex		uniqueVertex;
typedef struct uniqueTexcoord	uniqueTexcoord;
typedef struct uniqueNormal		uniqueNormal;
#endif	// __cplusplus


//-----------------------------------------------------------------------------


#define BUFFER_SIZE 2048

// ****
// load triangulated OBJ file
egpTriOBJDescriptor egpfwLoadTriangleOBJ(const char *objPath, const egpMeshNormalMode normalMode, const double globalScale)
{
	egpTriOBJDescriptor obj = { 0 };
	FILE* objFile = fopen(objPath, "r");
	if (objFile == NULL)
	{
		printf("Unable to open file %s.\n", objPath);
		return obj;
	}

	char lineBuffer[64];
	while (lineBuffer[0] != 'v' && fgets(lineBuffer, 64, objFile) != NULL) {}
	
	if (feof(objFile)) //reached end without finding a v
	{
		printf("Unable to open file %s.\n", objPath);
		return obj;
	}

	unsigned int numVerticies, numTexcoords, numNormals, numFaces;
	float3 vertexBuffer[BUFFER_SIZE]; //load a (relatively) large buffer for the verticies
	float2 vertexTexBuffer[BUFFER_SIZE]; //load a (relatively) large buffer for the verticies
	float3 vertexNorBuffer[BUFFER_SIZE]; //load a (relatively) large buffer for the verticies
	face faceBuffer[BUFFER_SIZE]; //load a (relatively) large buffer for the faces

	float3 currentF3;
	float2 currentF2;
	face f = { 0 };

	int currentIndex = 0;

	//Load all the verticies into vertexBuffer
	do 
	{
		sscanf(lineBuffer, "%*s %f %f %f", &currentF3.f0, &currentF3.f1, &currentF3.f2);
		vertexBuffer[currentIndex] = currentF3;
		currentIndex++;

		if (currentIndex >= BUFFER_SIZE)
		{
			printf("Number of verticies in obj overflowed buffer: %s", objPath);
			return obj;
		}

	} while (fgets(lineBuffer, 64, objFile) != NULL && lineBuffer[0] == 'v' && lineBuffer[1] == ' ');

	numVerticies = currentIndex;
	currentIndex = 0;

	//Load all the texcoords into vertexTexBuffer
	while (lineBuffer[0] == 'v' && lineBuffer[1] == 't')
	{
		sscanf(lineBuffer, "%*s %f %f", &currentF2.f0, &currentF2.f1);
		vertexTexBuffer[currentIndex] = currentF2;
		currentIndex++;

		if (currentIndex >= BUFFER_SIZE)
		{
			printf("Number of texcoords in obj overflowed buffer: %s", objPath);
			return obj;
		}
		
		if (fgets(lineBuffer, 64, objFile) == NULL)
			break;
	}

	numTexcoords = currentIndex;
	currentIndex = 0;
	
	//Load all the normals into vertexNorBuffer
	while (lineBuffer[0] == 'v' && lineBuffer[1] == 'n')
	{
		sscanf(lineBuffer, "%*s %f %f %f", &currentF3.f0, &currentF3.f1, &currentF3.f2);
		vertexNorBuffer[currentIndex] = currentF3;
		currentIndex++;

		if (currentIndex >= BUFFER_SIZE)
		{
			printf("Number of texcoords in obj overflowed buffer: %s", objPath);
			return obj;
		}

		if (fgets(lineBuffer, 64, objFile) == NULL)
			break;
	}

	numNormals = currentIndex;
	currentIndex = 0;

	//skip to the faces
	while (lineBuffer[0] != 'f' && fgets(lineBuffer, 64, objFile) != NULL) {}

	//Load all the faces into faceBuffer
	do
	{
		//format: f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
		//TODO: BREAKING ON THIS LINE, FIX IT
		sscanf(lineBuffer,
			"%*s %i %*c %i %*c %i %*s %i %*c %i %*c %i %*s %i %*c %i %*c %i",
			f.v0, f.vt0, f.vn0,
			f.v1, f.vt1, f.vn1,
			f.v2, f.vt2, f.vn2);

		faceBuffer[currentIndex] = f;
		currentIndex++;

		if (currentIndex >= BUFFER_SIZE)
		{
			printf("Number of faces in obj overflowed buffer: %s", objPath);
			return obj;
		}
		
	} while (fgets(lineBuffer, 64, objFile) != NULL && lineBuffer[0] == 'f' && lineBuffer[1] == ' ');

	numFaces = currentIndex;

	//Alright, we finally have all of our data. What's our total size?
	obj.dataSize = sizeof(float3) * numVerticies +
		sizeof(float2) * numTexcoords +
		sizeof(float3) * numNormals +
		sizeof(face) * numFaces;

	obj.attribOffset[ATTRIB_POSITION] = sizeof(face) * numFaces; //put the faces in first because we don't have an enum for them
	obj.attribOffset[ATTRIB_TEXCOORD] = obj.attribOffset[ATTRIB_POSITION] + sizeof(float3) * numVerticies;
	obj.attribOffset[ATTRIB_NORMAL] = obj.attribOffset[ATTRIB_TEXCOORD] + sizeof(float2) * numTexcoords;
	
	obj.data = (void*)malloc(obj.dataSize);

	//Okay, everything is set up. Now, stuff our data into the buffer.
	memcpy(obj.data, faceBuffer, sizeof(face) * numFaces);
	memcpy(BUFFER_OFFSET_BYTE(obj.data, obj.attribOffset[ATTRIB_POSITION]), vertexBuffer, sizeof(float3) * numVerticies);
	memcpy(BUFFER_OFFSET_BYTE(obj.data, obj.attribOffset[ATTRIB_TEXCOORD]), vertexTexBuffer, sizeof(float2) * numTexcoords);
	memcpy(BUFFER_OFFSET_BYTE(obj.data, obj.attribOffset[ATTRIB_NORMAL]), vertexNorBuffer, sizeof(float3) * numNormals);

	//printf("\nTrying to load an obj...\n");
	//...
	return obj;
}


// ****
// convert OBJ to VAO & VBO
int egpfwCreateVAOFromOBJ(const egpTriOBJDescriptor *obj, egpVertexArrayObjectDescriptor *vao_out, egpVertexBufferObjectDescriptor *vbo_out)
{
	//...
	//printf("Trying to create a VAO...\n");
	return 0;
}


// ****
// free obj data
int egpfwReleaseOBJ(egpTriOBJDescriptor *obj)
{
	//...
	return 0;
}


// ****
// save/load binary
int egpfwSaveBinaryOBJ(const egpTriOBJDescriptor *obj, const char *binPath)
{
	//...
	return 0;
}

egpTriOBJDescriptor egpfwLoadBinaryOBJ(const char *binPath)
{
	egpTriOBJDescriptor obj = { 0 };
	//...
	return obj;
}


// these functions are complete!
// get attribute data from OBJ
const void *egpfwGetOBJAttributeData(const egpTriOBJDescriptor *obj, const egpAttributeName attrib)
{
	if (obj && obj->data && obj->attribOffset[attrib])
		return BUFFER_OFFSET_BYTE(obj->data, obj->attribOffset[attrib]);
	return 0;
}

unsigned int egpfwGetOBJNumVertices(const egpTriOBJDescriptor *obj)
{
	if (obj && obj->data)
		return *((unsigned int *)obj->data);
	return 0;
}
