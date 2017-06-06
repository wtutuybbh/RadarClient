/*XYZ*/ //code to refactor to change the coordinate system

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

#include "resource1.h"
#include <eh.h>

#define VIEW_PORT_CONTROL_ID     100

#define PANEL_WIDTH 450
#define INFO_HEIGHT 250


#define WM_TOGGLEFULLSCREEN (WM_USER+1)									// Application Define Message For Toggling


#pragma comment(lib, "ComCtl32.lib")

bool g_isProgramLooping = true;	
bool g_InsaneLogMode = false;
bool gl_isProgramLooping = true;
																
float		g_flYRot = 0.0f;									// Rotation
int			g_nFPS = 0, g_nFrames = 0;							// FPS and FPS Counter
DWORD		g_dwLastFPS = 0;									// Last FPS Check Time	
float g_mpph, g_mppv, g_lon, g_lat;
int g_texsize;
std::string g_altFile, g_imgFile, g_datFile;
HWND g_ViewPortControl_hWnd;
HANDLE g_hIcon;
CViewPortControl *g_vpControl = nullptr;
CMinimap *g_Minimap = nullptr;
CUserInterface *g_UI = nullptr;
CRCSocket *g_Socket = nullptr;
#ifdef _DEBUG
DebugWindowInfo g_dwi;
#endif

bool g_Initialized = false;
bool g_AltPressed = false;

GL_Window*	g_window = nullptr;
Keys*		g_keys = nullptr;

std::string requestID = "RadarClient";

//считаем, что VBA и VBO у нас есть, чтобы запустилась первая проверка:
bool hasVBO = true;
bool hasVAO = true;

int g_nCmdShow;

std::thread *gl_thread = nullptr;
std::thread *so_thread = nullptr;

HWND g_SocketHwnd = nullptr;






void TerminateApplication()							// Terminate The Application
{
	gl_isProgramLooping = false;	
	
		
}

void ToggleFullscreen(GL_Window* window)								// Toggle Fullscreen/Windowed
{
	PostMessage(window->hWnd, WM_TOGGLEFULLSCREEN, 0, 0);				// Send A WM_TOGGLEFULLSCREEN Message
}



BOOL ChangeScreenResolution(int width, int height, int bitsPerPixel)	// Change The Screen Resolution
{
	string context = "ChangeScreenResolution";
	LOG_INFO(requestID, context, "Start");
	DEVMODE dmScreenSettings;											// Device Mode
	ZeroMemory(&dmScreenSettings, sizeof(DEVMODE));					// Make Sure Memory Is Cleared
	dmScreenSettings.dmSize = sizeof(DEVMODE);				// Size Of The Devmode Structure
	dmScreenSettings.dmPelsWidth = width;						// Select Screen Width
	dmScreenSettings.dmPelsHeight = height;						// Select Screen Height
	dmScreenSettings.dmBitsPerPel = bitsPerPixel;					// Select Bits Per Pixel
	dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
	{
		LOG_ERROR(requestID, context, "ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL => return false");
		return FALSE;													// Display Change Failed, Return False
	}
	LOG_INFO(requestID, context, "End => return true");
	return TRUE;														// Display Change Was Successful, Return True
}

BOOL CreateMainWindow(GL_Window* window)									// This Code Creates Window
{
	string context = "CreateMainWindow";
	LOG_INFO(requestID, context, "Start");

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
		LOG_ERROR(requestID, context, "window->hWnd == 0 => return false");
		return FALSE;													// If Not Return False
	}

	SetWindowLong(window->hWnd, GWL_STYLE, (GetWindowLong(window->hWnd, GWL_STYLE) | WS_MAXIMIZE));
	ShowWindowAsync(window->hWnd, SW_SHOWMAXIMIZED);

	window->isVisible = TRUE;											// Set isVisible To True

	//ReshapeGL(window->init.width, window->init.height);				// Reshape Our GL Window

	ZeroMemory(window->keys, sizeof(Keys));							// Clear All Keys

	window->lastTickCount = GetTickCount();							// Get Tick Count

	LOG_INFO(requestID, context, "End => return true");
	return TRUE;														// Window Creating Was A Success
																		// Initialization Will Be Done In WM_CREATE
}

