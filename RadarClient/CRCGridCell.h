#pragma once
#include "stdafx.h"

#define GRIDCELL(...) *(&(CRCGridCell::CRCGridCell(__VA_ARGS__))) 

class CRCGridCell
{	
	HWND hWnd;
	int index;
public:
	CRCGridCell(HWND hWnd, int index);
	CRCGridCell(HWND hWnd, int ncols, int r, int c);	
	CRCGridCell& operator=(const std::string value);
	CRCGridCell& operator=(const int value);
	CRCGridCell& operator=(const double value);

};
