// Altitude.cpp : Defines the entry point for the console application.
//
///#pragma once

///git add -u .
#ifdef _DEBUG
#define CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <fstream>

#include <GL/glew.h>
#include <GL/glut.h>



#include <ctime>


/***********************************************
*                                              *
*    Jeff Molofee's Revised OpenGL Basecode    *
*  Huge Thanks To Maxwell Sayles & Peter Puck  *
*            http://nehe.gamedev.net           *
*                     2001                     *
*                                              *
***********************************************/

#include <windows.h>													// Header File For The Windows Library
#include <windowsx.h>
#include <gl/gl.h>														// Header File For The OpenGL32 Library
#include <gl/glu.h>														// Header File For The GLu32 Library

#include "RadarClient.h"														// Header File For The NeHeGL Basecode
#include "CMesh.h"
#include "CScene.h"
#include "Util.h"

#include "CCamera.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"
//#include "FreeImage.h"

#include "ViewPortControl.h"
#include "CMinimap.h"

#include "CUserInterface.h"

#include "CRCSocket.h"

#define VIEW_PORT_CONTROL_ID     100

#define PANEL_WIDTH 500
#define INFO_HEIGHT 250


#define WM_TOGGLEFULLSCREEN (WM_USER+1)									// Application Define Message For Toggling

static BOOL g_isProgramLooping;											// Window Creation Loop, For FullScreen/Windowed Toggle																		// Between Fullscreen / Windowed Mode
static BOOL g_createFullScreen;											// If TRUE, Then Create Fullscreen

//CMesh*		g_pMesh = NULL;										// Mesh Data
//CScene*		g_Scn = NULL;
float		g_flYRot = 0.0f;									// Rotation
int			g_nFPS = 0, g_nFrames = 0;							// FPS and FPS Counter
DWORD		g_dwLastFPS = 0;									// Last FPS Check Time	
float g_mpph, g_mppv, g_lon, g_lat;
int g_texsize;
std::string g_altFile, g_imgFile, g_datFile;
//CCamera* g_Cam=NULL;



HWND g_ViewPortControl_hWnd;

HANDLE g_hIcon;

ViewPortControl *g_vpControl;
CMinimap *g_Minimap;
CUserInterface *g_UI;
CRCSocket *g_Socket;
#ifdef _DEBUG
DebugWindowInfo g_dwi;
#endif

void TerminateApplication(GL_Window* window)							// Terminate The Application
{
	PostMessage(window->hWnd, WM_QUIT, 0, 0);							// Send A WM_QUIT Message
	g_isProgramLooping = FALSE;											// Stop Looping Of The Program
}

void ToggleFullscreen(GL_Window* window)								// Toggle Fullscreen/Windowed
{
	PostMessage(window->hWnd, WM_TOGGLEFULLSCREEN, 0, 0);				// Send A WM_TOGGLEFULLSCREEN Message
}



BOOL ChangeScreenResolution(int width, int height, int bitsPerPixel)	// Change The Screen Resolution
{
	DEVMODE dmScreenSettings;											// Device Mode
	ZeroMemory(&dmScreenSettings, sizeof(DEVMODE));					// Make Sure Memory Is Cleared
	dmScreenSettings.dmSize = sizeof(DEVMODE);				// Size Of The Devmode Structure
	dmScreenSettings.dmPelsWidth = width;						// Select Screen Width
	dmScreenSettings.dmPelsHeight = height;						// Select Screen Height
	dmScreenSettings.dmBitsPerPel = bitsPerPixel;					// Select Bits Per Pixel
	dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
	{
		return FALSE;													// Display Change Failed, Return False
	}
	return TRUE;														// Display Change Was Successful, Return True
}

