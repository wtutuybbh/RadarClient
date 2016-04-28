#pragma once
#include "CUserInterface.h"
#include "CRCSocket.h"
#include "ViewPortControl.h"
#include "CCamera.h"
#include "CScene.h"
#include <CommCtrl.h>
#include <string>

#include "ZeeGrid.h"

/*CUserInterface::CUserInterface()
{
}*/
LRESULT Button1_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	return 0;
}

//in this method ID is retrieved as LOWORD(wParam)
LRESULT CUserInterface::Wnd_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CUserInterface *ui = this;
	if (!ui) 
		return NULL;
	for (ElementsMap::iterator it = Elements.begin(); it != Elements.end(); ++it) {
		if (LOWORD(wParam) == it->second->ID) {
			if (it->second->Action)
				CALL_MEMBER_FN(*ui, it->second->Action)(hwnd, uMsg, wParam, lParam);
		}
	}
	return LRESULT();
}
//in this method ID is retrieved as GetDlgCtrlID((HWND)lParam)
LRESULT CUserInterface::Wnd_Proc2(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CUserInterface *ui = this;
	if (!ui)
		return NULL;
	for (ElementsMap::iterator it = Elements.begin(); it != Elements.end(); ++it) {
		if (GetDlgCtrlID((HWND)lParam) == it->second->ID) {
			if (it->second->Action)
				CALL_MEMBER_FN(*ui, it->second->Action)(hwnd, uMsg, wParam, lParam);
		}
	}
	return LRESULT();
}

LRESULT CUserInterface::Button_Test(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return LRESULT();
}

LRESULT CUserInterface::Grid(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return LRESULT();
}

void CUserInterface::SetChecked(int id, bool checked)
{
	HWND hWnd = GetDlgItem(ParentHWND, id);
	SendMessage(hWnd, BM_SETCHECK, checked, 0);
}

glm::vec3 CUserInterface::GetDirection()
{
	float a = glm::radians((GetTrackbarValue_Turn() - 50) * 3.6f);
	float e = glm::radians(1.8 * GetTrackbarValue_VTilt()); //here elevation counts from top-up direction
	return glm::vec3(-sin(e)*sin(a), cos(e), sin(e)*cos(a));
}


int CUserInterface::InsertElement(DWORD xStyle, LPCSTR _class, LPCSTR text, DWORD style, int x, int y, int width, int height, UIWndProc action)
{
	Elements.insert({ CurrentID, new InterfaceElement{ CurrentID, xStyle, _class, text, style, x, y, width, height, NULL, action } });
	CurrentID++;
	return CurrentID-1;
}

LRESULT CUserInterface::Button_Connect(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (Socket && !Socket->IsConnected)
		Socket->Connect();
	if (Socket && Socket->IsConnected)
		Socket->Close();

	return LRESULT();
}

LRESULT CUserInterface::Checkbox_ObjOptions(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int ButtonID = LOWORD(wParam);
	HWND hWnd = GetDlgItem(hwnd, ButtonID);
	int Checked = !Button_GetCheck(hWnd);
	SendMessage(hWnd, BM_SETCHECK, Checked, 0);

	if (ButtonID == ObjOptions_ID[0]) { // points
		if (VPControl)
			VPControl->DisplayPoints = Checked;
	}
	if (ButtonID == ObjOptions_ID[1]) { // series
		if (VPControl)
			VPControl->DisplaySeries = Checked;
	}
	if (ButtonID == ObjOptions_ID[2]) { // RLIs
		if (VPControl)
			VPControl->DisplayRLIs = Checked;
	}

	return LRESULT();
}

LRESULT CUserInterface::Checkbox_MapOptions(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int ButtonID = LOWORD(wParam);
	HWND hWnd = GetDlgItem(hwnd, ButtonID);
	int Checked = !Button_GetCheck(hWnd);
	SendMessage(hWnd, BM_SETCHECK, Checked, 0);

	if (ButtonID == MapOptions_ID[0]) { // points
		if (VPControl)
			VPControl->DisplayMap = Checked;
	}
	if (ButtonID == MapOptions_ID[1]) { // series
		if (VPControl)
			VPControl->DisplayLandscape = Checked;
	}
	return LRESULT();
}