BOOL DestroyWindowGL(HWND hWnd, HDC hDC, HGLRC hRC)								// Destroy The OpenGL Window & Release Resources
{
	string context = "DestroyWindowGL";
	LOG_INFO(requestID, context, (boost::format("Start... hwnd=%1%, hDC=%2%, hRC=%3%") % hWnd % hDC % hRC).str().c_str());

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
	LOG_INFO(requestID, context, "End => return true");
	return TRUE;														// Return True
}
void CallUI(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (g_UI)
	{
		g_UI->Wnd_Proc(hWnd, uMsg, wParam, lParam);
	}
}
// Process Window Message Callbacks
LRESULT CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//LOG_INFO("DlgProc", "DlgProc", "hWnd=%x, uMsg=%d, wParam=%d, lParam=%d", hWnd, uMsg, wParam, lParam);

	CallUI(hWnd, uMsg, wParam, lParam);
	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
		POINT p;
		if (GetCursorPos(&p))
		{
			//cursor position now in p.x and p.y
		}
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
void ApplyPositionBounds(float &x, float &y, float &z)
{
	/* don't let the camera go underground */
	if (g_vpControl && g_vpControl->Scene && g_vpControl->Scene->MeshReady())
	{
		auto v = g_vpControl->Scene->Mesh->vertices;
		auto b = g_vpControl->Scene->Mesh->GetBounds();
		auto a = g_vpControl->Scene->Mesh->GetAltitudeDataFile();
		int res = g_vpControl->Scene->Mesh->GetResolution();
		
		if (v && b && a)
		{
			auto dlat = a->DLat();
			auto dlon = a->DLon();

			double lon = a->Lon0() + dlon * ((-x + b[1].x) / (b[1].x - b[0].x)) * res;
			double lat = a->Lat0() + dlat * ((z - b[0].z) / (b[1].z - b[0].z)) * res;

			auto hc = g_vpControl->Scene->Mesh->GetCenterHeight();
			auto h = a->ValueAtLL_max(lon, lat) + 5;

			auto mppv = CSettings::GetFloat(FloatMPPv);

			if (mppv && h - hc > mppv * y )
			{
				y = (h - hc) / mppv;
			}
		}
	}
	g_UI->FillInfoGrid(g_vpControl->Scene);
}
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	string context = "WindowProc";
	

	// Get The Window Context
	GL_Window* window = (GL_Window*)(GetWindowLong(hWnd, GWL_USERDATA));

	CallUI(hWnd, uMsg, wParam, lParam);

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
		LOG_INFO(requestID, context, "WM_CREATE");

		

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

			LOG_INFO(requestID, context, "Icon loaded successfully");
		}
		else
		{
			LOG_WARN(requestID, context, "Icon not loaded");
		}
		
		RECT clientRect;

		GetClientRect(hWnd, &clientRect);

		g_vpControl->Add(hWnd, PANEL_WIDTH, 0, (clientRect.right - clientRect.left) - PANEL_WIDTH, clientRect.bottom - clientRect.top - INFO_HEIGHT);
		g_vpControl->Id = Main;

		if (!g_vpControl->InitGL()) {
			TerminateApplication();
		}
		

		g_Socket = new CRCSocket();
		
		

		
				
		g_UI = new CUserInterface(hWnd, g_vpControl, g_Socket, PANEL_WIDTH);
		g_Minimap->Add(hWnd, 0, 0, PANEL_WIDTH - 30, PANEL_WIDTH - 30);
		g_Minimap->Id = MiniMap;

		if (!g_Minimap->InitGL()) {
			TerminateApplication();
		}		

		//return 0;
		

		HRSRC       hrsrc;
		//HGLOBAL     hglobal;
		auto hInstance = HINSTANCE(GetWindowLong(hWnd, GWL_HINSTANCE));
		hrsrc = FindResource(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), RT_DIALOG);

		//hglobal = ::LoadResource(hInstance, hrsrc);

		HWND hwnd1 = RCDialog(hInstance, IDD_DIALOG1, hWnd, DLGPROC(DlgProc));

		CUserInterface::ToolboxHWND = hwnd1;

		ShowWindow(hwnd1, g_nCmdShow);

		SetWindowPos(hwnd1, HWND_TOP, 0, PANEL_WIDTH + 30, 0, 0, SWP_NOSIZE);

