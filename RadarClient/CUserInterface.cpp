#pragma once
#include "CUserInterface.h"
#include "CRCSocket.h"
#include "ViewPortControl.h"
#include "CCamera.h"
#include "CScene.h"
#include <CommCtrl.h>
#include <string>



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


int CUserInterface::InsertElement(LPCSTR _class, LPCSTR text, DWORD style, int x, int y, int width, int height, UIWndProc action)
{
	Elements.insert({ CurrentID, new InterfaceElement{ CurrentID, _class, text, style, x, y, width, height, NULL, action } });
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
CUserInterface::CUserInterface(HWND parentHWND, ViewPortControl *vpControl, CRCSocket *socket)
{
	this->ParentHWND = parentHWND;
	this->VPControl = vpControl;
	this->Socket = socket;
	this->CurrentID = 1;


	IsConnected_ID = InsertElement(_T("STATIC"), TEXT_LABEL_NOT_CONNECTED, WS_VISIBLE | WS_CHILD, 10, 10, 270, 30, NULL);
	Button_Connect_ID = InsertElement(_T("BUTTON"), TEXT_BUTTON_CONNECT, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 50, 180, 30, &CUserInterface::Button_Connect);

	InsertElement(_T("STATIC"), TEXT_LABEL_SHOW, WS_VISIBLE | WS_CHILD, 10, 110, 270, 30, NULL);
	ObjOptions_ID[0] = InsertElement(_T("BUTTON"), TEXT_CHECKBOX_POINTS, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX, 10, 150, 270, 30, &CUserInterface::Checkbox_ObjOptions);
	ObjOptions_ID[1] = InsertElement(_T("BUTTON"), TEXT_CHECKBOX_SERIES, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX, 10, 180, 270, 30, &CUserInterface::Checkbox_ObjOptions);
	ObjOptions_ID[2] = InsertElement(_T("BUTTON"), TEXT_CHECKBOX_RLI, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX, 10, 210, 270, 30, &CUserInterface::Checkbox_ObjOptions);
	

	InsertElement(_T("STATIC"), TEXT_LABEL_VIEW, WS_VISIBLE | WS_CHILD, 10, 270, 270, 30, NULL);
	MapOptions_ID[0] = InsertElement(_T("BUTTON"), TEXT_CHECKBOX_LANDSCAPE, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX, 10, 310, 270, 30, &CUserInterface::Checkbox_MapOptions);
	MapOptions_ID[1] = InsertElement(_T("BUTTON"), TEXT_CHECKBOX_MAP, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX, 10, 340, 270, 30, &CUserInterface::Checkbox_MapOptions);

	InsertElement(_T("STATIC"), TEXT_LABEL_CAMERA_POSITION, WS_VISIBLE | WS_CHILD, 10, 400, 270, 30, NULL);
	CameraPosition_ID[0] = InsertElement(_T("BUTTON"), TEXT_RADIOBUTTON_CAMERA_POSITION_FROM_RADAR, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP, 10, 440, 300, 30, &CUserInterface::RadioGroup_CameraPosition);
	CameraPosition_ID[1] = InsertElement(_T("BUTTON"), TEXT_RADIOBUTTON_CAMERA_POSITION_FROM_5KM_ABOVE_RADAR, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 10, 470, 300, 30, &CUserInterface::RadioGroup_CameraPosition);
	CameraPosition_ID[2] = InsertElement(_T("BUTTON"), TEXT_RADIOBUTTON_CAMERA_POSITION_FROM_5KM_ABOVE_RADAR_5KM_NORTH, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 10, 500, 300, 30, &CUserInterface::RadioGroup_CameraPosition);

	CameraDirectionValue_ID[0] = InsertElement(_T("STATIC"), _T(""), WS_VISIBLE | WS_CHILD, 10, 530, 100, 30, NULL);
	CameraDirectionValue_ID[1] = InsertElement(_T("STATIC"), _T(""), WS_VISIBLE | WS_CHILD, 110, 530, 100, 30, NULL);

	CameraDirection_ID[0] = InsertElement(TRACKBAR_CLASS, TEXT_LABEL_CAMERA_POSITION, WS_VISIBLE | WS_CHILD | TBS_VERT | TBS_RIGHT, 300, 400, 50, 230, &CUserInterface::Trackbar_CameraDirection_VTilt);
	CameraDirection_ID[1] = InsertElement(TRACKBAR_CLASS, TEXT_LABEL_CAMERA_POSITION, WS_VISIBLE | WS_CHILD | TBS_HORZ | TBS_BOTTOM, 10, 600, 230, 50, &CUserInterface::Trackbar_CameraDirection_Turn);

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
	HFONT hDlgFont = CreateFontIndirect(&(ncm.lfMessageFont));

	// Set the dialog to use the system message box font
	//SetFont(m_DlgFont, TRUE);
	

	for (ElementsMap::iterator it = Elements.begin(); it != Elements.end(); ++it) {
		SendMessage(it->second->hWnd, WM_SETFONT, (WPARAM)hDlgFont, TRUE);		
	}
}

CUserInterface::~CUserInterface()
{
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


