#include "stdafx.h"
#include "RadarClient.h"
#include "CRCSocket.h"
#include "CMesh.h"
#include "CScene.h"
#include "Util.h"
#include "CCamera.h"
#include "CViewPortControl.h"
#include "CMinimap.h"
#include "CUserInterface.h"

#include "CSettings.h"
#include <GL/glut.h>
#include "CRCLogger.h"

#include "resource.h"
#include "resource1.h"
#include <CommCtrl.h>
#include "CRImage.h"

#define VIEW_PORT_CONTROL_ID     100

#define PANEL_WIDTH 450
#define INFO_HEIGHT 250


#define WM_TOGGLEFULLSCREEN (WM_USER+1)									// Application Define Message For Toggling

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "ComCtl32.lib")

static BOOL g_isProgramLooping;											// Window Creation Loop, For FullScreen/Windowed Toggle																		// Between Fullscreen / Windowed Mode
static BOOL g_createFullScreen;											// If TRUE, Then Create Fullscreen

float		g_flYRot = 0.0f;									// Rotation
int			g_nFPS = 0, g_nFrames = 0;							// FPS and FPS Counter
DWORD		g_dwLastFPS = 0;									// Last FPS Check Time	
float g_mpph, g_mppv, g_lon, g_lat;
int g_texsize;
std::string g_altFile, g_imgFile, g_datFile;

HWND g_ViewPortControl_hWnd;

HANDLE g_hIcon;

CViewPortControl *g_vpControl;
CMinimap *g_Minimap;
CUserInterface *g_UI;
CRCSocket *g_Socket;
#ifdef _DEBUG
DebugWindowInfo g_dwi;
#endif


//std::mutex m;

bool g_Initialized = false;

bool g_AltPressed = false;

GL_Window*	g_window;
Keys*		g_keys;

std::string requestID = "RadarClient";

//считаем, что VBA и VBO у нас есть, чтобы запустилась первая проверка:
bool hasVBO = true;
bool hasVAO = true;

int g_nCmdShow;

void TerminateApplication(GL_Window* window)							// Terminate The Application
{
	if (window)
	{
		PostMessage(window->hWnd, WM_QUIT, 0, 0);// Send A WM_QUIT Message
	}
							
	g_isProgramLooping = FALSE;											// Stop Looping Of The Program

	if (g_Socket)
	{
		delete g_Socket;
		g_Socket = nullptr;
	}
}

void ToggleFullscreen(GL_Window* window)								// Toggle Fullscreen/Windowed
{
	PostMessage(window->hWnd, WM_TOGGLEFULLSCREEN, 0, 0);				// Send A WM_TOGGLEFULLSCREEN Message
}



BOOL ChangeScreenResolution(int width, int height, int bitsPerPixel)	// Change The Screen Resolution
{
	string context = "ChangeScreenResolution";
	CRCLogger::Info(requestID, context, "Start");
	DEVMODE dmScreenSettings;											// Device Mode
	ZeroMemory(&dmScreenSettings, sizeof(DEVMODE));					// Make Sure Memory Is Cleared
	dmScreenSettings.dmSize = sizeof(DEVMODE);				// Size Of The Devmode Structure
	dmScreenSettings.dmPelsWidth = width;						// Select Screen Width
	dmScreenSettings.dmPelsHeight = height;						// Select Screen Height
	dmScreenSettings.dmBitsPerPel = bitsPerPixel;					// Select Bits Per Pixel
	dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
	{
		CRCLogger::Error(requestID, context, "ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL => return false");
		return FALSE;													// Display Change Failed, Return False
	}
	CRCLogger::Info(requestID, context, "End => return true");
	return TRUE;														// Display Change Was Successful, Return True
}

