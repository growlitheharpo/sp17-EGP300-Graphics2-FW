#pragma once
#include "egpfw/egpfw.h"
#include <vector>
#include <cbmath/cbtkVector.h>
#include <cbmath/cbtkMatrix.h>
struct egpMouse;
struct egpKeyboard;

class KeyframeWindow
{
	private:
		egpVertexArrayObjectDescriptor* mTargetVAO;
		std::vector<cbmath::vec4> waypoint;
		cbmath::mat4 mLittleBoxWindowMatrix;

	public:
		KeyframeWindow(egpVertexArrayObjectDescriptor* vao);
		~KeyframeWindow();

		void updateInput(egpMouse* m, egpKeyboard* key);
		void updateWindowSize(float viewport_tw, float viewport_th, float tmpNF, float win_w, float win_h);

		void render(egpFrameBufferObjectDescriptor* targetVBO, egpProgram* drawCurveProgram, int* uniformSet);
};

