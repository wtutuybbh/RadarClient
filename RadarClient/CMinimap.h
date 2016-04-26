#pragma once
#include "ViewPortControl.h"
#include "Util.h"

class CMinimap :
	public ViewPortControl
{
private:
	bool CameraDrag;
	bool IsCameraHere(int x, int y);
	glm::vec2 DragStart;
public:
	void Draw();
	CMinimap(LPCSTR className) : ViewPortControl(className) {	
		CameraDrag = false;
	}
	~CMinimap();
	LRESULT ViewPortControlProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void ReshapeGL(int width, int height);
#ifdef _DEBUG
	DebugWindowInfo *dwi;
#endif
};

