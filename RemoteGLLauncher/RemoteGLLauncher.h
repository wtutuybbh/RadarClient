#pragma once

#include "resource.h"



HWND RCDialog(int ID, HWND hWnd, DLGPROC DlgProc);
INT_PTR CALLBACK DialogProc1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

