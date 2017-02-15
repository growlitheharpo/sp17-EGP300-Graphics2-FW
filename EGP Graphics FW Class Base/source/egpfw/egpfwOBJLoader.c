// By Dan Buckstein
// Modified by: James Keats with permission from author.
#include "egpfw/egpfw/egpfwOBJLoader.h"


#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>


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

struct interleave
{
	float3 pos, nor;
	float2 texcoord;
};

typedef struct interleave interleave;


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
		sscanf(lineBuffer,
			"%*s%i%*c%i%*c%i%i%*c%i%*c%i%i%*c%i%*c%i",
			&f.v0, &f.vt0, &f.vn0,
			&f.v1, &f.vt1, &f.vn1,
			&f.v2, &f.vt2, &f.vn2);

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
	void* faceStart = obj->data;
	void* faceEnd = BUFFER_OFFSET_BYTE(obj->data, obj->attribOffset[ATTRIB_POSITION]);

	float3* vertexStart = (float3*)((char *)(obj->data) + obj->attribOffset[ATTRIB_POSITION]);
	float3* normalStart = (float3*)((char *)(obj->data) + obj->attribOffset[ATTRIB_NORMAL]);
	float2* texcoordStart = (float2*)((char *)(obj->data) + obj->attribOffset[ATTRIB_TEXCOORD]);

	unsigned int numberOfFaces = (unsigned int)((char*)faceEnd - (char*)faceStart) / sizeof(face);

	unsigned int vertexCount = numberOfFaces * 3;
	unsigned int interleavedBufferSize = sizeof(interleave) * vertexCount;
	interleave* interleavedBuffer = (interleave*)malloc(interleavedBufferSize);

	face fdata;
	interleave idata;
	int i = 0;

	for (void* walker = faceStart; walker != faceEnd; walker = BUFFER_OFFSET_BYTE(walker, sizeof(face)))
	{
		fdata = *(face*)walker;
		
		idata.pos = vertexStart[fdata.v0];
		idata.nor = normalStart[fdata.vn0];
		idata.texcoord = texcoordStart[fdata.vt0];

		interleavedBuffer[i] = idata;
		i++;

		idata.pos = vertexStart[fdata.v1];
		idata.nor = normalStart[fdata.vn1];
		idata.texcoord = texcoordStart[fdata.vt1];

		interleavedBuffer[i] = idata;
		i++;

		idata.pos = vertexStart[fdata.v2];
		idata.nor = normalStart[fdata.vn2];
		idata.texcoord = texcoordStart[fdata.vt2];

		interleavedBuffer[i] = idata;
		i++;
	}

	//generate a VBO
	glGenBuffers(1, &vbo_out->glhandle);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_out->glhandle);
	glBufferData(GL_ARRAY_BUFFER, interleavedBufferSize, (void*)interleavedBuffer, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	vbo_out->attribTypes[ATTRIB_NORMAL] = 1;
	vbo_out->attribTypes[ATTRIB_TEXCOORD] = 1;
	vbo_out->attribTypes[ATTRIB_POSITION] = 1;
	vbo_out->vertexSize = sizeof(float3);
	vbo_out->vertexCount = vertexCount;

	glGenVertexArrays(1, &vao_out->glhandle);
	glBindVertexArray(vao_out->glhandle);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_out->glhandle);
	
	egpCreateVAOInterleaved(PRIM_TRIANGLES, NULL, 3, vertexCount, vbo_out, NULL);

	glEnableVertexAttribArray(ATTRIB_POSITION);
	glVertexAttribPointer(ATTRIB_POSITION,
		3, GL_FLOAT, GL_FALSE, sizeof(float3) + sizeof(float3) + sizeof(float2), (void*)0);
	
	glEnableVertexAttribArray(ATTRIB_NORMAL);
	glVertexAttribPointer(ATTRIB_NORMAL,
		3, GL_FLOAT, GL_FALSE, sizeof(float3) + sizeof(float3) + sizeof(float2), (void*)sizeof(float3));

	glEnableVertexAttribArray(ATTRIB_TEXCOORD);
	glVertexAttribPointer(ATTRIB_TEXCOORD,
		3, GL_FLOAT, GL_FALSE, sizeof(float3) + sizeof(float3) + sizeof(float2), (void*)(sizeof(float3) + sizeof(float3)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	vao_out->vbo = vbo_out;
	vao_out->primType = PRIM_TRIANGLES;
	vao_out->ibo = NULL;

	free(interleavedBuffer);

	return 0;
}


// ****
// free obj data
int egpfwReleaseOBJ(egpTriOBJDescriptor *obj)
{
	//...
	free(obj->data);
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