BOOL CreateMainWindow(GL_Window* window)									// This Code Creates Window
{
	string context = "CreateMainWindow";
	CRCLogger::Info(requestID, context, "Start");

	DWORD windowStyle = WS_OVERLAPPEDWINDOW;							// Define Our Window Style
	DWORD windowExtendedStyle = WS_EX_APPWINDOW;						// Define The Window's Extended Style

	RECT windowRect = { 0, 0, window->init.width, window->init.height };	// Define Our Window Coordinates

	AdjustWindowRectEx(&windowRect, windowStyle, 0, windowExtendedStyle);

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

	if (window->hWnd == nullptr)												// Was Window Creation A Success?
	{
		CRCLogger::Error(requestID, context, "window->hWnd == 0 => return false");
		return FALSE;													// If Not Return False
	}

	SetWindowLong(window->hWnd, GWL_STYLE, (GetWindowLong(window->hWnd, GWL_STYLE) | WS_MAXIMIZE));
	ShowWindowAsync(window->hWnd, SW_SHOWMAXIMIZED);

	window->isVisible = TRUE;											// Set isVisible To True

	//ReshapeGL(window->init.width, window->init.height);				// Reshape Our GL Window

	ZeroMemory(window->keys, sizeof(Keys));							// Clear All Keys

	window->lastTickCount = GetTickCount();							// Get Tick Count

	CRCLogger::Info(requestID, context, "End => return true");
	return TRUE;														// Window Creating Was A Success
																		// Initialization Will Be Done In WM_CREATE
}

BOOL DestroyWindowGL(HWND hWnd, HDC hDC, HGLRC hRC)								// Destroy The OpenGL Window & Release Resources
{
	string context = "DestroyWindowGL";
	CRCLogger::Info(requestID, context, (boost::format("Start... hwnd=%1%, hDC=%2%, hRC=%3%") % hWnd % hDC % hRC).str());

	if (hWnd != nullptr)												// Does The Window Have A Handle?
	{
		if (hDC != nullptr)											// Does The Window Have A Device Context?
		{
			wglMakeCurrent(hDC, nullptr);							// Set The Current Active Rendering Context To Zero
			if (hRC != nullptr)										// Does The Window Have A Rendering Context?
			{
				wglDeleteContext(hRC);							// Release The Rendering Context
			}
			ReleaseDC(hWnd, hDC);						// Release The Device Context
		}
		DestroyWindow(hWnd);									// Destroy The Window
	}
	CRCLogger::Info(requestID, context, "End => return true");
	return TRUE;														// Return True
}

// Process Window Message Callbacks
LRESULT CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	string context = "WindowProc";
	

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

	case WM_CREATE:													// Window Creation
	{
		CRCLogger::Info(requestID, context, "WM_CREATE");
		CREATESTRUCT* creation = (CREATESTRUCT*)(lParam);			// Store Window Structure Pointer
		window = (GL_Window*)(creation->lpCreateParams);
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)(window));

		g_hIcon = LoadImage(nullptr, _T("radar.ico"), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
		if (g_hIcon) {
			//Change both icons to the same icon handle.
			SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hIcon);
			SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)g_hIcon);

			//This will ensure that the application icon gets changed too.
			SendMessage(GetWindow(hWnd, GW_OWNER), WM_SETICON, ICON_SMALL, (LPARAM)g_hIcon);
			SendMessage(GetWindow(hWnd, GW_OWNER), WM_SETICON, ICON_BIG, (LPARAM)g_hIcon);

			CRCLogger::Info(requestID, context, "Icon loaded successfully");
		}
		else
		{
			CRCLogger::Warn(requestID, context, "Icon not loaded");
		}
		
		RECT clientRect;

		GetClientRect(hWnd, &clientRect);

		g_vpControl->Add(hWnd, PANEL_WIDTH, 0, (clientRect.right - clientRect.left) - PANEL_WIDTH, clientRect.bottom - clientRect.top - INFO_HEIGHT);
		g_vpControl->Id = Main;
		g_vpControl->InitGL();
		
		g_Socket = new CRCSocket(hWnd);
		g_Socket->Connect();
		
		
		g_UI = new CUserInterface(hWnd, g_vpControl, g_Socket, PANEL_WIDTH);

		//return 0;
		g_Minimap->Add(hWnd, 0, 0, g_UI->MinimapSize, g_UI->MinimapSize);
		g_Minimap->Id = MiniMap;
		g_Minimap->InitGL();

		HRSRC       hrsrc;
		HGLOBAL     hglobal;
		HINSTANCE hInstance = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
		hrsrc = FindResource(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), RT_DIALOG);

		hglobal = ::LoadResource(hInstance, hrsrc);

		HWND hwnd1 = CreateDialogIndirect(hInstance, (LPCDLGTEMPLATE)hglobal, hWnd, (DLGPROC)DlgProc);

		ShowWindow(hwnd1, g_nCmdShow);

		SetWindowPos(hwnd1, HWND_TOP, 0, 720, 0, 0, SWP_NOSIZE);

