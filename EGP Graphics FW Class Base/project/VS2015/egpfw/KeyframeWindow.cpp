#include "KeyframeWindow.h"
#include <egpfw/egpfw/utils/egpfwInputUtils.h>
#include <iostream>
#include <cbmath/cbtkMatrix.h>
#include <GL/glew.h>


KeyframeWindow::KeyframeWindow(egpVertexArrayObjectDescriptor* vao)
{
	mTargetVAO = vao;
}

KeyframeWindow::~KeyframeWindow()
{

}

void KeyframeWindow::updateInput(egpMouse* m, egpKeyboard* key)
{
	if (egpMouseIsButtonPressed(m, 0))
		std::cout << "left click!" << std::endl;
}

void KeyframeWindow::updateWindowSize(float viewport_tw, float viewport_th, float tmpNF, float win_w, float win_h)
{
	mLittleBoxWindowMatrix = cbmath::m4Identity;
	mLittleBoxWindowMatrix.m00 = 2.0f / viewport_tw;
	mLittleBoxWindowMatrix.m11 = 2.0f / viewport_th;
	mLittleBoxWindowMatrix.m22 = -1.0f / tmpNF;
	mLittleBoxWindowMatrix.m30 = -win_w / viewport_tw;
	mLittleBoxWindowMatrix.m31 = -win_h / viewport_th;
}

void KeyframeWindow::render(egpFrameBufferObjectDescriptor* targetVBO, egpProgram* drawCurveProgram, int* uniformSet)
{
	egpfwActivateFBO(targetVBO);

	const cbmath::vec4 waypointColor(1.0, 0.5f, 0.0f, 1.0f);
	const cbmath::vec4 objectColor(1.0f, 1.0f, 0.5f, 1.0f);

	int i;
	cbmath::vec4 *waypointPtr;
	cbmath::mat4 waypointMVP;

	// clear
	glClear(GL_COLOR_BUFFER_BIT);

	// draw curve
	/*currentProgramIndex = drawCurveProgram;
	currentProgram = glslPrograms + currentProgramIndex;
	currentUniformSet = glslCommonUniforms[currentProgramIndex];*/
	egpActivateProgram(drawCurveProgram);
	egpSendUniformFloatMatrix(uniformSet[unif_mvp], UNIF_MAT4, 1, 0, mLittleBoxWindowMatrix.m);

	static int zeroTest = 0;
	static int trueTest = 1;
	static int vecSize = waypoint.size();

	// ship waypoint data to program, where it will be received by GS
	egpSendUniformFloat(uniformSet[unif_waypoint], UNIF_VEC4, vecSize, waypoint.data()->v);
	egpSendUniformInt(uniformSet[unif_waypointCount], UNIF_INT, 1, &vecSize);
	egpSendUniformInt(uniformSet[unif_curveMode], UNIF_INT, 1, &zeroTest);
	egpSendUniformInt(uniformSet[unif_useWaypoints], UNIF_INT, 1, &trueTest);

	egpActivateVAO(mTargetVAO);
	egpDrawActiveVAO();

	/*
	// draw waypoints using solid color program and sphere model
	currentProgramIndex = testSolidColorProgramIndex;
	currentProgram = glslPrograms + currentProgramIndex;
	currentUniformSet = glslCommonUniforms[currentProgramIndex];
	egpActivateProgram(currentProgram);
	egpSendUniformFloat(currentUniformSet[unif_color], UNIF_VEC4, 1, waypointColor.v);

	egpActivateVAO(vao + sphere8x6Model);

	// draw waypoints
	for (i = 0, waypointPtr = waypoint; i < waypointCount; ++i, ++waypointPtr)
	{
		// set position, update MVP for this waypoint and draw
		waypointModelMatrix.c3 = *waypointPtr;
		waypointMVP = curveDrawingProjectionMatrix * waypointModelMatrix;
		egpSendUniformFloatMatrix(currentUniformSet[unif_mvp], UNIF_MAT4, 1, 0, waypointMVP.m);
		egpDrawActiveVAO();
	}*/
}