BOOL CreateMainWindow(GL_Window* window)									// This Code Creates Window
{
	
	DWORD windowStyle = WS_OVERLAPPEDWINDOW;							// Define Our Window Style
	DWORD windowExtendedStyle = WS_EX_APPWINDOW;						// Define The Window's Extended Style

	/*PIXELFORMATDESCRIPTOR pfd =											// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),									// Size Of This Pixel Format Descriptor
		1,																// Version Number
		PFD_DRAW_TO_WINDOW |											// Format Must Support Window
		PFD_SUPPORT_OPENGL |											// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,												// Must Support Double Buffering
		PFD_TYPE_RGBA,													// Request An RGBA Format
		window->init.bitsPerPixel,										// Select Our Color Depth
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
	};*/

	RECT windowRect = { 0, 0, window->init.width, window->init.height };	// Define Our Window Coordinates

	//GLuint PixelFormat;													// Will Hold The Selected Pixel Format

	/*if (window->init.isFullScreen == TRUE)								// Fullscreen Requested, Try Changing Video Modes
	{
		if (ChangeScreenResolution(window->init.width, window->init.height, window->init.bitsPerPixel) == FALSE)
		{
			// Fullscreen Mode Failed.  Run In Windowed Mode Instead
			MessageBox(HWND_DESKTOP, "Mode Switch Failed.\nRunning In Windowed Mode.", "Error", MB_OK | MB_ICONEXCLAMATION);
			window->init.isFullScreen = FALSE;							// Set isFullscreen To False (Windowed Mode)
		}
		else															// Otherwise (If Fullscreen Mode Was Successful)
		{
			ShowCursor(FALSE);											// Turn Off The Cursor
			windowStyle = WS_POPUP;										// Set The WindowStyle To WS_POPUP (Popup Window)
			windowExtendedStyle |= WS_EX_TOPMOST;						// Set The Extended Window Style To WS_EX_TOPMOST
		}																// (Top Window Covering Everything Else)
	}
	else																// If Fullscreen Was Not Selected
	{*/
		// Adjust Window, Account For Window Borders
		AdjustWindowRectEx(&windowRect, windowStyle, 0, windowExtendedStyle);
	/*}*/

	// Create The  Window
	window->hWnd = CreateWindowEx(windowExtendedStyle,					// Extended Style
		window->init.application->className,	// Class Name
		window->init.title,					// Window Title
		windowStyle,							// Window Style
		0, 0,								// Window X,Y Position
		windowRect.right - windowRect.left,	// Window Width
		windowRect.bottom - windowRect.top,	// Window Height
		HWND_DESKTOP,						// Desktop Is Window's Parent
		0,									// No Menu
		window->init.application->hInstance, // Pass The Window Instance
		window);
	/*window->hWnd = CreateWindowEx(windowExtendedStyle,					// Extended Style
		window->init.application->className,	// Class Name
		window->init.title,					// Window Title
		windowStyle,							// Window Style
		0, 0,								// Window X,Y Position
		windowRect.right - windowRect.left,	// Window Width
		windowRect.bottom - windowRect.top,	// Window Height
		window->hWnd,						// Desktop Is Window's Parent
		0,									// No Menu
		window->init.application->hInstance, // Pass The Window Instance
		window);*/

	

	if (window->hWnd == 0)												// Was Window Creation A Success?
	{
		return FALSE;													// If Not Return False
	}

	/*window->hDC = GetDC(window->hWnd);									// Grab A Device Context For This Window
	if (window->hDC == 0)												// Did We Get A Device Context?
	{
		// Failed
		DestroyWindow(window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	PixelFormat = ChoosePixelFormat(window->hDC, &pfd);				// Find A Compatible Pixel Format
	if (PixelFormat == 0)												// Did We Find A Compatible Format?
	{
		// Failed
		ReleaseDC(window->hWnd, window->hDC);							// Release Our Device Context
		window->hDC = 0;												// Zero The Device Context
		DestroyWindow(window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	if (SetPixelFormat(window->hDC, PixelFormat, &pfd) == FALSE)		// Try To Set The Pixel Format
	{
		// Failed
		ReleaseDC(window->hWnd, window->hDC);							// Release Our Device Context
		window->hDC = 0;												// Zero The Device Context
		DestroyWindow(window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	window->hRC = wglCreateContext(window->hDC);						// Try To Get A Rendering Context
	if (window->hRC == 0)												// Did We Get A Rendering Context?
	{
		// Failed
		ReleaseDC(window->hWnd, window->hDC);							// Release Our Device Context
		window->hDC = 0;												// Zero The Device Context
		DestroyWindow(window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	// Make The Rendering Context Our Current Rendering Context
	if (wglMakeCurrent(window->hDC, window->hRC) == FALSE)
	{
		// Failed
		wglDeleteContext(window->hRC);									// Delete The Rendering Context
		window->hRC = 0;												// Zero The Rendering Context
		ReleaseDC(window->hWnd, window->hDC);							// Release Our Device Context
		window->hDC = 0;												// Zero The Device Context
		DestroyWindow(window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}*/

	//ShowWindow(window->hWnd, SW_SHOWMAXIMIZED);								// Make The Window Visible
	//ShowWindow(window->hWnd, SW_NORMAL);
	SetWindowLong(window->hWnd, GWL_STYLE, (GetWindowLong(window->hWnd, GWL_STYLE) | WS_MAXIMIZE));
	ShowWindowAsync(window->hWnd, SW_SHOWMAXIMIZED);

	window->isVisible = TRUE;											// Set isVisible To True

	//ReshapeGL(window->init.width, window->init.height);				// Reshape Our GL Window

	ZeroMemory(window->keys, sizeof(Keys));							// Clear All Keys

	window->lastTickCount = GetTickCount();							// Get Tick Count

	return TRUE;														// Window Creating Was A Success
																		// Initialization Will Be Done In WM_CREATE
}

