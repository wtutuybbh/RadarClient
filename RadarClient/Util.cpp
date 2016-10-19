#define WIN32_LEAN_AND_MEAN

#include "Util.h"
#include "glm/glm.hpp"
#include "glm/gtx/norm.hpp"

#include <string>
#include <sstream>
#include <vector>

#include <GL/glew.h>
#include <GL/freeglut.h>

namespace cnvrt {
	double latdg2m(double dg, double lat) {
		if (lat < 5) return dg * 110579;
		if (lat < 10) return dg * 110596;
		if (lat < 15) return dg * 110629;
		if (lat < 20) return dg * 110676;
		if (lat < 25) return dg * 110739;
		if (lat < 30) return dg * 110814;
		if (lat < 35) return dg * 110898;
		if (lat < 40) return dg * 110989;
		if (lat < 45) return dg * 111085;
		if (lat < 50) return dg * 111182;
		if (lat < 55) return dg * 111278;
		if (lat < 60) return dg * 111370;
		if (lat < 65) return dg * 111455;
		if (lat < 70) return dg * 111531;
		if (lat < 75) return dg * 111594;
		if (lat < 80) return dg * 111643;
		if (lat < 85) return dg * 111677;
		return dg * 111694;
	}

	double londg2m(double dg, double lat) {
		if (lat < 1) return dg * 111321;
		if (lat < 2) return dg * 111305;
		if (lat < 3) return dg * 111254;
		if (lat < 4) return dg * 111170;
		if (lat < 5) return dg * 111052;
		if (lat < 6) return dg * 110901;
		if (lat < 7) return dg * 110716;
		if (lat < 8) return dg * 110497;
		if (lat < 9) return dg * 110245;
		if (lat < 10) return dg * 109960;
		if (lat < 11) return dg * 109641;
		if (lat < 12) return dg * 109289;
		if (lat < 13) return dg * 108904;
		if (lat < 14) return dg * 108487;
		if (lat < 15) return dg * 108036;
		if (lat < 16) return dg * 107552;
		if (lat < 17) return dg * 107036;
		if (lat < 18) return dg * 106488;
		if (lat < 19) return dg * 105907;
		if (lat < 20) return dg * 105294;
		if (lat < 21) return dg * 104649;
		if (lat < 22) return dg * 103972;
		if (lat < 23) return dg * 103264;
		if (lat < 24) return dg * 102524;
		if (lat < 25) return dg * 101753;
		if (lat < 26) return dg * 100952;
		if (lat < 27) return dg * 100119;
		if (lat < 28) return dg * 99257;
		if (lat < 29) return dg * 98364;
		if (lat < 30) return dg * 97441;
		if (lat < 31) return dg * 96488;
		if (lat < 32) return dg * 95506;
		if (lat < 33) return dg * 94495;
		if (lat < 34) return dg * 93455;
		if (lat < 35) return dg * 92386;
		if (lat < 36) return dg * 91290;
		if (lat < 37) return dg * 90165;
		if (lat < 38) return dg * 89013;
		if (lat < 39) return dg * 87834;
		if (lat < 40) return dg * 86628;
		if (lat < 41) return dg * 85395;
		if (lat < 42) return dg * 84137;
		if (lat < 43) return dg * 82852;
		if (lat < 44) return dg * 81542;
		if (lat < 45) return dg * 80208;
		if (lat < 46) return dg * 78848;
		if (lat < 47) return dg * 77465;
		if (lat < 48) return dg * 76057;
		if (lat < 49) return dg * 74627;
		if (lat < 50) return dg * 73173;
		if (lat < 51) return dg * 71697;
		if (lat < 52) return dg * 70199;
		if (lat < 53) return dg * 68679;
		if (lat < 54) return dg * 67138;
		if (lat < 55) return dg * 65577;
		if (lat < 56) return dg * 63995;
		if (lat < 57) return dg * 62394;
		if (lat < 58) return dg * 60773;
		if (lat < 59) return dg * 59134;
		if (lat < 60) return dg * 57476;
		if (lat < 61) return dg * 55801;
		if (lat < 62) return dg * 54108;
		if (lat < 63) return dg * 52399;
		if (lat < 64) return dg * 50674;
		if (lat < 65) return dg * 48933;
		if (lat < 66) return dg * 47176;
		if (lat < 67) return dg * 45405;
		if (lat < 68) return dg * 43621;
		if (lat < 69) return dg * 41822;
		if (lat < 70) return dg * 40011;
		if (lat < 71) return dg * 38187;
		if (lat < 72) return dg * 36352;
		if (lat < 73) return dg * 34505;
		if (lat < 74) return dg * 32647;
		if (lat < 75) return dg * 30780;
		if (lat < 76) return dg * 28902;
		if (lat < 77) return dg * 27016;
		if (lat < 78) return dg * 25122;
		if (lat < 79) return dg * 23219;
		if (lat < 80) return dg * 21310;
		if (lat < 81) return dg * 19394;
		if (lat < 82) return dg * 17472;
		if (lat < 83) return dg * 15544;
		if (lat < 84) return dg * 13612;
		if (lat < 85) return dg * 11675;
		if (lat < 86) return dg * 9735;
		if (lat < 87) return dg * 7791;
		if (lat < 88) return dg * 5846;
		if (lat < 89) return dg * 3898;
		if (lat < 90) return dg * 1949;
		return 0;
	}
	std::string float2str(float number)
	{
		std::ostringstream buff;
		buff << number;
		return buff.str();
	}
}
namespace rcutils {
	// checks the value and sets new min or max if value is out of the (min, max) range
	void takeminmax(float value, float *min, float *max) {
		if (*max < value) *max = value;
		if (*min > value) *min = value;
	}
	// splits string using delimiter (v1)
	std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
		std::stringstream ss(s);
		std::string item;
		while (std::getline(ss, item, delim)) {
			elems.push_back(item);
		}
		return elems;
	}

	// splits string using delimiter (v2)
	std::vector<std::string> split(const std::string &s, char delim) {
		std::vector<std::string> elems;
		split(s, delim, elems);
		return elems;
	}

	// checks if x is between x1 and x2, assuming that they are points on circle (with period M_PIx2), taking direction into account
	bool between_on_circle(float x, float x1, float x2, short direction, bool take1, bool take2)
	{
		if (x>=M_PIx2)		
			x = x - round(x / M_PIx2)*M_PIx2;		

		if (x1 == x2)
		{
			if (x == x1)
				return true;
			return false;
		}
		if (take1 && x == x1 || take2 && x == x2)
			return true;
			
		if (direction>0)
		{
			if (x2 > x1 && x > x1 && x < x2)
				return true;
			if (x2 < x1 && (x < x2 || x > x1))
				return true;			
		}
		if (direction<0)
		{
			if (x1 > x2 && x > x2 && x < x1)
				return true;
			if (x1 < x2 && (x < x1 || x > x2))
				return true;
		}
		return false;
	}
}

