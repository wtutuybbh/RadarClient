#pragma once
#define _USE_MATH_DEFINES 
#include <math.h>
#include <string>
#include <sstream>
#include <vector>
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"

#include <fstream>
#include <iostream>

#include <Windows.h>
#include <windowsx.h>
#include <WinBase.h>

#include <tchar.h>

#define M_PIx2 6.28318530717959


namespace cnvrt {
	inline double dg2rad(double dg) {
		return dg*M_PI / 180.0;
	}
	inline double rad2dg(double rad) {
		return rad * 180 / M_PI;
	}
	double latdg2m(double dg, double lat);
	double londg2m(double dg, double lat);
	

	std::string float2str(float number);

}
namespace rcutils {
	void takeminmax(float value, float *min, float *max);
	
	std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
	std::vector<std::string> split(const std::string &s, char delim);

	bool between_on_circle(float x, float x1, float x2, short direction, bool take1, bool take2);
}

void BitmapString(float x, float y, float z, std::string s);
void BitmapString2D(float x, float y, std::string s);

/*class CVert														// Vertex Class
{
public:
	float x;													// X Component
	float y;													// Y Component
	float z;													// Z Component
};
typedef CVert CVec;												// The Definitions Are Synonymous


class CTexCoord													// Texture Coordinate Class
{
public:
	float u;													// U Component
	float v;													// V Component
};*/

class CColorRGBA {
public:
	float r;
	float g;
	float b;
	float a;
};

struct VBOData {
	glm::vec4 vert;
	glm::vec3 norm;
	glm::vec4 color;
	glm::vec2 texcoord;
};
struct VBOData_ext {
	glm::vec4 vert;
	glm::vec3 norm;
	glm::vec4 color;
	glm::vec2 texcoord;
	float rshadowheight;
};

template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

#ifdef _DEBUG
typedef struct {
	HWND hWnd;
	HWND Edit_hWnd;
	int DebugEdit_ID;
} DebugWindowInfo;
LRESULT CALLBACK DebugWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
HWND OpenDebugWindow(HINSTANCE hInst, int nShowCmd, HWND mainWindow, DebugWindowInfo *dwi);
void DebugMessage(DebugWindowInfo *dwi, std::string Text);
#endif

struct ZGNOTIFYSTRUCT {
	NMHDR header;
	BOOL CONTROL;
	BOOL SHIFT;
	BOOL ALT;
	int vkey;
};

float MinimumDistance(glm::vec3 v, glm::vec3 w, glm::vec3 p);

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#define CRCPOINT_CONSTRUCTOR_USES_RADIANS