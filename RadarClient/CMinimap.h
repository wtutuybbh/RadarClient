#pragma once
#include "CViewPortControl.h"
#include "Util.h"

class CMinimap :
	public CViewPortControl
{
	bool CameraDrag;
	bool IsCameraHere(int x, int y) const;
	glm::vec2 DragStart;
public:
	void Draw();
	CMinimap(LPCWSTR className) : CViewPortControl(className)
	{	
		CameraDrag = false;
	}
	~CMinimap();
	LRESULT ViewPortControlProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	void ReshapeGL() override;
	glm::mat4 GetProjMatrix() const override;
	glm::mat4 GetViewMatrix() const override;
#ifdef _DEBUG
	DebugWindowInfo *dwi{ nullptr };
#endif
};