BOOL DestroyWindowGL(HWND hWnd, HDC hDC, HGLRC hRC)								// Destroy The OpenGL Window & Release Resources
{
	if (hWnd != 0)												// Does The Window Have A Handle?
	{
		if (hDC != 0)											// Does The Window Have A Device Context?
		{
			wglMakeCurrent(hDC, 0);							// Set The Current Active Rendering Context To Zero
			if (hRC != 0)										// Does The Window Have A Rendering Context?
			{
				wglDeleteContext(hRC);							// Release The Rendering Context

			}
			ReleaseDC(hWnd, hDC);						// Release The Device Context
		}
		DestroyWindow(hWnd);									// Destroy The Window
	}

	/*if (window->init.isFullScreen)										// Is Window In Fullscreen Mode
	{
		ChangeDisplaySettings(NULL, 0);									// Switch Back To Desktop Resolution
		ShowCursor(TRUE);												// Show The Cursor
	}*/
	return TRUE;														// Return True
}

// Process Window Message Callbacks
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	

	// Get The Window Context
	GL_Window* window = (GL_Window*)(GetWindowLong(hWnd, GWL_USERDATA));

	if (uMsg == WM_COMMAND) {
		if (g_UI) {
			g_UI->Wnd_Proc(hWnd, uMsg, wParam, lParam);
		}
	}
	if (uMsg == WM_HSCROLL ||
		uMsg == WM_VSCROLL) {
		if (g_UI) {
			g_UI->Wnd_Proc2(hWnd, uMsg, wParam, lParam);
		}
	}

	switch (uMsg)														// Evaluate Window Message
	{		
	case WM_LBUTTONDOWN:
		POINT p;
		if (GetCursorPos(&p))
		{
			//cursor position now in p.x and p.y
		}
		break;
	case WM_SYSCOMMAND:												// Intercept System Commands
	{
		switch (wParam)												// Check System Calls
		{
		case SC_SCREENSAVE:										// Screensaver Trying To Start?
		case SC_MONITORPOWER:									// Monitor Trying To Enter Powersave?
			return 0;												// Prevent From Happening
		}
		break;														// Exit
	}
	return 0;														// Return

	case WM_CREATE:													// Window Creation
	{
		
		CREATESTRUCT* creation = (CREATESTRUCT*)(lParam);			// Store Window Structure Pointer
		window = (GL_Window*)(creation->lpCreateParams);
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)(window));

		g_hIcon = LoadImage(0, _T("radar.ico"), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
		if (g_hIcon) {
			//Change both icons to the same icon handle.
			SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hIcon);
			SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)g_hIcon);

			//This will ensure that the application icon gets changed too.
			SendMessage(GetWindow(hWnd, GW_OWNER), WM_SETICON, ICON_SMALL, (LPARAM)g_hIcon);
			SendMessage(GetWindow(hWnd, GW_OWNER), WM_SETICON, ICON_BIG, (LPARAM)g_hIcon);
		}


		/*CreateWindowEx((DWORD)VIEW_PORT_WC, NULL, NULL, WS_CHILD | WS_VISIBLE,
			0, 0, 100, 100, hWnd, (HMENU)VIEW_PORT_CONTROL_ID, window->init.application->hInstance, NULL);*/
		
		RECT clientRect;

		GetClientRect(hWnd, &clientRect);

		g_vpControl->Add(hWnd, PANEL_WIDTH, 0, (clientRect.right - clientRect.left) - PANEL_WIDTH, clientRect.bottom - clientRect.top - INFO_HEIGHT);
		
		g_vpControl->InitGL();
		
		g_Socket = new CRCSocket(hWnd);
		
		g_UI = new CUserInterface(hWnd, g_vpControl, g_Socket, PANEL_WIDTH);
		//return 0;
		g_Minimap->Add(hWnd, 0, 0, g_UI->MinimapSize, g_UI->MinimapSize);
		g_Minimap->InitGL();