LRESULT CUserInterface::RadioGroup_CameraPosition(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int ButtonID = LOWORD(wParam);
	HWND hWnd = GetDlgItem(hwnd, ButtonID);

	if (ButtonID == CameraPosition_ID[0]) { // FROM_RADAR
		this->VPControl->Camera->Position = glm::vec3(0, this->VPControl->Scene->y0, 0);
	}
	if (ButtonID == CameraPosition_ID[1]) { // FROM_5KM_ABOVE_RADAR
		this->VPControl->Camera->Position = glm::vec3(0, this->VPControl->Scene->y0 + 5000.0f / this->VPControl->Scene->mppv, 0);
	}
	if (ButtonID == CameraPosition_ID[2]) { // FROM_5KM_ABOVE_RADAR_5KM_NORTH
		this->VPControl->Camera->Position = glm::vec3(0, this->VPControl->Scene->y0 + 5000.0f / this->VPControl->Scene->mppv, 5000.0f / this->VPControl->Scene->mpph);
	}

	return LRESULT();
}

LRESULT CUserInterface::Trackbar_CameraDirection_VTilt(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int ID = GetDlgCtrlID((HWND)lParam);
	
	int val = SendMessage(GetDlgItem(hwnd, ID), TBM_GETPOS, 0, 0);

	SetDlgItemText(ParentHWND, CameraDirectionValue_ID[0], std::to_string(val).c_str());

	VPControl->Camera->Direction = GetDirection();

	return LRESULT();
}
LRESULT CUserInterface::Trackbar_CameraDirection_Turn(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int ID = GetDlgCtrlID((HWND)lParam);

	int val = SendMessage(GetDlgItem(hwnd, ID), TBM_GETPOS, 0, 0);

	SetDlgItemText(ParentHWND, CameraDirectionValue_ID[1], std::to_string(val).c_str());

	VPControl->Camera->Direction = GetDirection();

	return LRESULT();
}
//TRACKBAR_CLASS
CUserInterface::CUserInterface(HWND parentHWND, ViewPortControl *vpControl, CRCSocket *socket, int panelWidth)
{

	this->ParentHWND = parentHWND;
	this->VPControl = vpControl;
	this->Socket = socket;
	this->CurrentID = 1;

	/*int Column1X, Column2X;
	int VStep, VStepGrp;
	int MinimapSize, PanelWidth;*/

	Column1X = 10;
	PanelWidth = panelWidth;
	Column2X = panelWidth / 2;
	VStep = 22;
	VStepGrp = 30;
	MinimapSize = PanelWidth - VStepGrp;
	ButtonHeight = 20, ControlWidth = 110, ControlWidthL = 180, ControlWidthXL = 270;

	int CurrentY = 0;

	hgridmod = LoadLibrary("ZeeGrid.dll");
	if (!hgridmod)
	{
		MessageBox(parentHWND, "Unable to load ZeeGrid.DLL", "Error", MB_OK);
		PostQuitMessage(0);
	}

	//new view port control should be here

	CameraDirection_ID[0] = InsertElement(NULL, TRACKBAR_CLASS, TEXT_LABEL_CAMERA_POSITION, WS_VISIBLE | WS_CHILD | TBS_VERT | TBS_RIGHT, MinimapSize, CurrentY, VStepGrp, MinimapSize, &CUserInterface::Trackbar_CameraDirection_VTilt);
	CurrentY += MinimapSize;
	CameraDirection_ID[1] = InsertElement(NULL, TRACKBAR_CLASS, TEXT_LABEL_CAMERA_POSITION, WS_VISIBLE | WS_CHILD | TBS_HORZ | TBS_BOTTOM, 0, CurrentY, MinimapSize, VStepGrp, &CUserInterface::Trackbar_CameraDirection_Turn);

	CurrentY += VStepGrp + VStep;

	Button_Connect_ID = InsertElement(NULL, _T("BUTTON"), TEXT_BUTTON_CONNECT, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, Column1X, CurrentY, ControlWidthL, ButtonHeight, &CUserInterface::Button_Connect);
	IsConnected_ID = InsertElement(NULL, _T("STATIC"), TEXT_LABEL_NOT_CONNECTED, WS_VISIBLE | WS_CHILD, Column2X, CurrentY, ControlWidthL, ButtonHeight, NULL);
	
	CurrentY += VStepGrp;

	InsertElement(NULL, _T("STATIC"), TEXT_LABEL_SHOW, WS_VISIBLE | WS_CHILD, Column1X, CurrentY, ControlWidth, ButtonHeight, NULL);
	InsertElement(NULL, _T("STATIC"), TEXT_LABEL_VIEW, WS_VISIBLE | WS_CHILD, Column2X, CurrentY, ControlWidth, ButtonHeight, NULL);
	CurrentY += VStep;
	ObjOptions_ID[0] = InsertElement(NULL, _T("BUTTON"), TEXT_CHECKBOX_POINTS, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX, Column1X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::Checkbox_ObjOptions);
	MapOptions_ID[0] = InsertElement(NULL, _T("BUTTON"), TEXT_CHECKBOX_LANDSCAPE, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX, Column2X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::Checkbox_MapOptions);
	CurrentY += VStep;
	ObjOptions_ID[1] = InsertElement(NULL, _T("BUTTON"), TEXT_CHECKBOX_SERIES, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX, Column1X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::Checkbox_ObjOptions);
	MapOptions_ID[1] = InsertElement(NULL, _T("BUTTON"), TEXT_CHECKBOX_MAP, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX, Column2X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::Checkbox_MapOptions);
	CurrentY += VStep;
	ObjOptions_ID[2] = InsertElement(NULL, _T("BUTTON"), TEXT_CHECKBOX_RLI, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX, Column1X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::Checkbox_ObjOptions);
	
	CurrentY += VStepGrp;

	InsertElement(NULL, _T("STATIC"), TEXT_LABEL_CAMERA_POSITION, WS_VISIBLE | WS_CHILD, Column1X, CurrentY, ControlWidth, ButtonHeight, NULL);
	CameraPosition_ID[0] = InsertElement(NULL, _T("BUTTON"), TEXT_RADIOBUTTON_CAMERA_POSITION_FROM_RADAR, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP, Column1X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::RadioGroup_CameraPosition);
	CurrentY += VStep;
	CameraPosition_ID[1] = InsertElement(NULL, _T("BUTTON"), TEXT_RADIOBUTTON_CAMERA_POSITION_FROM_5KM_ABOVE_RADAR, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, Column1X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::RadioGroup_CameraPosition);
	CurrentY += VStep;
	CameraPosition_ID[2] = InsertElement(NULL, _T("BUTTON"), TEXT_RADIOBUTTON_CAMERA_POSITION_FROM_5KM_ABOVE_RADAR_5KM_NORTH, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, Column1X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::RadioGroup_CameraPosition);
	CurrentY += VStep;
	//CameraDirectionValue_ID[0] = InsertElement(_T("STATIC"), _T(""), WS_VISIBLE | WS_CHILD, 10, 530, 100, 30, NULL);
	//CameraDirectionValue_ID[1] = InsertElement(_T("STATIC"), _T(""), WS_VISIBLE | WS_CHILD, 110, 530, 100, 30, NULL);

	int gridX = panelWidth;
	int gridY = vpControl->Height;
	RECT clientRect;
	GetClientRect(parentHWND, &clientRect);


	//Grid_ID = InsertElement(NULL, _T("ZeeGrid"), TEXT_GRID_NAME, WS_BORDER | WS_TABSTOP | WS_VISIBLE | WS_CHILD, gridX, gridY, vpControl->Width, clientRect.right - panelWidth, &CUserInterface::Grid);

	
	for (ElementsMap::iterator it = Elements.begin(); it != Elements.end(); ++it) {
		it->second->hWnd = CreateWindowEx(NULL,
			it->second->Class,
			it->second->Text,
			it->second->Style,
			it->second->X,
			it->second->Y,
			it->second->Width,
			it->second->Height,
			parentHWND,
			(HMENU)it->second->ID,
			GetModuleHandle(NULL),
			this);
	}

	SetChecked(ObjOptions_ID[0], 1);
	SetChecked(ObjOptions_ID[1], 1);
	SetChecked(MapOptions_ID[0], 1);
	SetChecked(MapOptions_ID[1], 1);

	SendMessage(GetDlgItem(parentHWND, CameraDirection_ID[0]), TBM_SETPOS, 1, 50);
	SendMessage(GetDlgItem(parentHWND, CameraDirection_ID[1]), TBM_SETPOS, 1, 50);

	ControlEnable(ObjOptions_ID[1], false);
	ControlEnable(ObjOptions_ID[2], false);


	//////////font setting:
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(ncm);

	// If we're compiling with the Vista SDK or later, the NONCLIENTMETRICS struct
	// will be the wrong size for previous versions, so we need to adjust it.
#if(_MSC_VER >= 1500 && WINVER >= 0x0600)
	if (!IsVistaOrLater())
	{
		// In versions of Windows prior to Vista, the iPaddedBorderWidth member
		// is not present, so we need to subtract its size from cbSize.
		ncm.cbSize -= sizeof(ncm.iPaddedBorderWidth);
	}
#endif

	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
	hDlgFont = CreateFontIndirect(&(ncm.lfMessageFont));

	// Set the dialog to use the system message box font
	//SetFont(m_DlgFont, TRUE);
	

	for (ElementsMap::iterator it = Elements.begin(); it != Elements.end(); ++it) {
		SendMessage(it->second->hWnd, WM_SETFONT, (WPARAM)hDlgFont, TRUE);		
	}

	InitGrid();
}

