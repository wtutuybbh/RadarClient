#pragma once
#include "ViewPortControl.h"


class CMinimap :
	public ViewPortControl
{
public:
	void Draw();
	CMinimap(LPCSTR className) : ViewPortControl(className) {		
	}
	~CMinimap();
	LRESULT ViewPortControlProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void ReshapeGL(int width, int height);
};

