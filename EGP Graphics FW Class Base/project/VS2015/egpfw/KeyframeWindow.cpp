#include "KeyframeWindow.h"
#include <egpfw/egpfw/utils/egpfwInputUtils.h>
#include <iostream>
#include <cbmath/cbtkMatrix.h>
#include <GL/glew.h>
#include "transformMatrix.h"


KeyframeWindow::KeyframeWindow(egpVertexArrayObjectDescriptor* vao, egpFrameBufferObjectDescriptor* fbo, egpProgram* programs)
{
	mVAOList = vao;
	mFBOList = fbo;
	mProgramList = programs;
}

KeyframeWindow::~KeyframeWindow()
{

}

void KeyframeWindow::updateInput(egpMouse* m, egpKeyboard* key)
{
	if (egpMouseIsButtonPressed(m, 0))
	{
		mWaypoints.push_back(cbmath::vec4((float)egpMouseX(m), mWindowSize.y - (float)egpMouseY(m), 0.0f, 1.0f));
	}
}

void KeyframeWindow::updateWindowSize(float viewport_tw, float viewport_th, float tmpNF, float win_w, float win_h)
{
	mLittleBoxWindowMatrix = cbmath::m4Identity;
	mLittleBoxWindowMatrix.m00 = 2.0f / viewport_tw;
	mLittleBoxWindowMatrix.m11 = 2.0f / viewport_th;
	mLittleBoxWindowMatrix.m22 = -1.0f / tmpNF;
	mLittleBoxWindowMatrix.m30 = -win_w / viewport_tw;
	mLittleBoxWindowMatrix.m31 = -win_h / viewport_th;

	mWindowSize.set(win_w, win_h);
	mOnScreenMatrix = cbmath::makeScale4(0.45f);

	if (!gluInvertMatrix(mOnScreenMatrix.m, mOnScreenMatrixInv.m))
		throw std::invalid_argument("I have no idea how this is possible, but our on-screen transformation matrix could not be inverted!");
}

float KeyframeWindow::getValAtCurrentTime(KeyframeChannel c)
{
	return -1.0f; //Fill this in later
}

void KeyframeWindow::renderToFBO(int* curveUniformSet,  int* solidColorUniformSet)
{
	const cbmath::vec4 waypointColor(1.0, 0.5f, 0.0f, 1.0f);
	const cbmath::vec4 objectColor(1.0f, 1.0f, 0.5f, 1.0f);

	int i;
	cbmath::vec4 *waypointPtr;
	cbmath::mat4 waypointMVP;

	egpfwActivateFBO(mFBOList + curvesFBO);

	// clear
	glClear(GL_COLOR_BUFFER_BIT);

	// draw curve
	egpActivateProgram(mProgramList + drawCurveProgram);
	egpSendUniformFloatMatrix(curveUniformSet[unif_mvp], UNIF_MAT4, 1, 0, mLittleBoxWindowMatrix.m);

	int zeroTest = 0;
	int trueTest = 1;
	int vecSize = mWaypoints.size();

	// ship waypoint data to program, where it will be received by GS
	egpSendUniformFloat(curveUniformSet[unif_waypoint], UNIF_VEC4, vecSize, mWaypoints.data()->v);
	egpSendUniformInt(curveUniformSet[unif_waypointCount], UNIF_INT, 1, &vecSize);
	egpSendUniformInt(curveUniformSet[unif_curveMode], UNIF_INT, 1, &zeroTest);
	egpSendUniformInt(curveUniformSet[unif_useWaypoints], UNIF_INT, 1, &trueTest);

	egpActivateVAO(mVAOList + pointModel);
	egpDrawActiveVAO();

	// draw waypoints using solid color program and sphere model
	cbmath::mat4 waypointModelMatrix = cbmath::makeScale4(4.0f);
	egpActivateProgram(mProgramList + testSolidColorProgramIndex);
	egpSendUniformFloat(solidColorUniformSet[unif_color], UNIF_VEC4, 1, waypointColor.v);

	egpActivateVAO(mVAOList + sphere8x6Model);

	// draw waypoints
	for (i = 0, waypointPtr = mWaypoints.data(); i < mWaypoints.size(); ++i, ++waypointPtr)
	{
		// set position, update MVP for this waypoint and draw
		waypointModelMatrix.c3 = *waypointPtr;
		waypointMVP = mLittleBoxWindowMatrix * waypointModelMatrix;
		egpSendUniformFloatMatrix(solidColorUniformSet[unif_mvp], UNIF_MAT4, 1, 0, waypointMVP.m);
		egpDrawActiveVAO();
	}
}

void KeyframeWindow::renderToBackbuffer(int* textureUniformSet)
{
	egpActivateProgram(mProgramList + testTextureProgramIndex);
	egpActivateVAO(mVAOList + fsqModel);
	egpfwBindColorTargetTexture(mFBOList + curvesFBO, 0, 0);
	egpSendUniformFloatMatrix(textureUniformSet[unif_mvp], UNIF_MAT4, 1, 0, mOnScreenMatrix.m);
	egpDrawActiveVAO();

}