CUserInterface::~CUserInterface()
{
	for (ElementsMap::iterator it = Elements.begin(); it != Elements.end(); ++it) {
		delete it->second;
	}
	Elements.clear();
	ElementsMap().swap(Elements);
	if (hgridmod) {
		FreeLibrary(hgridmod);
	}
}

void CUserInterface::ConnectionStateChanged(bool IsConnected)
{
	if (IsConnected) {
		SetDlgItemText(ParentHWND, IsConnected_ID, TEXT_LABEL_CONNECTED);
		SetDlgItemText(ParentHWND, Button_Connect_ID, TEXT_BUTTON_DISCONNECT);
	}
	else {
		SetDlgItemText(ParentHWND, IsConnected_ID, TEXT_LABEL_NOT_CONNECTED);
		SetDlgItemText(ParentHWND, Button_Connect_ID, TEXT_BUTTON_CONNECT);
	}
}

bool CUserInterface::GetCheckboxState_Points()
{
	return Button_GetCheck(GetDlgItem(ParentHWND, ObjOptions_ID[0]));
}

int CUserInterface::GetTrackbarValue_VTilt()
{
	HWND hwnd = GetDlgItem(ParentHWND, CameraDirection_ID[0]);
	int val = SendMessage(hwnd, TBM_GETPOS, 0, 0);
	return val;
}

