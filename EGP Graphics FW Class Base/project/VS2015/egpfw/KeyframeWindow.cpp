#include "KeyframeWindow.h"
#include <egpfw/egpfw/utils/egpfwInputUtils.h>
#include <iostream>
#include <cbmath/cbtkMatrix.h>
#include <GL/glew.h>
#include "transformMatrix.h"

const std::array<cbmath::vec4, KeyframeWindow::NUM_OF_CHANNELS> COLORS = 
{
	cbmath::vec4(1.0f, 0.0f, 0.0f, 1.0f),
	cbmath::vec4(0.0f, 1.0f, 0.0f, 1.0f),
	cbmath::vec4(0.0f, 0.0f, 1.0f, 1.0f),
	cbmath::vec4(1.0f, 0.0f, 0.0f, 1.0f),
	cbmath::vec4(0.0f, 1.0f, 0.0f, 1.0f),
	cbmath::vec4(0.0f, 0.0f, 1.0f, 1.0f),
};

KeyframeWindow::KeyframeWindow(egpVertexArrayObjectDescriptor* vao, egpFrameBufferObjectDescriptor* fbo, egpProgram* programs)
{
	mCurrentTime = 0.0f;
	mVAOList = vao;
	mFBOList = fbo;
	mProgramList = programs;
	mCurrentChannel = CHANNEL_POS_X;
}

KeyframeWindow::~KeyframeWindow()
{

}

bool KeyframeWindow::updateInput(egpMouse* m, egpKeyboard* key)
{
	for (unsigned char c = '1', i = 0; i < NUM_OF_CHANNELS; ++c, ++i)
	{
		if (egpKeyboardIsKeyPressed(key, c))
			mCurrentChannel = static_cast<KeyframeChannel>(i);
	}

	if (egpKeyboardIsKeyPressed(key, 'y'))
	{
		for (auto& waypointList : mWaypointChannels)
			waypointList.clear();
	}

	cbmath::vec4 mousePos(egpMouseX(m), mWindowSize.y - egpMouseY(m), 0.0f, 1.0f);
	mousePos = mOnScreenMatrixInv * mousePos;

	if (mousePos.x / mWindowSize.x > 1.0f || mousePos.y / mWindowSize.y > 1.0f)
		return false;

	if (egpMouseIsButtonPressed(m, 0))
	{
		size_t insertIndex = 0;
		for (insertIndex = 0; insertIndex < mWaypointChannels[mCurrentChannel].size(); insertIndex++)
		{
			if (mWaypointChannels[mCurrentChannel][insertIndex].x > mousePos.x)
				break;
		}

		mWaypointChannels[mCurrentChannel].insert(mWaypointChannels[mCurrentChannel].begin() + insertIndex, mousePos);
	}

	return true;
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
	mOnScreenMatrix = cbmath::makeTranslation4(-0.595f, -0.59f, 0.0f) * cbmath::makeScale4(0.4f);

	if (!gluInvertMatrix(mOnScreenMatrix.m, mOnScreenMatrixInv.m))
		throw std::invalid_argument("I have no idea how this is possible, but our on-screen transformation matrix could not be inverted!");
}

float KeyframeWindow::getValAtCurrentTime(KeyframeChannel c)
{
	auto& list = mWaypointChannels[c];

	if (list.size() == 0)
		return 0.0f;

	return list[0].y;
}

void KeyframeWindow::renderToFBO(int* curveUniformSet, int* solidColorUniformSet)
{
	//const cbmath::vec4 waypointColor(1.0, 0.5f, 0.0f, 1.0f);
	const cbmath::vec4 objectColor(1.0f, 1.0f, 0.5f, 1.0f);

	int j;
	cbmath::vec4 *waypointPtr;
	cbmath::mat4 waypointMVP;

	egpfwActivateFBO(mFBOList + curvesFBO);

	// clear
	glClear(GL_COLOR_BUFFER_BIT);

	int zeroTest = 0;
	int trueTest = 1;
	int vecSize;

	for (size_t i = 0; i < mWaypointChannels.size(); ++i)
	{
		// draw curve
		egpActivateProgram(mProgramList + drawCurveProgram);
		egpSendUniformFloatMatrix(curveUniformSet[unif_mvp], UNIF_MAT4, 1, 0, mLittleBoxWindowMatrix.m);

		vecSize = mWaypointChannels[i].size();

		// ship waypoint data to program, where it will be received by GS
		egpSendUniformFloat(curveUniformSet[unif_waypoint], UNIF_VEC4, vecSize, mWaypointChannels[i].data()->v);
		egpSendUniformInt(curveUniformSet[unif_waypointCount], UNIF_INT, 1, &vecSize);
		egpSendUniformInt(curveUniformSet[unif_curveMode], UNIF_INT, 1, &zeroTest);
		egpSendUniformInt(curveUniformSet[unif_useWaypoints], UNIF_INT, 1, &trueTest);

		egpActivateVAO(mVAOList + pointModel);
		egpDrawActiveVAO();

		// draw waypoints using solid color program and sphere model
		cbmath::mat4 waypointModelMatrix = cbmath::makeScale4(4.0f);
		egpActivateProgram(mProgramList + testSolidColorProgramIndex);
		egpSendUniformFloat(solidColorUniformSet[unif_color], UNIF_VEC4, 1, COLORS[i].v);

		egpActivateVAO(mVAOList + sphere8x6Model);

		// draw waypoints
		for (j = 0, waypointPtr = mWaypointChannels[i].data(); j < mWaypointChannels[i].size(); ++j, ++waypointPtr)
		{
			// set position, update MVP for this waypoint and draw
			waypointModelMatrix.c3 = *waypointPtr;
			waypointMVP = mLittleBoxWindowMatrix * waypointModelMatrix;
			egpSendUniformFloatMatrix(solidColorUniformSet[unif_mvp], UNIF_MAT4, 1, 0, waypointMVP.m);
			egpDrawActiveVAO();
		}
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
