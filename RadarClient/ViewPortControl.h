#pragma once

/* File custom.h
* (custom control interface)
*/

#ifndef VIEW_PORT_CONTROL_H
#define VIEW_PORT_CONTROL_H

#include <tchar.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include <GL/glew.h>
#include <GL/glut.h>

#define _USE_MATH_DEFINES 
#include <math.h>

/* Window class */
#define VIEW_PORT_WC   _T("ViewPortControl")

class CScene;
class CCamera;
class CUserInterface;

class ViewPortControl {
public:
	CScene *Scene;
	CCamera *Camera;
	CUserInterface *UI;
	HWND hWnd;
	HDC hDC;
	HGLRC hRC;

	ViewPortControl(LPCSTR className);
	~ViewPortControl();
	/* Register/unregister the window class */
	
	void Unregister(void);

	bool InitGL();

	virtual void ReshapeGL(int width, int height);

	virtual LRESULT ViewPortControlProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void Paint();

	virtual void Draw();

	void Add(HWND parent, int x, int y, int w, int h);

	void SetPosition(int x, int y, int w, int h);

	int DisplayPoints, DisplaySeries, DisplayRLIs, DisplayMap, DisplayLandscape;

	bool MakeCurrent();

	LPCSTR ClassName;

	int Height;
	int Width;
	int X;
	int Y;
private:
	static LRESULT CALLBACK stWinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool Register(void);
	void Init();
};

#endif  /* CUSTOM_H */