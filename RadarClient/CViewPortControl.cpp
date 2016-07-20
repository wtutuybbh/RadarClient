/* File custom.c
* (custom control implementation)
*/

#include "CViewPortControl.h"
#include "CCamera.h"
#include "CScene.h"
#include "CUserInterface.h"
#include "C3DObject.h"
#include "Util.h"
#include "CSettings.h"

void CViewPortControl::Paint()
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rect;
	

	GetClientRect(hWnd, &rect);

	hdc = BeginPaint(hWnd, &ps);
	SetTextColor(hdc, RGB(0, 0, 0));
	SetBkMode(hdc, TRANSPARENT);
	DrawText(hdc, _T("Loading..."), -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	EndPaint(hWnd, &ps);
}

void CViewPortControl::Draw()
{
	//g_vpControl->MakeCurrent();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear Screen And Depth Buffer
															// Reset The Modelview Matrix
															//goto drawscene_debug;
															// Get FPS
	

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();													// Reset The Projection Matrix
	gluPerspective(60.0f, 4.0f/3.0f,			// Calculate The Aspect Ratio Of The Window
		1.0f, 10000.0f);
	if (Camera) {
		Camera->LookAt();
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (Scene)
		Scene->DrawScene(this);
}


LRESULT CViewPortControl::ViewPortControlProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	short fwKeys, zDelta;

	switch (uMsg) {

	case WM_PAINT:
		Paint();
		return 0;
	case WM_SIZE:
		switch (wParam)												// Evaluate Size Action
		{
		case SIZE_MINIMIZED:									// Was Window Minimized?
			//window->isVisible = FALSE;							// Set isVisible To False
			return 0;												// Return

		case SIZE_MAXIMIZED:									// Was Window Maximized?
			//window->isVisible = TRUE;							// Set isVisible To True
			ReshapeGL(LOWORD(lParam), HIWORD(lParam));		// Reshape Window - LoWord=Width, HiWord=Height
			return 0;												// Return

		case SIZE_RESTORED:										// Was Window Restored?
			//window->isVisible = TRUE;							// Set isVisible To True
			ReshapeGL(LOWORD(lParam), HIWORD(lParam));		// Reshape Window - LoWord=Width, HiWord=Height
			return 0;												// Return
		}
		return 0;
	

	case WM_MOUSEMOVE:
		if (Camera) {
			Camera->newPosition.x = GET_X_LPARAM(lParam);
			Camera->newPosition.y = GET_Y_LPARAM(lParam);
			if (wParam == MK_LBUTTON) {
				Camera->Rotate(Camera->newPosition.x - Camera->startPosition.x, glm::vec3(0, -1, 0));
				Camera->Rotate(Camera->newPosition.y - Camera->startPosition.y, glm::cross(glm::vec3(0, 1, 0), Camera->GetDirection()));

				if (UI) {
					float r = glm::length(Camera->GetDirection());
					float e = acos(Camera->GetDirection().y / r);

					float a = Camera->GetAzimut();

					UI->SetTrackbarValue_Turn(50 * a / M_PI + 50);
					//UI->SetTrackbarValue_VTilt(100 * e / M_PI);
				}
			}
			Camera->startPosition = Camera->newPosition;
			
			
		}
		break;
	case WM_LBUTTONDOWN:
		SetFocus(hwnd);
		if (Camera) {
			Camera->startPosition.x = GET_X_LPARAM(lParam);
			Camera->startPosition.y = GET_Y_LPARAM(lParam);
			C3DObjectModel* o = Get3DObject(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			
				if (Scene && UI)
				{
					if (o)
					{
						o->SetGeoCoords(Scene->GetGeographicCoordinates(o->GetCartesianCoords()));
						Scene->PushSelection(o);
					}
					else
					{
						Scene->ClearSelection();
					}
					UI->FillInfoGrid(Scene);
				}									
		}
		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
void CViewPortControl::Add(HWND parent, int x, int y, int w, int h)
{
	hWnd = CreateWindowEx(
		WS_EX_CLIENTEDGE, // give it a standard border
		ClassName,
		_T("A viewport control"),
		WS_VISIBLE | WS_CHILD,
		x, y, w, h,
		parent,
		NULL, GetModuleHandle(0), this
		);
}
void CViewPortControl::SetPosition(int x, int y, int w, int h)
{
	Width = w;
	Height = h;
	X = x;
	Y = y;
	SetWindowPos(hWnd, NULL, x, y, w, h, 0);
}

glm::mat4 CViewPortControl::GetProjMatrix() const
{
	if (Camera)
		return Camera->GetProjection();
	return glm::mat4(1.0f);
}

glm::mat4 CViewPortControl::GetViewMatrix() const
{
	if (Camera)
		return Camera->GetView();
	return glm::mat4(1.0f);
}

CViewPortControl::CViewPortControl(LPCSTR className)
{
	ClassName = className;

	hRC = NULL;
	hDC = NULL;
	Init();

	Scene = NULL;
	Camera = NULL;
	UI = NULL;
	Register();
}


CViewPortControl::~CViewPortControl()
{
	Unregister();
}

C3DObjectModel* CViewPortControl::Get3DObject(int x, int y)
{
	glm::vec4 viewport = glm::vec4(0, 0, Width, Height);

	glm::vec3 p0 = glm::unProject(glm::vec3(x, Height - y - 1, 0.0f), Camera->GetView(), Camera->GetProjection(), viewport);
	glm::vec3 p1 = glm::unProject(glm::vec3(x, Height - y - 1, 1.0f), Camera->GetView(), Camera->GetProjection(), viewport);

	if(UI->MeasureDistance())
	{
		Scene->GetPointOnSurface(p0, p1);
	}
	int index;
	C3DObjectModel *o = Scene->GetSectorPoint(this, glm::vec2(x, y), index);
	C3DObjectModel *t = Scene->GetFirstTrackBetweenPoints(this, glm::vec2(x, y), index);
	if (o && !t) {
		o->SetColor(glm::vec4(0, 1, 0, 1));
		return o;
	}
	if (!o && t) {
		t->SetColor(glm::vec4(0, 1, 0, 1));
		return t;
	}
	if(o && t)
	{
		float dist_o = o->DistanceToPoint(p0);
		float dist_t = t->DistanceToPoint(p0);
		if (dist_t < dist_o) {
			t->SetColor(glm::vec4(0, 1, 0, 1));
			return t;
		}
		else
		{
			o->SetColor(glm::vec4(0, 1, 0, 1));
			return o;
		}
	}
	return o;
}

bool CViewPortControl::MakeCurrent()
{
	if (wglMakeCurrent(hDC, hRC) == FALSE)
	{
		// Failed
		wglDeleteContext(hRC);									// Delete The Rendering Context
		hRC = 0;												// Zero The Rendering Context
		ReleaseDC(hWnd, hDC);							// Release Our Device Context
		hDC = 0;												// Zero The Device Context
		DestroyWindow(hWnd);									// Destroy The Window
		hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	return TRUE;
}

LRESULT CALLBACK CViewPortControl::stWinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CViewPortControl *vpc;
	if (uMsg == WM_NCCREATE)
	{
		// get the pointer to the window from
		// lpCreateParams which was set in CreateWindow
		SetWindowLong(hwnd, GWL_USERDATA, (long)((LPCREATESTRUCT(lParam))->lpCreateParams));
	}

	// get the pointer to the window
	vpc = (CViewPortControl*)GetWindowLong(hwnd, GWL_USERDATA);

	// if we have the pointer, go to the message handler of the window
	// else, use DefWindowProc
	if (vpc)
		return vpc->ViewPortControlProc(hwnd, uMsg, wParam, lParam);
	else
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool CViewPortControl::Register(void)
{
	WNDCLASSEXA wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)stWinMsgHandler;// ViewPortControlProc;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = ClassName;

	wc.hInstance = GetModuleHandle(0);
	wc.hIcon = 0;
	wc.lpszMenuName = 0;
	wc.hbrBackground = (HBRUSH)GetSysColorBrush(COLOR_BTNFACE);
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hIconSm = 0;

	if (RegisterClassEx(&wc) == 0)							// Did Registering The Class Fail?
	{
		// NOTE: Failure, Should Never Happen
		MessageBox(HWND_DESKTOP, "CViewPortControl: RegisterClassEx Failed!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;													// Return False (Failure)
	}
	return TRUE;
}

void CViewPortControl::Init()
{
	DisplayPoints = 1;
	DisplaySeries = 1;
	DisplayRLIs = 0;
	DisplayMap = 1;
	DisplayLandscape = 1;
}

void CViewPortControl::Unregister(void)
{
	UnregisterClass(ClassName, NULL);
}

bool CViewPortControl::InitGL()
{
	DWORD windowStyle = WS_OVERLAPPEDWINDOW;							// Define Our Window Style
	DWORD windowExtendedStyle = WS_EX_APPWINDOW;						// Define The Window's Extended Style

	PIXELFORMATDESCRIPTOR pfd =											// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),									// Size Of This Pixel Format Descriptor
		1,																// Version Number
		PFD_DRAW_TO_WINDOW |											// Format Must Support Window
		PFD_SUPPORT_OPENGL |											// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,												// Must Support Double Buffering
		PFD_TYPE_RGBA,													// Request An RGBA Format
		16,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,												// Color Bits Ignored
		0,																// No Alpha Buffer
		0,																// Shift Bit Ignored
		0,																// No Accumulation Buffer
		0, 0, 0, 0,														// Accumulation Bits Ignored
		16,																// 16Bit Z-Buffer (Depth Buffer)  
		0,																// No Stencil Buffer
		0,																// No Auxiliary Buffer
		PFD_MAIN_PLANE,													// Main Drawing Layer
		0,																// Reserved
		0, 0, 0															// Layer Masks Ignored
	};

	RECT clientRect;
	
	GetClientRect(hWnd, &clientRect);

	RECT windowRect = { 0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top };	// Define Our Window Coordinates

	GLuint PixelFormat;

	hDC = GetDC(hWnd);

	if (hDC == 0)												// Did We Get A Device Context?
	{
		// Failed
		DestroyWindow(hWnd);									// Destroy The Window
		hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	PixelFormat = ChoosePixelFormat(hDC, &pfd);

	if (PixelFormat == 0)												// Did We Find A Compatible Format?
	{
		// Failed
		ReleaseDC(hWnd, hDC);							// Release Our Device Context
		hDC = 0;												// Zero The Device Context
		DestroyWindow(hWnd);									// Destroy The Window
		hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	if (SetPixelFormat(hDC, PixelFormat, &pfd) == FALSE)		// Try To Set The Pixel Format
	{
		// Failed
		ReleaseDC(hWnd, hDC);							// Release Our Device Context
		hDC = 0;												// Zero The Device Context
		DestroyWindow(hWnd);									// Destroy The Window
		hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	hRC = wglCreateContext(hDC);						// Try To Get A Rendering Context
	if (hRC == 0)												// Did We Get A Rendering Context?
	{
		// Failed
		ReleaseDC(hWnd, hDC);							// Release Our Device Context
		hDC = 0;												// Zero The Device Context
		DestroyWindow(hWnd);									// Destroy The Window
		hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	//return true;
	// Make The Rendering Context Our Current Rendering Context
	if (!MakeCurrent())
		return false;

	GLenum err = glewInit();

	if (GLEW_OK != err)
	{
		GLenum en = glGetError();
		const GLubyte *s = glewGetErrorString(err);
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", s);
	}
	glm::vec4 bgColor = CSettings::GetColor(ColorBackgroud);
	glClearColor(bgColor.r, bgColor.g, bgColor.b, bgColor.a);
																//glClearDepth(1.0f);										// Depth Buffer Setup
	glDepthFunc(GL_LEQUAL);									// The Type Of Depth Testing¸¸¸¸ (Less Or Equal)

	glShadeModel(GL_SMOOTH);									// Select Smooth Shading
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);			// Set Perspective Calculations To Most Accurate

	return true;
}

void CViewPortControl::ReshapeGL(int width, int height)									// Reshape The Window When It's Moved Or Resized
{
	Height = height;
	Width = width;
	if (Scene) {
		Scene->width = width;
		Scene->height = height;
	}
	if (hDC && hRC) {
		MakeCurrent();
		glViewport(0, 0, (GLsizei)(width), (GLsizei)(height));				// Reset The Current Viewport
	}
}