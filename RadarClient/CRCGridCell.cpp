#include "stdafx.h"
#include "CRCGridCell.h"
#include "ZeeGrid.h"

CRCGridCell::CRCGridCell(HWND hWnd, int index)
{
	this->hWnd = hWnd;
	this->index = index;
}

CRCGridCell::CRCGridCell(HWND hWnd, int ncols, int r, int c)
{
	this->hWnd = hWnd;
	this->index = ncols * r + c;
}

CRCGridCell& CRCGridCell::operator=(const std::string value)
{
	SendMessage(hWnd, ZGM_SETCELLTEXT, index, (LPARAM)value.c_str());
	return *this;
}