int CUserInterface::GetTrackbarValue_Turn()
{
	HWND hwnd = GetDlgItem(ParentHWND, CameraDirection_ID[1]);
	int val = SendMessage(hwnd, TBM_GETPOS, 0, 0);
	return val;
}

void CUserInterface::SetTrackbarValue_VTilt(int val)
{
	SendMessage(GetDlgItem(ParentHWND, CameraDirection_ID[0]), TBM_SETPOS, 1, val);
}

void CUserInterface::SetTrackbarValue_Turn(int val)
{
	SendMessage(GetDlgItem(ParentHWND, CameraDirection_ID[1]), TBM_SETPOS, 1, val);
}

void CUserInterface::ControlEnable(int ID, bool enable)
{
	Button_Enable(GetDlgItem(ParentHWND, ID), enable);
}

bool CUserInterface::IsVistaOrLater()
{
	OSVERSIONINFO	vi;

	memset(&vi, 0, sizeof vi);
	vi.dwOSVersionInfoSize = sizeof vi;
	GetVersionEx(&vi);
	return (vi.dwPlatformId == VER_PLATFORM_WIN32_NT  &&  vi.dwMajorVersion >= 6);
}

void CUserInterface::Resize()
{
	RECT clientRect;
	GetClientRect(ParentHWND, &clientRect);
	HWND gridHwnd = GetDlgItem(ParentHWND, Grid_ID);
	MoveWindow(gridHwnd, PanelWidth, VPControl->Height + 2, VPControl->Width, clientRect.bottom - VPControl->Height - 2, TRUE);
	//SetWindowPos(gridHwnd, NULL, PanelWidth, VPControl->Height + 2, VPControl->Width, clientRect.bottom - VPControl->Height - 2, 0);
}

