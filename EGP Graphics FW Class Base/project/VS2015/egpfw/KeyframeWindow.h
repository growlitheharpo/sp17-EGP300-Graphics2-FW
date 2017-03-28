#pragma once
#include "egpfw/egpfw.h"
#include <vector>
#include <cbmath/cbtkVector.h>
#include <cbmath/cbtkMatrix.h>
struct egpMouse;
struct egpKeyboard;

class KeyframeWindow
{
	public:
		enum KeyframeChannel
		{
			CHANNEL_POS_X,
			CHANNEL_POS_Y,
			CHANNEL_POS_Z,
			CHANNEL_ROT_X,
			CHANNEL_ROT_Y,
			CHANNEL_ROT_Z,
		};

	private:
		egpVertexArrayObjectDescriptor* mVAOList;
		std::vector<cbmath::vec4> mWaypoints;
		cbmath::vec2 mWindowSize;
		cbmath::mat4 mLittleBoxWindowMatrix;
		float mCurrentTime;

	public:
		KeyframeWindow(egpVertexArrayObjectDescriptor* vao);
		~KeyframeWindow();

		void updateInput(egpMouse* m, egpKeyboard* key);
		void updateWindowSize(float viewport_tw, float viewport_th, float tmpNF, float win_w, float win_h);

		float getValAtCurrentTime(KeyframeChannel c);

		void render(egpFrameBufferObjectDescriptor* targetFBO, egpProgram* drawCurveProgram, int* uniformSet);
};

