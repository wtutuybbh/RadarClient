#pragma once
#include "stdafx.h"

/* File custom.h
* (custom control interface)
*/


class C3DObjectModel;

/* Window class */
#define VIEW_PORT_WC   _T("CViewPortControl")

enum ViewPortName
{
	Main,
	MiniMap
};

class CScene;
class CCamera;
class CUserInterface;
class old_C3DObject;

class CViewPortControl {
	static LRESULT CALLBACK stWinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool Register(void);
	void Init();
	static const std::string requestID;
	glm::mat4 pv;
protected:
	bool needs_resize{ false };
	int width, height;
	virtual void SetSize(int width, int height);
public:
	CScene *Scene{ nullptr };
	CCamera *Camera{ nullptr };
	CUserInterface *UI{ nullptr };
	HWND hWnd{ nullptr };
	HDC hDC{ nullptr };
	HGLRC hRC {nullptr};

	CViewPortControl(LPCWSTR className);
	virtual ~CViewPortControl();
	/* Register/unregister the window class */
	
	void Unregister(void);

	bool InitGL();

	virtual void ReshapeGL();
	

	virtual LRESULT ViewPortControlProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void Paint();

	virtual void Draw();

	void Add(HWND parent, int x, int y, int w, int h);

	void SetPosition(int x, int y, int w, int h);
	virtual glm::mat4 GetProjMatrix() const;
	virtual glm::mat4 GetViewMatrix() const;
	virtual void CalcPV();
	glm::mat4 GetPV() const;
	int DisplayPoints, DisplaySeries, DisplayRLIs, DisplayMap, DisplayLandscape, DisplayBlindZones;

	C3DObjectModel* Get3DObject(int x, int y);

	bool MakeCurrent() const;

	LPCWSTR ClassName;

	int GetHeight() const;
	int GetWidth() const;
	int X;
	int Y;	

	int Id;
};