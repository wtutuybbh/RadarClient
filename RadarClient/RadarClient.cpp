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

#include "CSector.h"

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

		SetWindowPos(hwnd1, HWND_TOP, 0, PANEL_WIDTH + 10, 0, 0, SWP_NOSIZE);

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

DWORD CSector::flight_start_;
std::vector<CFlightPoint> CSector::flight_points_;

void fill_flight_points()
{
	CSector::flight_points_.push_back(CFlightPoint(51.96722, 55.609722, 270, 0));
	CSector::flight_points_.push_back(CFlightPoint(51.930828, 55.609955, 270, 15));
	CSector::flight_points_.push_back(CFlightPoint(51.897614, 55.610138, 270, 30));
	CSector::flight_points_.push_back(CFlightPoint(51.864956, 55.610413, 270, 45));
	CSector::flight_points_.push_back(CFlightPoint(51.842506, 55.610558, 270, 56));
	CSector::flight_points_.push_back(CFlightPoint(51.80706, 55.61084, 270, 70));
	CSector::flight_points_.push_back(CFlightPoint(51.772324, 55.611099, 270, 85));
	CSector::flight_points_.push_back(CFlightPoint(51.735119, 55.61142, 270, 100));
	CSector::flight_points_.push_back(CFlightPoint(51.695652, 55.611721, 270, 115));
	CSector::flight_points_.push_back(CFlightPoint(51.656216, 55.612015, 270, 130));
	CSector::flight_points_.push_back(CFlightPoint(51.5769, 55.612564, 270, 161));
	CSector::flight_points_.push_back(CFlightPoint(51.483433, 55.613113, 270, 195));
	CSector::flight_points_.push_back(CFlightPoint(51.387302, 55.614716, 275, 230));
	CSector::flight_points_.push_back(CFlightPoint(51.305153, 55.621902, 280, 261));
	CSector::flight_points_.push_back(CFlightPoint(51.210354, 55.632385, 281, 295));
	CSector::flight_points_.push_back(CFlightPoint(51.032326, 55.65239, 281, 360));
	CSector::flight_points_.push_back(CFlightPoint(50.846889, 55.67276, 281, 425));
	CSector::flight_points_.push_back(CFlightPoint(50.659538, 55.69313, 280, 490));
	CSector::flight_points_.push_back(CFlightPoint(50.463207, 55.713997, 280, 556));
	CSector::flight_points_.push_back(CFlightPoint(50.264034, 55.734879, 280, 620));
	CSector::flight_points_.push_back(CFlightPoint(50.055874, 55.756302, 280, 685));
	CSector::flight_points_.push_back(CFlightPoint(49.853458, 55.776855, 280, 746));
	CSector::flight_points_.push_back(CFlightPoint(49.64888, 55.797272, 279, 807));
	CSector::flight_points_.push_back(CFlightPoint(49.430069, 55.818741, 279, 870));
	CSector::flight_points_.push_back(CFlightPoint(49.219582, 55.838928, 279, 932));
	CSector::flight_points_.push_back(CFlightPoint(49.000187, 55.859482, 279, 996));
	CSector::flight_points_.push_back(CFlightPoint(48.775864, 55.880096, 279, 1060));
	CSector::flight_points_.push_back(CFlightPoint(48.55442, 55.900021, 279, 1123));
	CSector::flight_points_.push_back(CFlightPoint(48.344391, 55.918453, 278, 1184));
	CSector::flight_points_.push_back(CFlightPoint(48.13435, 55.936569, 278, 1245));
	CSector::flight_points_.push_back(CFlightPoint(47.91404, 55.955185, 278, 1308));
	CSector::flight_points_.push_back(CFlightPoint(47.691566, 55.973511, 278, 1372));
	CSector::flight_points_.push_back(CFlightPoint(47.468262, 55.991501, 278, 1435));
	CSector::flight_points_.push_back(CFlightPoint(47.256317, 56.008209, 277, 1496));
	CSector::flight_points_.push_back(CFlightPoint(47.032814, 56.025433, 277, 1560));
	CSector::flight_points_.push_back(CFlightPoint(46.819351, 56.041447, 277, 1621));
	CSector::flight_points_.push_back(CFlightPoint(46.594391, 56.057972, 277, 1684));
	CSector::flight_points_.push_back(CFlightPoint(46.380367, 56.059021, 266, 1745));
	CSector::flight_points_.push_back(CFlightPoint(46.15451, 56.051407, 266, 1808));
	CSector::flight_points_.push_back(CFlightPoint(45.939247, 56.043777, 266, 1869));
	CSector::flight_points_.push_back(CFlightPoint(45.713779, 56.035355, 266, 1933));
	CSector::flight_points_.push_back(CFlightPoint(45.48872, 56.02655, 265, 1996));
	CSector::flight_points_.push_back(CFlightPoint(45.27174, 56.017612, 265, 2057));
	CSector::flight_points_.push_back(CFlightPoint(45.04686, 56.009171, 269, 2120));
	CSector::flight_points_.push_back(CFlightPoint(44.82988, 56.021988, 277, 2182));
	CSector::flight_points_.push_back(CFlightPoint(44.605076, 56.038513, 277, 2245));
	CSector::flight_points_.push_back(CFlightPoint(44.381351, 56.054581, 277, 2308));
	CSector::flight_points_.push_back(CFlightPoint(44.156883, 56.0704, 277, 2372));
	CSector::flight_points_.push_back(CFlightPoint(43.933296, 56.08567, 276, 2435));
	CSector::flight_points_.push_back(CFlightPoint(43.719921, 56.099869, 276, 2496));
	CSector::flight_points_.push_back(CFlightPoint(43.492626, 56.114594, 276, 2559));
	CSector::flight_points_.push_back(CFlightPoint(43.279476, 56.128006, 276, 2620));
	CSector::flight_points_.push_back(CFlightPoint(43.058769, 56.141441, 276, 2682));
	CSector::flight_points_.push_back(CFlightPoint(42.842388, 56.154194, 276, 2743));
	CSector::flight_points_.push_back(CFlightPoint(42.618214, 56.167007, 275, 2806));
	CSector::flight_points_.push_back(CFlightPoint(42.395908, 56.179321, 275, 2869));
	CSector::flight_points_.push_back(CFlightPoint(42.187252, 56.190491, 275, 2930));
	CSector::flight_points_.push_back(CFlightPoint(41.956871, 56.202377, 275, 2993));
	CSector::flight_points_.push_back(CFlightPoint(41.742886, 56.213058, 275, 3054));
	CSector::flight_points_.push_back(CFlightPoint(41.519478, 56.223789, 274, 3117));
	CSector::flight_points_.push_back(CFlightPoint(41.294613, 56.234207, 274, 3180));
	CSector::flight_points_.push_back(CFlightPoint(41.080627, 56.243729, 274, 3241));
	CSector::flight_points_.push_back(CFlightPoint(40.855324, 56.253304, 274, 3304));
	CSector::flight_points_.push_back(CFlightPoint(40.590652, 56.256729, 260, 3378));
	CSector::flight_points_.push_back(CFlightPoint(40.56839, 56.253864, 255, 3384));
	CSector::flight_points_.push_back(CFlightPoint(40.516464, 56.2439, 247, 3400));
	CSector::flight_points_.push_back(CFlightPoint(40.482388, 56.235477, 245, 3408));
	CSector::flight_points_.push_back(CFlightPoint(40.273132, 56.184402, 246, 3470));
	CSector::flight_points_.push_back(CFlightPoint(40.032463, 56.125195, 246, 3539));
	CSector::flight_points_.push_back(CFlightPoint(39.830433, 56.075089, 245, 3600));
	CSector::flight_points_.push_back(CFlightPoint(39.626022, 56.025284, 249, 3662));
	CSector::flight_points_.push_back(CFlightPoint(39.558605, 56.014435, 257, 3684));
	CSector::flight_points_.push_back(CFlightPoint(39.539795, 56.012352, 260, 3690));
	CSector::flight_points_.push_back(CFlightPoint(39.522068, 56.010818, 262, 3696));
	CSector::flight_points_.push_back(CFlightPoint(39.501011, 56.009399, 263, 3702));
	CSector::flight_points_.push_back(CFlightPoint(39.481514, 56.008255, 264, 3708));
	CSector::flight_points_.push_back(CFlightPoint(39.283531, 55.998962, 265, 3771));
	CSector::flight_points_.push_back(CFlightPoint(39.102402, 55.990517, 265, 3832));
	CSector::flight_points_.push_back(CFlightPoint(38.924561, 55.981705, 264, 3895));
	CSector::flight_points_.push_back(CFlightPoint(38.824139, 55.976646, 264, 3928));
	CSector::flight_points_.push_back(CFlightPoint(38.733673, 55.971989, 264, 3961));
	CSector::flight_points_.push_back(CFlightPoint(38.657726, 55.967972, 264, 3992));
	CSector::flight_points_.push_back(CFlightPoint(38.578751, 55.963844, 264, 4025));
	CSector::flight_points_.push_back(CFlightPoint(38.549393, 55.962261, 264, 4037));
	CSector::flight_points_.push_back(CFlightPoint(38.522812, 55.960876, 264, 4049));
	CSector::flight_points_.push_back(CFlightPoint(38.494461, 55.959373, 264, 4061));
	CSector::flight_points_.push_back(CFlightPoint(38.467297, 55.957901, 264, 4073));
	CSector::flight_points_.push_back(CFlightPoint(38.440662, 55.956528, 264, 4085));
	CSector::flight_points_.push_back(CFlightPoint(38.412323, 55.954998, 264, 4097));
	CSector::flight_points_.push_back(CFlightPoint(38.384983, 55.952637, 258, 4109));
	CSector::flight_points_.push_back(CFlightPoint(38.358681, 55.948837, 253, 4121));
	CSector::flight_points_.push_back(CFlightPoint(38.344448, 55.946411, 253, 4127));
	CSector::flight_points_.push_back(CFlightPoint(38.310783, 55.940708, 253, 4142));
	CSector::flight_points_.push_back(CFlightPoint(38.285023, 55.936295, 252, 4154));
	CSector::flight_points_.push_back(CFlightPoint(38.259804, 55.931946, 252, 4166));
	CSector::flight_points_.push_back(CFlightPoint(38.2355, 55.927689, 252, 4178));
	CSector::flight_points_.push_back(CFlightPoint(38.209534, 55.923157, 252, 4190));
	CSector::flight_points_.push_back(CFlightPoint(38.183411, 55.918594, 252, 4202));
	CSector::flight_points_.push_back(CFlightPoint(38.159122, 55.914406, 252, 4214));
	CSector::flight_points_.push_back(CFlightPoint(38.132797, 55.909882, 253, 4227));
	CSector::flight_points_.push_back(CFlightPoint(38.108494, 55.905762, 253, 4239));
	CSector::flight_points_.push_back(CFlightPoint(38.083336, 55.901604, 253, 4251));
	CSector::flight_points_.push_back(CFlightPoint(38.058971, 55.897568, 253, 4264));
	CSector::flight_points_.push_back(CFlightPoint(38.033504, 55.893311, 253, 4275));
	CSector::flight_points_.push_back(CFlightPoint(38.003208, 55.888184, 253, 4290));
	CSector::flight_points_.push_back(CFlightPoint(37.97982, 55.884247, 253, 4302));
	CSector::flight_points_.push_back(CFlightPoint(37.955849, 55.880219, 253, 4314));
	CSector::flight_points_.push_back(CFlightPoint(37.934959, 55.876648, 253, 4326));
	CSector::flight_points_.push_back(CFlightPoint(37.910072, 55.872437, 253, 4338));
	CSector::flight_points_.push_back(CFlightPoint(37.882061, 55.868877, 263, 4353));
	CSector::flight_points_.push_back(CFlightPoint(37.869701, 55.869156, 276, 4360));
	CSector::flight_points_.push_back(CFlightPoint(37.859573, 55.870365, 284, 4366));
	CSector::flight_points_.push_back(CFlightPoint(37.850647, 55.872227, 290, 4372));
	CSector::flight_points_.push_back(CFlightPoint(37.840775, 55.875488, 304, 4378));
	CSector::flight_points_.push_back(CFlightPoint(37.832878, 55.878887, 307, 4384));
	CSector::flight_points_.push_back(CFlightPoint(37.824265, 55.882462, 306, 4390));
	CSector::flight_points_.push_back(CFlightPoint(37.81786, 55.885078, 305, 4396));
	CSector::flight_points_.push_back(CFlightPoint(37.808846, 55.888802, 306, 4402));
	CSector::flight_points_.push_back(CFlightPoint(37.794052, 55.894867, 306, 4414));
	CSector::flight_points_.push_back(CFlightPoint(37.779663, 55.90081, 306, 4426));
	CSector::flight_points_.push_back(CFlightPoint(37.764256, 55.907272, 307, 4438));
	CSector::flight_points_.push_back(CFlightPoint(37.750938, 55.913681, 314, 4450));
	CSector::flight_points_.push_back(CFlightPoint(37.745361, 55.917068, 319, 4456));
	CSector::flight_points_.push_back(CFlightPoint(37.740013, 55.921154, 325, 4462));
	CSector::flight_points_.push_back(CFlightPoint(37.735458, 55.925446, 330, 4468));
	CSector::flight_points_.push_back(CFlightPoint(37.731941, 55.929489, 335, 4474));
	CSector::flight_points_.push_back(CFlightPoint(37.728634, 55.934143, 339, 4480));
	CSector::flight_points_.push_back(CFlightPoint(37.726135, 55.938446, 342, 4486));
	CSector::flight_points_.push_back(CFlightPoint(37.723789, 55.943035, 344, 4492));
	CSector::flight_points_.push_back(CFlightPoint(37.722057, 55.947052, 346, 4498));
	CSector::flight_points_.push_back(CFlightPoint(37.720394, 55.951447, 348, 4505));
	CSector::flight_points_.push_back(CFlightPoint(37.718227, 55.958084, 350, 4514));
	CSector::flight_points_.push_back(CFlightPoint(37.717094, 55.962166, 351, 4520));
	CSector::flight_points_.push_back(CFlightPoint(37.716148, 55.966404, 354, 4526));
	CSector::flight_points_.push_back(CFlightPoint(37.719059, 55.984131, 8, 4550));
	CSector::flight_points_.push_back(CFlightPoint(37.720612, 55.990425, 7, 4559));
	CSector::flight_points_.push_back(CFlightPoint(37.721558, 55.994892, 6, 4565));
	CSector::flight_points_.push_back(CFlightPoint(37.722073, 55.999317, 3, 4571));
	CSector::flight_points_.push_back(CFlightPoint(37.710739, 56.013931, 302, 4596));
	CSector::flight_points_.push_back(CFlightPoint(37.704163, 56.015625, 292, 4602));
	CSector::flight_points_.push_back(CFlightPoint(37.696323, 56.016308, 269, 4608));
	CSector::flight_points_.push_back(CFlightPoint(37.689114, 56.015423, 254, 4614));
	CSector::flight_points_.push_back(CFlightPoint(37.681648, 56.014072, 251, 4621));
	CSector::flight_points_.push_back(CFlightPoint(37.67062, 56.012009, 252, 4629));
	CSector::flight_points_.push_back(CFlightPoint(37.66259, 56.010628, 253, 4636));
	CSector::flight_points_.push_back(CFlightPoint(37.655472, 56.009537, 255, 4642));
	CSector::flight_points_.push_back(CFlightPoint(37.648067, 56.008484, 255, 4648));
	CSector::flight_points_.push_back(CFlightPoint(37.640491, 56.007385, 255, 4654));
	CSector::flight_points_.push_back(CFlightPoint(37.633083, 56.006332, 255, 4660));
	CSector::flight_points_.push_back(CFlightPoint(37.625843, 56.00528, 255, 4666));
	CSector::flight_points_.push_back(CFlightPoint(37.618301, 56.004204, 255, 4673));
	CSector::flight_points_.push_back(CFlightPoint(37.610695, 56.003036, 255, 4680));
	CSector::flight_points_.push_back(CFlightPoint(37.599506, 56.001366, 255, 4688));
	CSector::flight_points_.push_back(CFlightPoint(37.5928, 56.000381, 255, 4694));
	CSector::flight_points_.push_back(CFlightPoint(37.585392, 55.999329, 255, 4700));
	CSector::flight_points_.push_back(CFlightPoint(37.578049, 55.998199, 255, 4706));
	CSector::flight_points_.push_back(CFlightPoint(37.57058, 55.997131, 255, 4712));
	CSector::flight_points_.push_back(CFlightPoint(37.563629, 55.996105, 255, 4718));
	CSector::flight_points_.push_back(CFlightPoint(37.556679, 55.995079, 255, 4725));
	CSector::flight_points_.push_back(CFlightPoint(37.548866, 55.993916, 255, 4731));
	CSector::flight_points_.push_back(CFlightPoint(37.541199, 55.992783, 254, 4737));
	CSector::flight_points_.push_back(CFlightPoint(37.534042, 55.991684, 255, 4743));
	CSector::flight_points_.push_back(CFlightPoint(37.526634, 55.990612, 255, 4749));
	CSector::flight_points_.push_back(CFlightPoint(37.51931, 55.989532, 255, 4756));
	CSector::flight_points_.push_back(CFlightPoint(37.508907, 55.987976, 255, 4765));
	CSector::flight_points_.push_back(CFlightPoint(37.500584, 55.98674, 255, 4771));
	CSector::flight_points_.push_back(CFlightPoint(37.493076, 55.98563, 255, 4777));
	CSector::flight_points_.push_back(CFlightPoint(37.488358, 55.984932, 255, 4783));
	CSector::flight_points_.push_back(CFlightPoint(37.478569, 55.98349, 255, 4789));
	CSector::flight_points_.push_back(CFlightPoint(37.468704, 55.981979, 254, 4798));
	CSector::flight_points_.push_back(CFlightPoint(37.448147, 55.978912, 255, 4814));
	CSector::flight_points_.push_back(CFlightPoint(37.440407, 55.977768, 255, 4821));
	CSector::flight_points_.push_back(CFlightPoint(37.430004, 55.976257, 254, 4829));
	CSector::flight_points_.push_back(CFlightPoint(37.424667, 55.97546, 255, 4835));
	CSector::flight_points_.push_back(CFlightPoint(37.419037, 55.974598, 255, 4841));
	CSector::flight_points_.push_back(CFlightPoint(37.416386, 55.974201, 255, 4848));
	CSector::flight_points_.push_back(CFlightPoint(37.414478, 55.973911, 255, 4856));
	CSector::flight_points_.push_back(CFlightPoint(37.414005, 55.97385, 255, 4862));
	CSector::flight_points_.push_back(CFlightPoint(37.413464, 55.973808, 255, 4868));
	CSector::flight_points_.push_back(CFlightPoint(37.413017, 55.973839, 255, 4874));
	CSector::flight_points_.push_back(CFlightPoint(37.412567, 55.973934, 255, 4880));
	CSector::flight_points_.push_back(CFlightPoint(37.412247, 55.974144, 255, 4886));
	CSector::flight_points_.push_back(CFlightPoint(37.412094, 55.974422, 255, 4892));
	CSector::flight_points_.push_back(CFlightPoint(37.411922, 55.974773, 255, 4902));
	CSector::flight_points_.push_back(CFlightPoint(37.411816, 55.974991, 334, 4910));
	CSector::flight_points_.push_back(CFlightPoint(37.411686, 55.975307, 334, 4920));
	CSector::flight_points_.push_back(CFlightPoint(37.411472, 55.975773, 334, 4932));
	CSector::flight_points_.push_back(CFlightPoint(37.411324, 55.976063, 334, 4938));
	CSector::flight_points_.push_back(CFlightPoint(37.411213, 55.976292, 334, 4944));
	CSector::flight_points_.push_back(CFlightPoint(37.411022, 55.976715, 334, 4956));
	CSector::flight_points_.push_back(CFlightPoint(37.410873, 55.976982, 334, 4965));
	CSector::flight_points_.push_back(CFlightPoint(37.410656, 55.97752, 334, 4974));
	CSector::flight_points_.push_back(CFlightPoint(37.410591, 55.977768, 334, 4980));
	CSector::flight_points_.push_back(CFlightPoint(37.410549, 55.978031, 334, 4986));
	CSector::flight_points_.push_back(CFlightPoint(37.410446, 55.978283, 334, 4992));
	CSector::flight_points_.push_back(CFlightPoint(37.410336, 55.978542, 334, 4998));
	CSector::flight_points_.push_back(CFlightPoint(37.410194, 55.978821, 334, 5004));
	CSector::flight_points_.push_back(CFlightPoint(37.41003, 55.979187, 334, 5016));
	CSector::flight_points_.push_back(CFlightPoint(37.409904, 55.979473, 334, 5028));
	CSector::flight_points_.push_back(CFlightPoint(37.409779, 55.979702, 334, 5040));

	CSector::flight_time0 = 4505;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{	
	//asioTcpServer();

	fill_flight_points();

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



