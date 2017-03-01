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
public:
	CScene *Scene{ nullptr };
	CCamera *Camera{ nullptr };
	CUserInterface *UI{ nullptr };
	HWND hWnd;
	HDC hDC;
	HGLRC hRC;

	CViewPortControl(LPCWSTR className);
	virtual ~CViewPortControl();
	/* Register/unregister the window class */
	
	void Unregister(void);

	bool InitGL();

	virtual void ReshapeGL(int width, int height);

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

	bool MakeCurrent();

	LPCWSTR ClassName;

	int Height;
	int Width;
	int X;
	int Y;	

	int Id;
};