#ifdef _DEBUG
		g_Minimap->dwi = &g_dwi;
		g_UI->dwi = &g_dwi;
		g_Socket->dwi = &g_dwi;
#endif // _DEBUG
		CRCLogger::Info(requestID, context, "WM_CREATE: End");
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

	case WM_KEYDOWN: 
	{											// Update Keyboard Buffers For Keys Pressed
		if ((wParam >= 0) && (wParam <= 255))						// Is Key (wParam) In A Valid Range?
		{
			window->keys->keyDown[wParam] = TRUE;					// Set The Selected Key (wParam) To True
			//return 0;												// Return
		}
		if (g_AltPressed && wParam == 76) // Alt + L
		{
			Initialize();
		}
		CRCLogger::Info(requestID, context, (boost::format("WM_KEYDOWN: uMsg=%1%, wParam=%2%, lParam=%3%") % hWnd % wParam % lParam).str());
	}
					 break;															// Break

	case WM_KEYUP: {												// Update Keyboard Buffers For Keys Released
		if ((wParam >= 0) && (wParam <= 255))						// Is Key (wParam) In A Valid Range?
		{
			window->keys->keyDown[wParam] = FALSE;					// Set The Selected Key (wParam) To False
			//return 0;												// Return
		}
		CRCLogger::Info(requestID, context, (boost::format("WM_KEYUP: uMsg=%1%, wParam=%2%, lParam=%3%") % hWnd % wParam % lParam).str());
	}
				   break;															// Break
	case WM_SYSKEYDOWN:
	{
		if (wParam == 18)
		{
			g_AltPressed = true;
		}
		CRCLogger::Info(requestID, context, (boost::format("WM_SYSKEYDOWN: uMsg=%1%, wParam=%2%, lParam=%3%") % hWnd % wParam % lParam).str());
		return 0;
	}
	break;
	case WM_SYSKEYUP:
	{
		if (wParam == 18)
		{
			g_AltPressed = false;
		}
		CRCLogger::Info(requestID, context, (boost::format("WM_SYSKEYUP: uMsg=%1%, wParam=%2%, lParam=%3%") % hWnd % wParam % lParam).str());
		return 0;
	}
	break;
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
			/*MessageBox(hWnd,
				"Connection to server failed",
				"Error",
				MB_OK | MB_ICONERROR);
			SendMessage(hWnd, WM_DESTROY, NULL, NULL);*/
			LOG_ERROR__("Connection to server failed");
			break;
		}
		switch (WSAGETSELECTEVENT(lParam)) 
		{
		case FD_READ:
			if (g_Socket) 
			{
				g_Socket->Read();
			}
			break;

		case FD_CLOSE:
			if (g_Socket)
			{
				g_Socket->Close();
			}
			break;
		case FD_CONNECT:
			if (g_Socket)
			{
				LOG_INFO__("CONNECT OK");
			}
			break;
		}

	}
		break;
	case CM_POSTDATA: {
		unsigned int msg=-1;
		if (g_Socket)
			msg = g_Socket->PostData(wParam, lParam);
		//g_vpControl->MakeCurrent();
		if (g_vpControl && g_vpControl->Scene) {
			switch (msg) 
			{
				case MSG_RPOINTS: 
				{
					g_vpControl->Scene->RefreshSector(g_Socket->info_p, g_Socket->pts, g_Socket->s_rdrinit);
				}
				break;
				case MSG_OBJTRK:
				{
					g_vpControl->Scene->RefreshTracks(&g_Socket->Tracks);
				}
				break;
				case MSG_RIMAGE:
				{
					g_vpControl->Scene->RefreshImages(g_Socket->info_i, g_Socket->pixels);
				}
				break;
				case MSG_INIT:
				{
					g_vpControl->Scene->Init(g_Socket->s_rdrinit);
				}
				break;
				default:
					break;
			}
		}
		if (g_UI) {
			g_UI->FillGrid(&g_Socket->Tracks);
		}

		g_Socket->FreeMemory((char *)wParam);
	}
	break;
	case CM_CONNECT: {
		if (g_UI) {
			g_UI->ConnectionStateChanged(wParam);
			g_UI->FillInfoGrid(g_vpControl->Scene);
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
	windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);			// Load The Arrow Pointer
	windowClass.lpszClassName = application->className;				// Sets The Applications Classname
	if (RegisterClassEx(&windowClass) == 0)							// Did Registering The Class Fail?
	{
		// NOTE: Failure, Should Never Happen
		MessageBox(HWND_DESKTOP, "RegisterClassEx Failed!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;													// Return False (Failure)
	}
	return TRUE;														// Return True (Success)
}

class outbuf2 : public std::streambuf {
public:
	outbuf2() {
		setp(nullptr, nullptr);
	}

	virtual int_type overflow(int_type c = traits_type::eof()) {
		return fputc(c, stdout) == EOF ? traits_type::eof() : c;
	}
};

// Program Entry (WinMain)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{	
	g_nCmdShow = nCmdShow;
	MSG msg;

	AllocConsole();
	HWND console = GetConsoleWindow();
	SetWindowPos(console, HWND_TOPMOST, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	ShowWindow(console, SW_NORMAL);

	freopen("CONOUT$", "w", stdout);

	string context = "WinMain";
	
	/*
	HWND hDlg;	
	BOOL ret;
	InitCommonControls();
	hDlg = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_FORMVIEW), 0, (DLGPROC)WindowProc, 0);

	if (!hDlg)
	{
		string err = GetLastErrorAsString();
		LOG_ERROR__(err.c_str());
		return -1;
	}

	ShowWindow(hDlg, nCmdShow);

	while ((ret = GetMessage(&msg, 0, 0, 0)) != 0) {
		if (ret == -1)
			return -1;

		if (!IsDialogMessage(hDlg, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return 0; 
	
	*/

	

	LOG_INFO__("RadarClient started.");

	LOG_INFO__("sizeof(RDR_INITCL) = %d", sizeof(RDR_INITCL));
		
	CSettings::Init();
	C3DObjectModel::_id = 0;
	C3DObjectModel::_testid = 0;

	g_hIcon = nullptr;

	Application			application;									// Application Structure
	GL_Window			window;											// Window Structure
	Keys				keys;											// Key Structure
	BOOL				isMessagePumpActive;							// Message Pump Active?
	DWORD				tickCount;										// Used For The Tick Counter
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	long lBreakAlloc = 0;
	if (lBreakAlloc > 0)
	{
		_CrtSetBreakAlloc(lBreakAlloc);
	}
#endif
	
	std::ifstream settings_txt("settings.txt");
	
	if (!settings_txt) {
		LOG_ERROR__("settings.txt not found.");
		return 0;
	}
	std::getline(settings_txt, g_altFile);
	std::getline(settings_txt, g_imgFile);
	std::getline(settings_txt, g_datFile);
	string s;
	while (settings_txt.good()) {
		std::getline(settings_txt, s);
		std::vector<std::string> strparts = rcutils::split(s, '=');
		string settingName = strparts.at(0);
		if (settingName.substr(0, 6) == "String")
		{
			std::string settingValue(strparts.at(1).c_str());
			CSettings::SetString(CSettings::GetIndex(settingName), settingValue);
		}
		if(settingName.substr(0, 5) == "Float")
		{
			float settingValue = ::atof(strparts.at(1).c_str());
			CSettings::SetFloat(CSettings::GetIndex(settingName), settingValue);
		}
		if (settingName.substr(0, 3) == "Int")
		{
			int settingValue = ::_atoi64(strparts.at(1).c_str());
			CSettings::SetInt(CSettings::GetIndex(settingName), settingValue);
		}
		if (settingName.substr(0, 5) == "Color")
		{
			unsigned short shift=0;
			if (strparts.at(1).substr(0, 1) == "#")
			{
				shift = 1;
			}
			glm::vec4 settingValue;
			unsigned short v;
			std::stringstream ss, ss1, ss2, ss3;

			string str_r = strparts.at(1).substr(shift, 2);						
			ss << std::hex << str_r;
			ss >> v;
			settingValue.r = (float)v / 255.0;

			str_r = strparts.at(1).substr(shift+2, 2);
			ss1 << std::hex << str_r;
			ss1 >> v;
			settingValue.g = (float)v / 255.0;

			str_r = strparts.at(1).substr(shift + 4, 2);
			ss2 << std::hex << str_r;
			ss2 >> v;
			settingValue.b = (float)v / 255.0;

			str_r = strparts.at(1).substr(shift + 6, 2);
			ss3 << std::hex << str_r;
			ss3 >> v;
			settingValue.a = (float)v / 255.0;

			CSettings::SetColor(CSettings::GetIndex(settingName), settingValue);
		}
	}

	settings_txt.close();
	CRCLogger::Info(requestID, context, "Settings loaded.");

	if (!CRImage::InitPalette("rimagecolor.png"))
	{
		LOG_ERROR__("CRImage palette not initialized.");
		return 0;
	}

	if (strcmp(lpCmdLine, "") == 0) {
		CRCLogger::Info(requestID, context, "No parameters provided, using defaults. Usage: RadarClient lon lat 5 5 1300.");
		g_lon = 37.631424;
		g_lat = 54.792393;
		g_mpph = 5.0f;
		g_mppv = 5.0f;
		g_texsize = 1300;
	}
	else {
		std::string strCmdLine(lpCmdLine);
		std::vector<std::string> v = rcutils::split(strCmdLine, ' ');

		g_lon = std::stof(v[0]);
		g_lat = std::stof(v[1]);
		g_mpph= std::stof(v[2]);
		g_mppv = std::stof(v[3]);
		g_texsize = std::stoi(v[4]);
		
		CRCLogger::Info(requestID, context, (boost::format("Parameters set from command line: lon=%1%, lat=%2%, mpph=%3%, mppv=%4%, texsize=%5%") % g_lon % g_lat % g_mpph % g_mppv % g_texsize).str());
	}
	
	CSettings::SetFloat(FloatMPPh, g_mpph);
	CSettings::SetFloat(FloatMPPv, g_mppv);

	g_vpControl = new CViewPortControl("VP3D");
	g_Minimap = new CMinimap("VPMiniMap");

																	
	application.className = "OpenGL";									// Application Class Name
	application.hInstance = hInstance;									// Application Instance

																		
	ZeroMemory(&window, sizeof(GL_Window));							// Make Sure Memory Is Zeroed
	window.keys = &keys;								// Window Key Structure
	window.init.application = &application;							// Window Application
	window.init.title = "RadarClient DEMO version 0.001";	// Window Title
	window.init.width = 1366;									// Window Width
	window.init.height = 768;									// Window Height
	window.init.bitsPerPixel = 16;									// Bits Per Pixel

	ZeroMemory(&keys, sizeof(Keys));									// Zero keys Structure


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

	g_isProgramLooping = TRUE;											// Program Looping Is Set To TRUE

	CRCLogger::Info(requestID, context, (boost::format("-=point before message loop=-")).str());
	while (g_isProgramLooping)											// Loop Until WM_QUIT Is Received
	{
		if (CreateMainWindow(&window) == TRUE)							// Was Window Creation Successful?
		{
			g_window = &window;
			g_keys = &keys;
				isMessagePumpActive = TRUE;								// Set isMessagePumpActive To TRUE
				
				while (isMessagePumpActive == TRUE)						// While The Message Pump Is Active
				{
					if (!g_Initialized && hasVBO && hasVAO) {
						if (g_Socket && g_Socket->Initialized) 
						{
							Initialize();							
						}						
					}
					// Success Creating Window.  Check For Window Messages
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
			//}															// If (Initialize (...

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

	Deinitialize();
	UnregisterClass(application.className, application.hInstance);		// UnRegister Window Class
	
	return 0;	
}																		// End Of WinMain()												

BOOL Initialize()					// Any GL Init Code & User Initialiazation Goes Here
{
	string context = "Initialize";
	CRCLogger::Info(requestID, context, "Start");

	hasVBO = GLEW_ARB_vertex_buffer_object == TRUE;
	hasVAO = GLEW_ARB_vertex_array_object == TRUE;

	if (!hasVBO || !hasVAO)
	{
		if (!hasVBO)
		{
			CRCLogger::Error(requestID, context, "GLEW_ARB_vertex_buffer_object = FALSE. End (returned false). Perhaps you are launching this program via RDP?");
		}
		if (!hasVAO)
		{
			CRCLogger::Error(requestID, context, "GLEW_ARB_vertex_array_object = FALSE. End (returned false). Perhaps you are launching this program via RDP?");
		}
		return false;
	}

	g_vpControl->Scene = new CScene();	 
	g_vpControl->Scene->Socket = g_Socket;
	g_vpControl->Scene->UI = g_UI;
	g_vpControl->UI = g_UI;
	g_vpControl->Camera = g_vpControl->Scene->Camera;
	
	g_Minimap->Scene = g_vpControl->Scene;	
	g_Minimap->UI = g_UI;
	g_Minimap->Camera = g_vpControl->Scene->Camera;
	
	g_vpControl->Camera->SetAll(0, 0, 0, 0, 0, 1, 0, 1, 0, 
		60.0f, 4.0f/3.0f, 1.0f, 10000.0f,
		0.01, LookAtCallback_);

	g_UI->FillInfoGrid(g_vpControl->Scene);

	g_Initialized = true;

	CRCLogger::Info(requestID, context, "End (returned true).");
	return TRUE;												// Return TRUE (Initialization Successful)
}

int Deinitialize(void)										// Any User DeInitialization Goes Here
{
	string context = "Deinitialize";
	CRCLogger::Info(requestID, context, "Start");
	if (g_vpControl) {
		if (g_vpControl->Scene)
			delete g_vpControl->Scene;
		delete g_vpControl;
	}
	if (g_Minimap)
		delete g_Minimap;

	if (g_UI)
		delete g_UI;
	if (g_hIcon)
		DestroyIcon((HICON)g_hIcon);	

	CRCLogger::Info(requestID, context, "End - return 0");
	return 0;
}

void Update(DWORD milliseconds)								
{

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
		if (g_window)
			SetWindowText(g_window->hWnd, szTitle);				// Set The Title
	}
	g_nFrames++;

}

void LookAtCallback_(double eyex, double eyey, double eyez, double centerx, double centery, double centerz, double upx, double upy, double upz)
{
	gluLookAt(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);
}