#ifdef _DEBUG
		g_Minimap->dwi = &g_dwi;

#endif // _DEBUG


		//g_Socket->Connect();
		/*g_ViewPortControl_hWnd = CreateWindowEx(
			WS_EX_CLIENTEDGE, // give it a standard border
			VIEW_PORT_WC,
			_T("A viewport control"),
			WS_VISIBLE | WS_CHILD,
			PANEL_WIDTH, 0, (clientRect.right - clientRect.left)- PANEL_WIDTH, clientRect.bottom - clientRect.top,
			hWnd,
			NULL, GetModuleHandle(0), NULL
			);*/
	}
	return 0;														// Return

	case WM_CLOSE: {												// Closing The Window
		TerminateApplication(window);								// Terminate The Application
	}
				   return 0;														// Return

	case WM_SIZE: {												// Size Action Has Taken Place
		RECT clientRect;
		GetClientRect(hWnd, &clientRect);
		g_vpControl->SetPosition(PANEL_WIDTH, 0, (clientRect.right - clientRect.left) - PANEL_WIDTH, clientRect.bottom - clientRect.top - INFO_HEIGHT);
		g_UI->Resize();
		switch (wParam)												// Evaluate Size Action
		{
		case SIZE_MINIMIZED:									// Was Window Minimized?
			window->isVisible = FALSE;							// Set isVisible To False
			return 0;												// Return

		case SIZE_MAXIMIZED:									// Was Window Maximized?
			window->isVisible = TRUE;							// Set isVisible To True
			//ReshapeGL(LOWORD(lParam), HIWORD(lParam));		// Reshape Window - LoWord=Width, HiWord=Height
			return 0;												// Return

		case SIZE_RESTORED:										// Was Window Restored?
			window->isVisible = TRUE;							// Set isVisible To True
			//ReshapeGL(LOWORD(lParam), HIWORD(lParam));		// Reshape Window - LoWord=Width, HiWord=Height
			return 0;												// Return
		}
	}
				  break;															// Break

	case WM_KEYDOWN: {											// Update Keyboard Buffers For Keys Pressed
		if ((wParam >= 0) && (wParam <= 255))						// Is Key (wParam) In A Valid Range?
		{
			window->keys->keyDown[wParam] = TRUE;					// Set The Selected Key (wParam) To True
			return 0;												// Return
		}
	}
					 break;															// Break

	case WM_KEYUP: {												// Update Keyboard Buffers For Keys Released
		if ((wParam >= 0) && (wParam <= 255))						// Is Key (wParam) In A Valid Range?
		{
			window->keys->keyDown[wParam] = FALSE;					// Set The Selected Key (wParam) To False
			return 0;												// Return
		}
	}
				   break;															// Break

	case WM_TOGGLEFULLSCREEN: {							// Toggle FullScreen Mode On/Off
		g_createFullScreen = (g_createFullScreen == TRUE) ? FALSE : TRUE;
		PostMessage(hWnd, WM_QUIT, 0, 0);
		break;															// Break
	}
	case WM_MOUSEWHEEL: {
		double zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		if (g_vpControl->Camera) {
			g_vpControl->Camera->MoveByView(zDelta);
		}
	}
						break;
	case WM_SOCKET: {
		if (WSAGETSELECTERROR(lParam))
		{
			MessageBox(hWnd,
				"Connection to server failed",
				"Error",
				MB_OK | MB_ICONERROR);
			SendMessage(hWnd, WM_DESTROY, NULL, NULL);
			break;
		}
		switch (WSAGETSELECTEVENT(lParam)) {
		case FD_READ:
			g_Socket->Read();
			/*SendMessage(hWnd,
				WM_SETTEXT,
				sizeof(szIncoming) - 1,
				reinterpret_cast<LPARAM>(&szHistory));*/
			break;

		case FD_CLOSE:

			/*MessageBox(hWnd,
				"Server closed connection",
				"Connection closed!",
				MB_ICONINFORMATION | MB_OK);*/
			g_Socket->Close();
			/*SendMessage(hWnd, WM_DESTROY, NULL, NULL);*/
			 
			break;
		}
	}
		break;
	case CM_POSTDATA: {
		g_Socket->PostData(wParam, lParam);
		g_vpControl->MakeCurrent();
		g_vpControl->Scene->RefreshSector(g_Socket->info_p, g_Socket->pts, g_Socket->s_rdrinit);

		g_UI->FillGrid(&g_Socket->Tracks);

		g_Socket->FreeMemory((char *)wParam);
	}
	break;
	case CM_CONNECT: {
		if (g_UI) {
			g_UI->ConnectionStateChanged(wParam);
		}
	}
	break;
	}



	return DefWindowProc(hWnd, uMsg, wParam, lParam);					// Pass Unhandled Messages To DefWindowProc
}