#ifdef _DEBUG
		g_Minimap->dwi = &g_dwi;
		//g_UI->dwi = &g_dwi;
		//g_Socket->dwi = &g_dwi;
#endif // _DEBUG

		wglMakeCurrent(nullptr, nullptr);

		so_thread = new std::thread(SocketMain);

		LOG_INFO(requestID, context, "WM_CREATE: End");
	}
	return 0;														// Return

	case WM_CLOSE: {												// Closing The Window		
		TerminateApplication();								// Terminate The Application
	}
				   return 0;														// Return

	case WM_SIZE: {												// Size Action Has Taken Place
		RECT clientRect;
		GetClientRect(hWnd, &clientRect);
		if (g_vpControl)
			g_vpControl->SetPosition(PANEL_WIDTH, 0, (clientRect.right - clientRect.left) - PANEL_WIDTH, clientRect.bottom - clientRect.top - INFO_HEIGHT);
		if (g_vpControl->Camera)
			g_vpControl->Camera->SetAspect(float((clientRect.right - clientRect.left) - PANEL_WIDTH) / (clientRect.bottom - clientRect.top - INFO_HEIGHT));
		if (g_UI)
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
		LOG_INFO(requestID, context, (boost::format("WM_KEYDOWN: uMsg=%1%, wParam=%2%, lParam=%3%") % hWnd % wParam % lParam).str().c_str());
	}
					 break;															// Break

	case WM_KEYUP: {												// Update Keyboard Buffers For Keys Released
		if ((wParam >= 0) && (wParam <= 255))						// Is Key (wParam) In A Valid Range?
		{
			window->keys->keyDown[wParam] = FALSE;					// Set The Selected Key (wParam) To False
			//return 0;												// Return
		}
		LOG_INFO("msgtest", context, (boost::format("WM_KEYUP: uMsg=%1%, wParam=%2%, lParam=%3%") % hWnd % wParam % lParam).str().c_str());
	}
				   break;															// Break
	case WM_SYSKEYDOWN:
	{
		if (wParam == 18)
		{
			g_AltPressed = true;
		}
		LOG_INFO(requestID, context, (boost::format("WM_SYSKEYDOWN: uMsg=%1%, wParam=%2%, lParam=%3%") % hWnd % wParam % lParam).str().c_str());
		return 0;
	}
	break;
	case WM_SYSKEYUP:
	{
		if (wParam == 18)
		{
			g_AltPressed = false;
		}
		LOG_INFO(requestID, context, (boost::format("WM_SYSKEYUP: uMsg=%1%, wParam=%2%, lParam=%3%") % hWnd % wParam % lParam).str().c_str());
		return 0;
	}
	break;
	case WM_MOUSEWHEEL: {
		double zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		if (g_vpControl->Camera) {
			g_vpControl->Camera->MoveByView(zDelta/10);
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
		MessageBox(HWND_DESKTOP, L"RegisterClassEx Failed!", L"Error", MB_OK | MB_ICONEXCLAMATION);
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
//BOOL				g_isMessagePumpActive;							// Message Pump Active?
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{	
	//asioTcpServer();

	CUserInterface::ToolboxHWND = nullptr;
	g_nCmdShow = nCmdShow;
	MSG msg;

#ifdef _DEBUG
	AllocConsole();
	HWND console = GetConsoleWindow();
	SetWindowPos(console, HWND_TOPMOST, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	ShowWindow(console, SW_NORMAL);

	freopen("CONOUT$", "w", stdout);
#endif
	string context = "WinMain";
	
	float x; 
	float x1; 
	float x2; 
	short direction;
	bool take1;
	bool take2;

	glm::vec3 test;

	x1 = 0; x = 0.5; x2 = 2; direction = 1, take1 = false, take2 = true;
	LOG_INFO__("rcutils::between_on_circle(%f, %f, %f, %d, %d, %d) = %d ", x, x1, x2, direction, take1, take2, rcutils::between_on_circle(x, x1, x2, direction, take1, take2));

	LOG_INFO__("RadarClient started.");

	LOG_INFO__("sizeof(RDR_INITCL) = %d", sizeof(RDR_INITCL));
	LOG_INFO__("sizeof(_sh) = %d", sizeof(_sh));
		
	if (!CSettings::Init())
	{
		LOG_ERROR__("Settings error, program terminated");
		return -2;
	}
	//
	/*write_json("settings.json", CSettings::pt);

	ptree newpt;
	read_json("settings.json", newpt);
	write_json("settings2.json", newpt);*/
	C3DObjectModel::_id = 0;
	C3DObjectModel::_testid = 0;

	g_hIcon = nullptr;

	Application			application;									// Application Structure
	GL_Window			window;											// Window Structure
	Keys				keys;											// Key Structure
	
	DWORD				tickCount;										// Used For The Tick Counter
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	long lBreakAlloc = 0;
	if (lBreakAlloc > 0)
	{
		_CrtSetBreakAlloc(lBreakAlloc);
	}
#endif
	
	CSettings::Load();
	LOG_INFO(requestID, context, "Settings loaded.");

	if (!CSettings::InitPalette())
	{
		LOG_ERROR__("Settings error, program terminated");
		return -2;
	}

	if (strcmp(lpCmdLine, "") == 0) {
		LOG_INFO(requestID, context, "No parameters provided, using defaults. Usage: RadarClient lon lat 5 5 1300.");
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
		LOG_INFO(requestID, context, (boost::format("Parameters set from command line: lon=%1%, lat=%2%, mpph=%3%, mppv=%4%, texsize=%5%") % g_lon % g_lat % g_mpph % g_mppv % g_texsize).str().c_str());
	}
	g_vpControl = new CViewPortControl(L"VP3D");
	g_Minimap = new CMinimap(L"VPMiniMap");															
	application.className = L"OpenGL";									// Application Class Name
	application.hInstance = hInstance;									// Application Instance
																
	ZeroMemory(&window, sizeof(GL_Window));							// Make Sure Memory Is Zeroed
	window.keys = &keys;								// Window Key Structure
	window.init.application = &application;							// Window Application
	window.init.title = L"RadarClient DEMO version 0.001";	// Window Title
	window.init.width = 1366;									// Window Width
	window.init.height = 768;									// Window Height
	window.init.bitsPerPixel = 16;									// Bits Per Pixel
	ZeroMemory(&keys, sizeof(Keys));									// Zero keys Structure


	// Register A Class For Our Window To Use
	if (RegisterWindowClass(&application) == FALSE)					// Did Registering A Class Fail?
	{
		// Failure
		MessageBox(HWND_DESKTOP, L"Error Registering Window Class!", L"Error", MB_OK | MB_ICONEXCLAMATION);
		return -1;														// Terminate Application
	}
	
	char *myargv[1];
	int myargc = 1;
	myargv[0] = strdup("RadarClient");
	
	glutInit(&myargc, myargv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	free(myargv[0]);

	g_isProgramLooping = TRUE;											// Program Looping Is Set To TRUE	

	bool winCreated = CreateMainWindow(&window);
	g_window = &window;
	g_keys = &keys;

	while (g_isProgramLooping)											// Loop Until WM_QUIT Is Received
	{
		if (winCreated == TRUE)							// Was Window Creation Successful?
		{
			
			gl_thread = new std::thread(GLProc);
			
				while (g_window && g_isProgramLooping == TRUE)						// While The Message Pump Is Active
				{
					if (!g_Initialized && hasVBO && hasVAO) {
						Initialize();
					}
					// Success Creating Window.  Check For Window Messages
					if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) != 0)
					{
						// Check For WM_QUIT Message
						if (msg.message != WM_QUIT )						// Is The Message A WM_QUIT Message?
						{
							// If Not, Dispatch The Message
							if (!g_UI || !IsWindow(g_UI->GetSettingsHWND()) || !IsDialogMessage(g_UI->GetSettingsHWND(), &msg)) 
							{
								TranslateMessage(&msg);
								DispatchMessage(&msg);
							}
							else
							{
								int i = 0;
							}
							if (g_UI->GetSettingsHWND())
							{
								int t = 1;
							}
						}
						else											// Otherwise (If Message Is WM_QUIT)
						{
							TerminateApplication();				// Terminate The Message Pump
						}
					}
					else												// If There Are No Messages
					{
						if (g_window->isVisible == FALSE)					// If Window Is Not Visible
						{
							WaitMessage();								// Application Is Minimized Wait For A Message
						}
						else											// If Window Is Visible
						{	
						}
					}
				}		
				if (gl_thread)
				{
					if (gl_thread->joinable())
					{
						gl_thread->join();
					}
					delete gl_thread;
				}
				if (so_thread)
				{
					if (g_SocketHwnd)
					{
						PostMessage(g_SocketHwnd, WM_QUIT, 0, 0);
					}
					if (so_thread->joinable())
					{
						so_thread->join();
					}
					delete so_thread;
				}
				LOG_INFO__("g_window && g_isMessagePumpActive == FALSE");
			//}															// If (Initialize (...

			DestroyWindowGL(g_vpControl->hWnd, g_vpControl->hDC, g_vpControl->hRC);															// Application Is Finished
			DestroyWindowGL(g_Minimap->hWnd, g_Minimap->hDC, g_Minimap->hRC);
			//Deinitialize();											// User Defined DeInitialization
			
												// Destroy The Active Window
		}
		else															// If Window Creation Failed
		{
			// Error Creating Window
			MessageBox(HWND_DESKTOP, L"Error Creating OpenGL Window", L"Error", MB_OK | MB_ICONEXCLAMATION);
			g_isProgramLooping = FALSE;									// Terminate The Loop
		}
	}																	// While (isProgramLooping)

	LOG_INFO__("Before Deinitialize()");
	Deinitialize();
#ifdef _DEBUG
	DestroyWindow(console);
#endif
	UnregisterClass(application.className, application.hInstance);		// UnRegister Window Class
	

	/*g_isProgramLooping = false;*/
	


	return 0;	
}																		// End Of WinMain()												

BOOL Initialize()					// Any GL Init Code & User Initialiazation Goes Here
{
	string context = "Initialize";
	LOG_INFO(requestID, context, "Start");

	hasVBO = GLEW_ARB_vertex_buffer_object == TRUE;
	hasVAO = GLEW_ARB_vertex_array_object == TRUE;

	if (!hasVBO || !hasVAO)
	{
		if (!hasVBO)
		{
			LOG_ERROR(requestID, context, "GLEW_ARB_vertex_buffer_object = FALSE. End (returned false). Perhaps you are launching this program via RDP?");
		}
		if (!hasVAO)
		{
			LOG_ERROR(requestID, context, "GLEW_ARB_vertex_array_object = FALSE. End (returned false). Perhaps you are launching this program via RDP?");
		}
		return false;
	}
	if (g_vpControl) {
		g_vpControl->Scene = new CScene();
		g_vpControl->Scene->Socket = g_Socket;
		g_vpControl->Scene->UI = g_UI;
		g_vpControl->UI = g_UI;
		g_vpControl->Camera = g_vpControl->Scene->Camera;
		g_vpControl->Camera->ApplyPositionBounds(ApplyPositionBounds);
		CUserInterface::Scene = g_vpControl->Scene;
	}
	
	if (g_Minimap) {
		g_Minimap->Scene = g_vpControl->Scene;
		g_Minimap->UI = g_UI;
		g_Minimap->Camera = g_vpControl->Scene->Camera;
	}
	
	if (g_vpControl) {
		g_vpControl->Camera->SetAll(0, 0, 0, 0, 0, 1, 0, 1, 0,
			CSettings::GetFloat(FloatFovy), CSettings::GetFloat(FloatAspect), CSettings::GetFloat(FloatZNear), CSettings::GetFloat(FloatZFar),
			0.01, LookAtCallback_);
	}

	g_UI->FillInfoGrid(g_vpControl->Scene);

	g_Initialized = true;

	LOG_INFO(requestID, context, "End (returned true).");
	return TRUE;												// Return TRUE (Initialization Successful)
}

int Deinitialize(void)										// Any User DeInitialization Goes Here
{
	string context = "Deinitialize";
	LOG_INFO(requestID, context, "Start");
	if (g_vpControl) {
		if (g_vpControl->Scene)
			delete g_vpControl->Scene;
		delete g_vpControl;
	}
	if (g_Minimap)
		delete g_Minimap;

	if (g_UI) {
		delete g_UI;
		g_UI = nullptr;
	}
	if (g_hIcon)
		DestroyIcon((HICON)g_hIcon);	

	if (g_Socket)
	{
		delete g_Socket;
		g_Socket = nullptr;
	}


	LOG_INFO(requestID, context, "End - return 0");
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

		wchar_t szTitle[256] = { 0 };									// Build The Title String
		swprintf(szTitle, L"RadarClient v0.000 - %d FPS", g_nFPS);
		if (g_window)
			SetWindowText(g_window->hWnd, szTitle);				// Set The Title
	}
	g_nFrames++;

}

void GLProc()
{
	std::string context = "GLProc";
	//bool isMessagePumpActive = true;

	//g_isProgramLooping = TRUE;											// Program Looping Is Set To TRUE	
	
	LOG_INFO__("Start");																	//LOG_INFO(requestID, context, (boost::format("-=point before message loop=-")).str());
	while (gl_isProgramLooping)											// Loop Until WM_QUIT Is Received
	{
		//g_isMessagePumpActive = TRUE;								// Set isMessagePumpActive To TRUE

			if (g_window)						// While The Message Pump Is Active
			{
				// Success Creating Window.  Check For Window Messages
					//if (g_InsaneLogMode)
					//{
					//	LOG_INFO__("!!!");
					//}
					if (g_window->isVisible && g_isProgramLooping)					// If Window Is Not Visible
					{						
						Draw();

						if (g_vpControl && g_vpControl->hRC) {
							g_vpControl->MakeCurrent();
							g_vpControl->Draw();
							SwapBuffers(g_vpControl->hDC);					// Swap Buffers (Double Buffering)
						}
						if (g_Minimap && g_Minimap->hRC) {
							g_Minimap->MakeCurrent();
							g_Minimap->Draw();
							SwapBuffers(g_Minimap->hDC);
						}

					}
				
			}														// Loop While isMessagePumpActive == TRUE
																	//}															// If (Initialize (...
			
			//DestroyWindowGL(g_vpControl->hWnd, g_vpControl->hDC, g_vpControl->hRC);															// Application Is Finished
			//DestroyWindowGL(g_Minimap->hWnd, g_Minimap->hDC, g_Minimap->hRC);

	}																	// While (isProgramLooping)

	LOG_INFO__("gl_isProgramLooping is false, GLProc() exit");
	g_isProgramLooping = false;
	return;
	if (g_window)
	{
		g_isProgramLooping = false;
		PostMessage(g_window->hWnd, WM_QUIT, 0, 0);// Send A WM_QUIT Message
	}
	//LOG_INFO__("gl_isProgramLooping is false, GLProc() exit, thread::id=%d", std::this_thread::get_id());
	//Deinitialize();
}
LRESULT CALLBACK SocketProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LOG_INFO("SocketProc", "SocketProc", "hWnd=%d, uMsg=%d, wParam=%d, lParam=%d", hWnd, uMsg, wParam, lParam);
	switch (uMsg)														// Evaluate Window Message
	{
	case WM_CREATE:
		g_Socket->Init(hWnd);
		g_Socket->Connect();
		break;
	case WM_SOCKET: {
		if (WsaAsyncEvents_Log) LOG_INFO("WsaAsyncEvents_Log", "WM_SOCKET", "lParam=%d, wParam=%d");
		if (WSAGETSELECTERROR(lParam))
		{
			LOG_ERROR("SocketProc", "WM_SOCKET", "WSAGETSELECTERROR(lParam)=%d", WSAGETSELECTERROR(lParam));
			break;
		}
		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_READ:
			if (g_Socket && g_isProgramLooping) //g_isMessagePumpActive - workaround because of crash
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
				LOG_INFO("SocketProc", "WM_SOCKET", "FD_CONNECT");
			}
			break;
		}

	}
					break;
	case CM_POSTDATA: {
		unsigned int msg = -1;
		if (g_Socket)
			msg = g_Socket->PostData(wParam, lParam);
		//g_vpControl->MakeCurrent();

		if (g_vpControl && g_vpControl->Scene) {
			if (g_vpControl->Scene && !g_vpControl->Scene->Initialized && g_Socket->s_rdrinit)
			{
				g_vpControl->Scene->Init(g_Socket->s_rdrinit);
			}
			auto tmpbuf = (char *)wParam;
			switch (msg)
			{

			case MSG_RPOINTS:
			{
				g_vpControl->Scene->RefreshSector(g_Socket->info_p, g_Socket->pts, g_Socket->s_rdrinit);
				tmpbuf ? delete tmpbuf : 0;
			}
			break;
			case MSG_OBJTRK:
			{
				g_vpControl->Scene->RefreshTracks(&g_Socket->Tracks);
				tmpbuf ? delete tmpbuf : 0;
			}
			break;
			case MSG_RIMAGE:
			{
				g_vpControl->Scene->RefreshImages(g_Socket->info_i, g_Socket->pixels, g_Socket->s_rdrinit);
				tmpbuf ? delete tmpbuf : 0;
			}
			break;
			case MSG_INIT:
			{
				tmpbuf ? delete tmpbuf : 0;
			}
			break;
			default:
				tmpbuf ? delete tmpbuf : 0;
				break;
			}
		}
		if (g_UI) {
			g_UI->FillGrid(&g_Socket->Tracks);
		}


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
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
void SocketMain()
{
	MSG msg;
	BOOL bRet;
	WNDCLASS wc;
	

	// Register the window class for the main window. 


		wc.style = 0;
		wc.lpfnWndProc = (WNDPROC)SocketProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = GetModuleHandle(NULL);
		wc.hIcon = LoadIcon((HINSTANCE)NULL,
			IDI_APPLICATION);
		wc.hCursor = LoadCursor((HINSTANCE)NULL,
			IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_MENU + 1);
		wc.lpszMenuName = TEXT("SocketMainMenu");
		wc.lpszClassName = TEXT("SocketMainWndClass");

		if (!RegisterClass(&wc))
			return;


	auto hinst = GetModuleHandle(NULL);  // save instance handle 

						// Create the main window. 

	auto hwndMain = CreateWindow(TEXT("SocketMainWndClass"), TEXT("Sample"),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT, (HWND)NULL,
		(HMENU)NULL, hinst, (LPVOID)NULL);
	
	g_SocketHwnd = hwndMain;
	// If the main window cannot be created, terminate 
	// the application. 

	if (!hwndMain)
		return;

	// Show the window and paint its contents. 

	//ShowWindow(hwndMain, true);
	//UpdateWindow(hwndMain);

	// Start the message loop. 

	//
	if (SocketWindow_Log) LOG_INFO("SocketWindow_Log", "SocketMain", "START");
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0 && gl_isProgramLooping)
	{
		if (SocketWindow_Log) LOG_INFO("SocketWindow_Log", "SocketMain", "lParam=%d, wParam=%d", msg.lParam, msg.wParam);
		if (bRet == -1)
		{
			// handle the error and possibly exit
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	if (SocketWindow_Log) LOG_INFO("SocketWindow_Log", "SocketMain", "END");

	// Return the exit code to the system. 

	return;
}

void LookAtCallback_(double eyex, double eyey, double eyez, double centerx, double centery, double centerz, double upx, double upy, double upz)
{
	gluLookAt(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);
}



