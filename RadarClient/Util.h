#pragma once
#include "stdafx.h"

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

class CColorRGBA {
public:
	float r;
	float g;
	float b;
	float a;
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

//https://helloacm.com/cc-function-to-compute-the-bilinear-interpolation/

inline float BilinearInterpolation(float q11, float q12, float q21, float q22, float x1, float x2, float y1, float y2, float x, float y)
{
	if (x1 == x2)
	{
		return q11 * (y - y1) / ( + q22
	}

	float x2x1, y2y1, x2x, y2y, yy1, xx1;
	x2x1 = x2 - x1;
	y2y1 = y2 - y1;
	x2x = x2 - x;
	y2y = y2 - y;
	yy1 = y - y1;
	xx1 = x - x1;
	return 1.0f / (x2x1 * y2y1) * (
		q11 * x2x * y2y +
		q21 * xx1 * y2y +
		q12 * x2x * yy1 +
		q22 * xx1 * yy1
		);
}

typedef int(_cdecl * GDPALTITUDEMAP)(const char *, double *, int *, short *);
typedef int(_cdecl * GDPALTITUDEMAP_SIZES)(const char *, double *, int *);

inline double zero_if_negative(double x) {
	return x < 0 ? 0 : x;
}

static inline void ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(),
		std::not1(std::ptr_fun<int, int>(std::isspace))));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(),
		std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
	ltrim(s);
	rtrim(s);
}

// trim from start (copying)
static inline std::string ltrimmed(std::string s) {
	ltrim(s);
	return s;
}

// trim from end (copying)
static inline std::string rtrimmed(std::string s) {
	rtrim(s);
	return s;
}

// trim from both ends (copying)
static inline std::string trimmed(std::string s) {
	trim(s);
	return s;
}

class outbuf : public std::streambuf {
public:
	outbuf() {
		setp(nullptr, nullptr);
	}

	virtual int_type overflow(int_type c = traits_type::eof()) {
		return fputc(c, stdout) == EOF ? traits_type::eof() : c;
	}
};

std::string num2str(double num, std::streamsize precision);

std::string mat4row2str(glm::mat4 m, int row, std::streamsize precision);

std::string format(const char *fmt, ...);
#define formatc(fmt, ...) format(fmt, __VA_ARGS__).c_str()
#define formatmsg(fmt, ...) (LPARAM)format(fmt, __VA_ARGS__).c_str()


std::string vformat(const char *fmt, va_list ap);
//for copy-paste:
//CRCLogger::Info(requestID, context, (boost::format("%1%") % 1).str());


std::string GetLastErrorAsString();

std::wstring HTMLColorFormat(int r, int g, int b, int a);

HWND RCDialog(HINSTANCE hInstance, int ID, HWND hWnd, DLGPROC DlgProc);