//helper function to draw bitmap string (3d version)
void BitmapString(float x, float y, float z, std::string s)
{
	glRasterPos3f(x, y, z);
	for (int n = 0; n<s.size(); ++n) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, s[n]);
	}
}

//helper function to draw bitmap string (3d version)
void BitmapString2D(float x, float y, std::string s)
{
	glRasterPos2f(x, y);
	for (int n = 0; n<s.size(); ++n) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, s[n]);
	}
}

// helper function to get file length in bytes
unsigned long getFileLength(std::ifstream& file)
{
	if (!file.good()) return 0;

	unsigned long pos = file.tellg();
	file.seekg(0, std::ios::end);
	unsigned long len = file.tellg();
	file.seekg(std::ios::beg);

	return len;
}
#ifdef _DEBUG
HWND OpenDebugWindow(HINSTANCE hInst, int nShowCmd, HWND mainWindow, DebugWindowInfo *dwi) {
	WNDCLASSEX windowclassforwindow2;
	ZeroMemory(&windowclassforwindow2, sizeof(WNDCLASSEX));
	windowclassforwindow2.cbClsExtra = NULL;
	windowclassforwindow2.cbSize = sizeof(WNDCLASSEX);
	windowclassforwindow2.cbWndExtra = NULL;
	windowclassforwindow2.hbrBackground = (HBRUSH)COLOR_WINDOW;
	windowclassforwindow2.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowclassforwindow2.hIcon = NULL;
	windowclassforwindow2.hIconSm = NULL;
	windowclassforwindow2.hInstance = hInst;
	windowclassforwindow2.lpfnWndProc = (WNDPROC)DebugWndProc;
	windowclassforwindow2.lpszClassName = _T("Debug window");
	windowclassforwindow2.lpszMenuName = NULL;
	windowclassforwindow2.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassEx(&windowclassforwindow2))
	{
		int nResult = GetLastError();
		MessageBox(NULL,
			_T("Window class creation failed for window 2"),
			_T("Window Class Failed"),
			MB_ICONERROR);
	}

	HWND handleforwindow2 = CreateWindowEx(NULL,
		windowclassforwindow2.lpszClassName,
		_T("Debug Window"),
		WS_OVERLAPPEDWINDOW,
		200,
		150,
		640,
		480,
		NULL,
		NULL,
		hInst, dwi);

	if (!handleforwindow2)
	{
		int nResult = GetLastError();

		MessageBox(NULL,
			_T("Window creation failed"),
			_T("Window Creation Failed"),
			MB_ICONERROR);
	}

	ShowWindow(handleforwindow2, nShowCmd);
	//SetParent(handleforwindow2, mainWindow);
	SetWindowLong(handleforwindow2, GWL_HWNDPARENT, (long)mainWindow);

	//SetWindowLong(handleforwindow2, GWL_USERDATA, (long)dwi);

	dwi->hWnd = handleforwindow2;

	return handleforwindow2;
}
#define CM_POSTDATA (WM_USER + 2)
LRESULT CALLBACK DebugWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_NCCREATE)
	{
		// get the pointer to the window from
		// lpCreateParams which was set in CreateWindow
		SetWindowLong(hWnd, GWL_USERDATA, (long)((LPCREATESTRUCT(lParam))->lpCreateParams));
	}
	DebugWindowInfo *dwi = (DebugWindowInfo*)GetWindowLong(hWnd, GWL_USERDATA);
	switch (msg)
	{
	case WM_DESTROY: {
		/*MessageBox(NULL,
			_T("Window 2 closed"),
			_T("Message"),
			MB_ICONINFORMATION);*/
		return 0;
	}
					 break;
	case WM_CREATE: {
		if (dwi) {
			dwi->Edit_hWnd = CreateWindowEx(NULL,
				_T("EDIT"),
				NULL,
				WS_CHILD | WS_VISIBLE | WS_VSCROLL |
				ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
				0,
				0,
				0,
				0,
				hWnd,
				(HMENU)dwi->DebugEdit_ID,
				GetModuleHandle(NULL),
				NULL);
			return 0;
		}
	}
		break;
	case WM_SIZE:
		if (dwi) {
			MoveWindow(dwi->Edit_hWnd,
				0, 0,                  // starting x- and y-coordinates 
				LOWORD(lParam),        // width of client area 
				HIWORD(lParam),        // height of client area 
				TRUE);                 // repaint window 
			return 0;
		}
	case CM_POSTDATA: {
		DebugMessage(dwi, "Why we are here?");
	}
					  break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}
void DebugMessage(DebugWindowInfo *dwi, std::string Text)
{
	if (dwi) {
		Text = Text + "\r\n";
		int idx = GetWindowTextLength(dwi->Edit_hWnd);
		SendMessage(dwi->Edit_hWnd, EM_SETSEL, (WPARAM)idx, (LPARAM)idx);
		SendMessage(dwi->Edit_hWnd, EM_REPLACESEL, 0, (LPARAM)Text.c_str());
	}
}
#endif
float MinimumDistance(glm::vec3 v, glm::vec3 w, glm::vec3 p)
{
	// Return minimum distance between line segment vw and point p
	const float l2 = glm::length2(w - v);  // i.e. |w-v|^2 -  avoid a sqrt
	if (l2 == 0.0) return glm::distance(p, v);   // v == w case
												 // Consider the line extending the segment, parameterized as v + t (w - v).
												 // We find projection of point p onto the line. 
												 // It falls where t = [(p-v) . (w-v)] / |w-v|^2
												 // We clamp t from [0,1] to handle points outside the segment vw.
	const float t = max(0, min(1, dot(p - v, w - v) / l2));
	const glm::vec3 projection = v + t * (w - v);  // Projection falls on the segment
	return glm::distance(p, projection);
}

