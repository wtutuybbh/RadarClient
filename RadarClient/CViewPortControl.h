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

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"
//#include "C3DObject.h"
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
public:
	CScene *Scene{ NULL };
	CCamera *Camera{ NULL };
	CUserInterface *UI{ NULL };
	HWND hWnd;
	HDC hDC;
	HGLRC hRC;

	CViewPortControl(LPCSTR className);
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
	int DisplayPoints, DisplaySeries, DisplayRLIs, DisplayMap, DisplayLandscape;

	C3DObjectModel* Get3DObject(int x, int y);

	bool MakeCurrent();

	LPCSTR ClassName;

	int Height;
	int Width;
	int X;
	int Y;	

	int Id;
};

#endif  /* CUSTOM_H */