BOOL RegisterWindowClass(Application* application)						// Register A Window Class For This Application.
{																		// TRUE If Successful
																		// Register A Window Class
	WNDCLASSEX windowClass;												// Window Class
	ZeroMemory(&windowClass, sizeof(WNDCLASSEX));						// Make Sure Memory Is Cleared
	windowClass.cbSize = sizeof(WNDCLASSEX);					// Size Of The windowClass Structure
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraws The Window For Any Movement / Resizing
	windowClass.lpfnWndProc = (WNDPROC)(WindowProc);				// WindowProc Handles Messages
	windowClass.hInstance = application->hInstance;				// Set The Instance
	windowClass.hbrBackground = (HBRUSH)(COLOR_MENU + 1);			// Class Background Brush Color
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	windowClass.lpszClassName = application->className;				// Sets The Applications Classname
	if (RegisterClassEx(&windowClass) == 0)							// Did Registering The Class Fail?
	{
		// NOTE: Failure, Should Never Happen
		MessageBox(HWND_DESKTOP, "RegisterClassEx Failed!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;													// Return False (Failure)
	}
	return TRUE;														// Return True (Success)
}

// Program Entry (WinMain)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//MessageBox(HWND_DESKTOP, "Attach WinDbg and press OK", MB_OK, MB_ICONINFORMATION);
	g_hIcon = NULL;

	Application			application;									// Application Structure
	GL_Window			window;											// Window Structure
	Keys				keys;											// Key Structure
	BOOL				isMessagePumpActive;							// Message Pump Active?
	MSG					msg;											// Window Message Structure
	DWORD				tickCount;										// Used For The Tick Counter
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	long lBreakAlloc = 0;
	if (lBreakAlloc > 0)
	{
		_CrtSetBreakAlloc(lBreakAlloc);
	}
#endif
	std::ifstream files_txt("files.txt");
	
	if (!files_txt) {
		MessageBox(HWND_DESKTOP, "files.txt not found.", MB_OK, MB_ICONERROR);
		return 0;
	}
	std::getline(files_txt, g_altFile);
	std::getline(files_txt, g_imgFile);
	std::getline(files_txt, g_datFile);

	files_txt.close();

	if (strcmp(lpCmdLine, "") == 0) {
		g_lon = 37.631424;
		g_lat = 54.792393;
		g_mpph = 5.0f;
		g_mppv = 5.0f;
		g_texsize = 2048;
	}
	else {
		std::string strCmdLine(lpCmdLine);
		std::vector<std::string> v = rcutils::split(strCmdLine, ' ');

		g_lon = std::stof(v[0]);
		g_lat = std::stof(v[1]);
		g_mpph= std::stof(v[2]);
		g_mppv = std::stof(v[3]);
		g_texsize = std::stoi(v[4]);
	}
	
	g_vpControl = new ViewPortControl("VP3D");
	g_Minimap = new CMinimap("VPMiniMap");

	//return Deinitialize();
	//g_vpControl->Register();

																	// Fill Out Application Data
	application.className = "OpenGL";									// Application Class Name
	application.hInstance = hInstance;									// Application Instance

																		// Fill Out Window
	ZeroMemory(&window, sizeof(GL_Window));							// Make Sure Memory Is Zeroed
	window.keys = &keys;								// Window Key Structure
	window.init.application = &application;							// Window Application
	window.init.title = "RadarClient DEMO version 0.001";	// Window Title
	window.init.width = 1366;									// Window Width
	window.init.height = 768;									// Window Height
	window.init.bitsPerPixel = 16;									// Bits Per Pixel
	//window.init.isFullScreen = TRUE;									// Fullscreen? (Set To TRUE)

	ZeroMemory(&keys, sizeof(Keys));									// Zero keys Structure

																		// Ask The User If They Want To Start In FullScreen Mode?
	/*if (MessageBox(HWND_DESKTOP, "Would You Like To Run In Fullscreen Mode?", "Start FullScreen?", MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		window.init.isFullScreen = FALSE;								// If Not, Run In Windowed Mode
	}*/
	//window.init.isFullScreen = FALSE;								// If Not, Run In Windowed Mode

	// Register A Class For Our Window To Use
	if (RegisterWindowClass(&application) == FALSE)					// Did Registering A Class Fail?
	{
		// Failure
		MessageBox(HWND_DESKTOP, "Error Registering Window Class!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return -1;														// Terminate Application
	}
	
	char *myargv[1];
	int myargc = 1;
	myargv[0] = strdup("RadarClient");
	
	glutInit(&myargc, myargv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	free(myargv[0]);
	// Setup GL States
	//return 0;
	


	g_isProgramLooping = TRUE;											// Program Looping Is Set To TRUE
	//g_createFullScreen = window.init.isFullScreen;						// g_createFullScreen Is Set To User Default
	
	while (g_isProgramLooping)											// Loop Until WM_QUIT Is Received
	{
		// Create A Window
		//window.init.isFullScreen = g_createFullScreen;					// Set Init Param Of Window Creation To Fullscreen?
		
		if (CreateMainWindow(&window) == TRUE)							// Was Window Creation Successful?
		{
			//
#ifdef _DEBUG
			g_dwi.DebugEdit_ID = 1;
			OpenDebugWindow(hInstance, SW_SHOWMINIMIZED, window.hWnd, &g_dwi);
			DebugMessage(&g_dwi, "Hello");
#endif
			
			if (Initialize(&window, &keys) == FALSE)					// Call User Intialization
			{
				// Failure
				TerminateApplication(&window);							// Close Window, This Will Handle The Shutdown
			}

			else														// Otherwise (Start The Message Pump)
			{	// Initialize was a success
				isMessagePumpActive = TRUE;								// Set isMessagePumpActive To TRUE
				
				while (isMessagePumpActive == TRUE)						// While The Message Pump Is Active
				{
					
					// Success Creating Window.  Check For Window Messages
#ifdef _DEBUG
					if (PeekMessage(&msg, g_dwi.hWnd, 0, 0, PM_REMOVE) != 0) {
						DispatchMessage(&msg);
					}
#endif
					if (PeekMessage(&msg, window.hWnd, 0, 0, PM_REMOVE) != 0)
					{
						// Check For WM_QUIT Message
						if (msg.message != WM_QUIT)						// Is The Message A WM_QUIT Message?
						{
							DispatchMessage(&msg);						// If Not, Dispatch The Message
						}
						else											// Otherwise (If Message Is WM_QUIT)
						{
							isMessagePumpActive = FALSE;				// Terminate The Message Pump
						}
					}
					else												// If There Are No Messages
					{
						if (window.isVisible == FALSE)					// If Window Is Not Visible
						{
							WaitMessage();								// Application Is Minimized Wait For A Message
						}
						else											// If Window Is Visible
						{
							
							// Process Application Loop
							tickCount = GetTickCount();				// Get The Tick Count
							Update(tickCount - window.lastTickCount);	// Update The Counter
							window.lastTickCount = tickCount;			// Set Last Count To Current Count
							
							Draw();							

							if (g_vpControl->hRC) {
								g_vpControl->MakeCurrent();
								g_vpControl->Draw();
								SwapBuffers(g_vpControl->hDC);					// Swap Buffers (Double Buffering)
							}
							if (g_Minimap->hRC) {
								g_Minimap->MakeCurrent();
								g_Minimap->Draw();
								SwapBuffers(g_Minimap->hDC);
							}
							
						}
					}
				}														// Loop While isMessagePumpActive == TRUE
			}															// If (Initialize (...

			DestroyWindowGL(g_vpControl->hWnd, g_vpControl->hDC, g_vpControl->hRC);															// Application Is Finished
			DestroyWindowGL(g_Minimap->hWnd, g_Minimap->hDC, g_Minimap->hRC);
			//Deinitialize();											// User Defined DeInitialization

												// Destroy The Active Window
		}
		else															// If Window Creation Failed
		{
			// Error Creating Window
			MessageBox(HWND_DESKTOP, "Error Creating OpenGL Window", "Error", MB_OK | MB_ICONEXCLAMATION);
			g_isProgramLooping = FALSE;									// Terminate The Loop
		}
	}																	// While (isProgramLooping)
debugreturn:
	Deinitialize();
	UnregisterClass(application.className, application.hInstance);		// UnRegister Window Class
	return 0;
	
}																		// End Of WinMain()


														

GL_Window*	g_window;
Keys*		g_keys;

// TUTORIAL
// Based Off Of Code Supplied At OpenGL.org
bool IsExtensionSupported(char* szTargetExtension)
{
	const unsigned char *pszExtensions = NULL;
	const unsigned char *pszStart;
	unsigned char *pszWhere, *pszTerminator;

	// Extension names should not have spaces
	pszWhere = (unsigned char *)strchr(szTargetExtension, ' ');
	if (pszWhere || *szTargetExtension == '\0')
		return false;

	// Get Extensions String
	pszExtensions = glGetString(GL_EXTENSIONS);

	// Search The Extensions String For An Exact Copy
	pszStart = pszExtensions;
	for (;;)
	{
		pszWhere = (unsigned char *)strstr((const char *)pszStart, szTargetExtension);
		if (!pszWhere)
			break;
		pszTerminator = pszWhere + strlen(szTargetExtension);
		if (pszWhere == pszStart || *(pszWhere - 1) == ' ')
			if (*pszTerminator == ' ' || *pszTerminator == '\0')
				return true;
		pszStart = pszTerminator;
	}
	return false;
}
//~TUTORIAL

BOOL Initialize(GL_Window* window, Keys* keys)					// Any GL Init Code & User Initialiazation Goes Here
{
	g_window = window;
	g_keys = keys;




	g_vpControl->Scene = new CScene(g_altFile, g_imgFile, g_datFile, g_lon, g_lat, g_mpph, g_mppv, g_texsize);	 
	g_vpControl->Scene->Socket = g_Socket;
	g_vpControl->Scene->UI = g_UI;
	g_vpControl->UI = g_UI;
	g_vpControl->Camera = g_vpControl->Scene->Camera;
	
	g_Minimap->Scene = g_vpControl->Scene;	
	g_Minimap->UI = g_UI;
	g_Minimap->Camera = g_vpControl->Scene->Camera;

/*	if (!g_vpControl->Scene->PrepareVBOs())
	{
		MessageBox(NULL, "Error Loading Scene", "Error", MB_OK);
		return false;
	}

	

	g_vpControl->Scene->BuildVBOs();*/									// Build The VBOs
	
	g_vpControl->Camera->SetAll(0, 1, 0, 0, 0, 1, 0, 1, 0, 
		60.0f, 4.0f/3.0f, 1.0f, 10000.0f,
		0.01, LookAtCallback_);


	


	return TRUE;												// Return TRUE (Initialization Successful)
}

int Deinitialize(void)										// Any User DeInitialization Goes Here
{

	if (g_vpControl) {
		if (g_vpControl->Scene)
			delete g_vpControl->Scene;
		delete g_vpControl;
	}
	if (g_Minimap)
		delete g_Minimap;
	if (g_Socket)
		delete g_Socket;
	if (g_UI)
		delete g_UI;
	if (g_hIcon)
		DestroyIcon((HICON)g_hIcon);
	return 0;
}

void Update(DWORD milliseconds)								// Perform Motion Updates Here
{
	//g_flYRot += (float)(milliseconds) / 1000.0f * 25.0f;		// Consistantly Rotate The Scenery

	if (g_keys->keyDown[VK_ESCAPE] == TRUE)					// Is ESC Being Pressed?
	{
		TerminateApplication(g_window);						// Terminate The Program
	}

	if (g_keys->keyDown[VK_F1] == TRUE)						// Is F1 Being Pressed?
	{
		ToggleFullscreen(g_window);							// Toggle Fullscreen Mode
	}
}

void Draw(void)
{
	if (GetTickCount() - g_dwLastFPS >= 1000)					// When A Second Has Passed...
	{
		g_dwLastFPS = GetTickCount();							// Update Our Time Variable
		g_nFPS = g_nFrames;										// Save The FPS
		g_nFrames = 0;											// Reset The FPS Counter

		char szTitle[256] = { 0 };									// Build The Title String
		sprintf(szTitle, "RadarClient v0.000 - %d FPS", g_nFPS);

		SetWindowText(g_window->hWnd, szTitle);				// Set The Title
	}
	g_nFrames++;
	// Increment Our FPS Counter

//drawscene_debug:
	//g_vpControl->Scene->DrawScene();

}

void LookAtCallback_(double eyex, double eyey, double eyez, double centerx, double centery, double centerz, double upx, double upy, double upz)
{
	gluLookAt(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);
}