void CUserInterface::InitGrid()
{
	HWND hg = GetDlgItem(ParentHWND, Grid_ID);
	/*SendMessage(gridHwnd, BGM_SETGRIDDIM, 5, 7);
	SendMessage(gridHwnd, BGM_SETCOLSNUMBERED, FALSE, 0);
	SendMessage(gridHwnd, BGM_EXTENDLASTCOLUMN, FALSE, 0);
	SendMessage(gridHwnd, BGM_SETFONT, (WPARAM)hDlgFont, TRUE);

	PutCell(gridHwnd, 0, 1, (long)(new std::string("ID"))->c_str());
	PutCell(gridHwnd, 0, 2, (long)(new std::string("Начальная точка"))->c_str());
	PutCell(gridHwnd, 0, 3, (long)"Конечная точка");
	PutCell(gridHwnd, 0, 4, (long)"Скорость");
	PutCell(gridHwnd, 0, 5, (long)"Азимут");
	PutCell(gridHwnd, 0, 6, (long)"Время");*/
	SendMessage(hg, ZGM_DIMGRID, 10, 0);
	SendMessage(hg, ZGM_SHOWROWNUMBERS, TRUE, 0);
	int j;
	for (j = 1; j <= 5; j++)
	{
		SendMessage(hg, ZGM_APPENDROW, 0, 0);
	}
	//set column header titles
	SendMessage(hg, ZGM_SETCELLTEXT, 1, (LPARAM)"First\nColumn");
	SendMessage(hg, ZGM_SETCELLTEXT, 2, (LPARAM)"Second\nColumn");
	SendMessage(hg, ZGM_SETCELLTEXT, 3, (LPARAM)"Third\nColumn");
	SendMessage(hg, ZGM_SETCELLTEXT, 4, (LPARAM)"Fourth\nColumn");
	SendMessage(hg, ZGM_SETCELLTEXT, 5, (LPARAM)"Fifth\nColumn");
	SendMessage(hg, ZGM_SETCELLTEXT, 6, (LPARAM)"Sixth\nColumn");
	SendMessage(hg, ZGM_SETCELLTEXT, 7, (LPARAM)"Seventh Column is wide");
	SendMessage(hg, ZGM_SETCELLTEXT, 8, (LPARAM)"Eighth\nColumn");
	SendMessage(hg, ZGM_SETCELLTEXT, 9, (LPARAM)"Ninth\nColumn");
	SendMessage(hg, ZGM_SETCELLTEXT, 10, (LPARAM)"Tenth\nand\nlast\nColumn");

	//make column 3 editable by the user
	SendMessage(hg, ZGM_SETCOLEDIT, 3, 1);

	//auto size all columns
	SendMessage(hg, ZGM_AUTOSIZE_ALL_COLUMNS, 0, 0);
}


