// RemoteGLLauncher.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "RemoteGLLauncher.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

HWND main_hwnd = nullptr;

COLORREF crIdle = RGB(255, 255, 255);
COLORREF crConnected = RGB(0, 255, 0);
COLORREF crBk = crIdle; 

namespace
{
	const int HELLO_PORT = 50013;
	const char * HELLO_PORT_STR = "50013";
}

TCHAR urls[2][255] =
{
	TEXT("localhost"), TEXT("rloc")
};

bool is_connected = false;

void asioTcpClient(TCHAR* host)
{
	try
	{
		boost::asio::io_service aios;

		boost::asio::ip::tcp::resolver resolver(aios);
		boost::asio::ip::tcp::resolver::iterator endpoint = resolver.resolve(
			boost::asio::ip::tcp::resolver::query(to__string(host), HELLO_PORT_STR));
		/*
		boost::asio::ip::tcp::socket socket(aios);
		boost::system::error_code error = boost::asio::error::host_not_found;
		boost::asio::ip::tcp::resolver::iterator end;
		while(error && endpoint != end)
		{
		socket.close();
		socket.connect(*endpoint++, error);
		}
		if(error)
		throw boost::system::system_error(error);
		*/
		boost::asio::ip::tcp::socket socket(aios);
		// open the connection for the specified endpoint, or throws a system_error
		boost::asio::connect(socket, endpoint);

		for (;;)
		{
			std::array<char, 4> buf;
			boost::system::error_code error;
			size_t len = socket.read_some(boost::asio::buffer(buf), error);

			if (error == boost::asio::error::eof)
				break; // Connection closed cleanly by peer
			else if (error)
				throw boost::system::system_error(error);

			std::cout.write(buf.data(), len);
			std::cout << '|';
		}
		std::cout << std::endl;
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_REMOTEGLLAUNCHER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_REMOTEGLLAUNCHER));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_REMOTEGLLAUNCHER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_REMOTEGLLAUNCHER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{	
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
	case WM_CREATE:
	{
		main_hwnd = hWnd;
		auto hwnd1 = RCDialog(IDD_DIALOG1, hWnd, DLGPROC(DialogProc1));
		ShowWindow(hwnd1, SW_SHOW);
		SetWindowPos(hwnd1, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE);

		RECT r, main_client_rect, main_window_rect;
		GetClientRect(main_hwnd, &main_client_rect);
		GetWindowRect(main_hwnd, &main_window_rect);
		GetWindowRect(hwnd1, &r);

		auto width = main_window_rect.right - main_window_rect.left - main_client_rect.right + main_client_rect.left + r.right - r.left;
		auto height = main_window_rect.bottom - main_window_rect.top - main_client_rect.bottom + main_client_rect.top + r.bottom - r.top;

		SetWindowPos(main_hwnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE);
	}
		break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
INT_PTR CALLBACK DialogProc1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
	{
		auto hWndComboBox = GetDlgItem(hDlg, IDC_COMBO_URLS);

		TCHAR A[255];
		int  k = 0;

		memset(&A, 0, sizeof(A));
		for (k = 0; k < 2; k++)
		{
			wcscpy_s(A, sizeof(A) / sizeof(TCHAR), (TCHAR*)urls[k]);

			// Add string to combobox.
			SendMessage(hWndComboBox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)A);
		}

		// Send the CB_SETCURSEL message to display an initial item 
		//  in the selection field  
		SendMessage(hWndComboBox, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
	}
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			//EndDialog(hDlg, LOWORD(wParam));
			PostMessage(main_hwnd, WM_CLOSE, 0, 0);
			return (INT_PTR)TRUE;
		}
		if (LOWORD(wParam) == BTN_CONNECT)
		{
			//EndDialog(hDlg, LOWORD(wParam));
			TCHAR buf[255];
			GetDlgItemText(hDlg, IDC_COMBO_URLS, buf, 255);

			SetDlgItemText(hDlg, IDC_EDIT1, buf);

			if (!is_connected)
			{
				asioTcpClient(buf);

				
			}
			
			is_connected = !is_connected;

			crBk = crBk == crIdle ? crConnected : crIdle;

			auto hctrl = GetDlgItem(hDlg, IDC_EDIT1);
			RECT rect;
			GetClientRect(hctrl, &rect);
			InvalidateRect(hctrl, &rect, TRUE);
			MapWindowPoints(hctrl, hDlg, (POINT *)&rect, 2);
			RedrawWindow(hDlg, &rect, NULL, RDW_ERASE | RDW_INVALIDATE);

		}
		break;
	case WM_CTLCOLOREDIT: 
	{
		HDC hdc = (HDC)wParam;
		HWND hwnd = (HWND)lParam;

		if (GetDlgCtrlID(hwnd) == IDC_EDIT1)
		{
			SetBkColor(hdc, crBk); 
			SetDCBrushColor(hdc, crBk);
		}
		if (GetDlgCtrlID(hwnd) == IDC_COMBO_URLS)
		{
			SetBkColor(hdc, crIdle);
			SetDCBrushColor(hdc, crIdle);
		}
		
		return (LRESULT)GetStockObject(DC_BRUSH);
	}
		break;
	}

	
	return (INT_PTR)FALSE;
}

HWND RCDialog(int ID, HWND hWnd, DLGPROC DlgProc)
{
	auto hInstance = HINSTANCE(GetWindowLong(hWnd, GWL_HINSTANCE));
	return CreateDialogIndirect(hInstance, LPCDLGTEMPLATE(LoadResource(hInstance, FindResource(hInstance, MAKEINTRESOURCE(ID), RT_DIALOG))), hWnd, DlgProc);
}