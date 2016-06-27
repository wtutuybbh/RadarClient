#pragma once
#include "CUserInterface.h"
#include "CRCSocket.h"
#include "CViewPortControl.h"
#include "CCamera.h"
#include "CScene.h"
#include <CommCtrl.h>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "ZeeGrid.h"

#include "Util.h"
#include "CMesh.h"

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
#ifdef _DEBUG
	std::stringstream s;
	glm::vec4 radarPoint = glm::vec4(100, 200, 300, 1);
	glm::vec4 radarPoint1 = radarPoint + glm::vec4(1, 0, 0, 0);
	glm::vec4 radarPoint2 = radarPoint + glm::vec4(0, 1, 0, 0);
	glm::vec4 radarPoint3 = radarPoint + glm::vec4(0, 0, 1, 0);
	glm::mat4 mvp = VPControl->Camera->GetProjection() * VPControl->Camera->GetView();
	glm::vec4 screenPoint = mvp * radarPoint;
	glm::vec4 screenPoint1 = mvp * radarPoint1;
	glm::vec4 screenPoint2 = mvp * radarPoint2;
	glm::vec4 screenPoint3 = mvp * radarPoint3;
	screenPoint = screenPoint / screenPoint.w;
	screenPoint1 = screenPoint1 / screenPoint1.w;
	screenPoint2 = screenPoint2 / screenPoint2.w;
	screenPoint3 = screenPoint3 / screenPoint3.w;

	glm::mat4 model1 = glm::scale(glm::vec3(1 / glm::distance(screenPoint, screenPoint1) / VPControl->Scene->height));
	glm::mat4 model2 = glm::scale(glm::vec3(1 / glm::distance(screenPoint, screenPoint2) / VPControl->Scene->height));
	glm::mat4 model3 = glm::scale(glm::vec3(1 / glm::distance(screenPoint, screenPoint3) / VPControl->Scene->height));

	glm::mat4 model = glm::scale(glm::vec3(
		1 / glm::distance(screenPoint, screenPoint1) / VPControl->Scene->height, 
		1 / glm::distance(screenPoint, screenPoint2) / VPControl->Scene->height, 
		1 / glm::distance(screenPoint, screenPoint3) / VPControl->Scene->height));

	glm::vec4 screenPoint1_1 = VPControl->Camera->GetProjection() * VPControl->Camera->GetView() * model1 * radarPoint1;
	screenPoint1_1 = screenPoint1_1 / screenPoint1_1.w;

	glm::vec4 screenPoint2_1 = VPControl->Camera->GetProjection() * VPControl->Camera->GetView() * model2 * radarPoint2;
	screenPoint2_1 = screenPoint2_1 / screenPoint2_1.w;

	glm::vec4 screenPoint3_1 = VPControl->Camera->GetProjection() * VPControl->Camera->GetView() * model3 * radarPoint3;
	screenPoint3_1 = screenPoint3_1 / screenPoint3_1.w;

	glm::vec4 screenPoint1_2 = VPControl->Camera->GetProjection() * VPControl->Camera->GetView() * model * radarPoint1;
	screenPoint1_2 = screenPoint1_2 / screenPoint1_2.w;

	glm::vec4 screenPoint2_2 = VPControl->Camera->GetProjection() * VPControl->Camera->GetView() * model * radarPoint2;
	screenPoint2_2 = screenPoint2_2 / screenPoint2_2.w;

	glm::vec4 screenPoint3_2 = VPControl->Camera->GetProjection() * VPControl->Camera->GetView() * model * radarPoint3;
	screenPoint3_2 = screenPoint3_2 / screenPoint3_2.w;

	/*s << "(0, " << radarPoint.y << ", 0, 1) ->(" << screenPoint.x << ", " << screenPoint.y << ", " << screenPoint.z << ", " << screenPoint.w << ")";
	DebugMessage(dwi, s.str());
	s.str(std::string());*/
	s << glm::distance(screenPoint, screenPoint1)*VPControl->Scene->height << " vs " << glm::distance(screenPoint, screenPoint1_1) * VPControl->Scene->height << " dist " << 1 / glm::length(VPControl->Camera->GetPosition()) << " test " << (1 / glm::distance(screenPoint, screenPoint1) / (glm::length(VPControl->Camera->GetPosition())));
	DebugMessage(dwi, s.str());
	s.str(std::string());
	s << glm::distance(screenPoint, screenPoint2)*VPControl->Scene->height << " vs " << glm::distance(screenPoint, screenPoint2_1) * VPControl->Scene->height << " dist " << 1 / glm::length(VPControl->Camera->GetPosition()) << " test " << (1 / glm::distance(screenPoint, screenPoint2) / (glm::length(VPControl->Camera->GetPosition())));
	DebugMessage(dwi, s.str());
	s.str(std::string());
	s << glm::distance(screenPoint, screenPoint3)*VPControl->Scene->height << " vs " << glm::distance(screenPoint, screenPoint3_1) * VPControl->Scene->height << " dist " << 1 / glm::length(VPControl->Camera->GetPosition()) << " test " << (1 / glm::distance(screenPoint, screenPoint3) / (glm::length(VPControl->Camera->GetPosition())));
	DebugMessage(dwi, s.str());
	s.str(std::string());
	s << glm::distance(screenPoint, screenPoint1)*VPControl->Scene->height << " vs " << glm::distance(screenPoint, screenPoint1_2) * VPControl->Scene->height << " dist " << 1 / glm::length(VPControl->Camera->GetPosition()) << " test " << (1 / glm::distance(screenPoint, screenPoint1) / (glm::length(VPControl->Camera->GetPosition())));
	DebugMessage(dwi, s.str());
	s.str(std::string());
	s << glm::distance(screenPoint, screenPoint2)*VPControl->Scene->height << " vs " << glm::distance(screenPoint, screenPoint2_2) * VPControl->Scene->height << " dist " << 1 / glm::length(VPControl->Camera->GetPosition()) << " test " << (1 / glm::distance(screenPoint, screenPoint2) / (glm::length(VPControl->Camera->GetPosition())));
	DebugMessage(dwi, s.str());
	s.str(std::string());
	s << glm::distance(screenPoint, screenPoint3)*VPControl->Scene->height << " vs " << glm::distance(screenPoint, screenPoint3_2) * VPControl->Scene->height << " dist " << 1 / glm::length(VPControl->Camera->GetPosition()) << " test " << (1 / glm::distance(screenPoint, screenPoint3) / (glm::length(VPControl->Camera->GetPosition())));
	DebugMessage(dwi, s.str());

	
#endif // _DEBUG
	return LRESULT();
}

LRESULT CUserInterface::Grid(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return LRESULT();
}

LRESULT CUserInterface::InfoGrid(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

float CUserInterface::GetHeight()
{
	return GetTrackbarValue_VTilt(); //here elevation counts from top-up direction
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

LRESULT CUserInterface::Checkbox_MarkupOptions(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int ButtonID = LOWORD(wParam);
	HWND hWnd = GetDlgItem(hwnd, ButtonID);
	int Checked = !Button_GetCheck(hWnd);
	SendMessage(hWnd, BM_SETCHECK, Checked, 0);

	return LRESULT();
}

LRESULT CUserInterface::Checkbox_FixViewToRadar(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int ButtonID = LOWORD(wParam);
	HWND hWnd = GetDlgItem(hwnd, ButtonID);
	int Checked = !Button_GetCheck(hWnd);
	SendMessage(hWnd, BM_SETCHECK, Checked, 0);
	if (VPControl && VPControl->Camera)
		this->VPControl->Camera->FixViewOnRadar = Checked;
	return LRESULT();
}

LRESULT CUserInterface::Checkbox_MeasureDistance(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int ButtonID = LOWORD(wParam);
	HWND hWnd = GetDlgItem(hwnd, ButtonID);
	int Checked = !Button_GetCheck(hWnd);
	SendMessage(hWnd, BM_SETCHECK, Checked, 0);

	return LRESULT();
}

LRESULT CUserInterface::RadioGroup_CameraPosition(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int ButtonID = LOWORD(wParam);
	HWND hWnd = GetDlgItem(hwnd, ButtonID);
	if (VPControl && VPControl->Camera)
	{
		if (ButtonID == CameraPosition_ID[0]) { // FROM_RADAR
			this->VPControl->Camera->SetPosition(0, this->VPControl->Scene->y0 + 1, 0);
		}
		if (ButtonID == CameraPosition_ID[1]) { // FROM_100M_ABOVE_RADAR
			this->VPControl->Camera->SetPosition(0, this->VPControl->Scene->y0 + 100.0f / this->VPControl->Scene->mppv, 0);
		}
		if (ButtonID == CameraPosition_ID[2]) { // FROM_1000M_ABOVE_RADAR
			this->VPControl->Camera->SetPosition(0, this->VPControl->Scene->y0 + 1000.0f / this->VPControl->Scene->mppv, 0);
		}
	}
	return LRESULT();
}

LRESULT CUserInterface::Trackbar_CameraDirection_VTilt(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int ID = GetDlgCtrlID((HWND)lParam);
	
	int val = SendMessage(GetDlgItem(hwnd, ID), TBM_GETPOS, 0, 0);

	//SetDlgItemText(ParentHWND, CameraDirectionValue_ID[0], std::to_string(val).c_str());
	if (VPControl && VPControl->Camera && VPControl->Scene)
		//VPControl->Camera->Direction = GetDirection();
	{
		glm::vec3 p = VPControl->Camera->GetPosition();
		
		VPControl->Camera->SetPosition(p.x, VPControl->Scene->y0 + (5000 - VPControl->Scene->y0)*(100 - val)/100/ VPControl->Scene->mppv, p.z);
	}

	return LRESULT();
}
LRESULT CUserInterface::Trackbar_CameraDirection_Turn(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int ID = GetDlgCtrlID((HWND)lParam);

	int val = SendMessage(GetDlgItem(hwnd, ID), TBM_GETPOS, 0, 0);

	SetDlgItemText(ParentHWND, CameraDirectionValue_ID[1], std::to_string(val).c_str());
	
	if (VPControl && VPControl->Camera)
		VPControl->Camera->Direction = GetDirection();

	return LRESULT();
}
//TRACKBAR_CLASS
CUserInterface::CUserInterface(HWND parentHWND, CViewPortControl *vpControl, CRCSocket *socket, int panelWidth)
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
	Column2X = panelWidth / 3;
	Column3X = Column2X * 2;
	VStep = 22;
	VStepGrp = 30;
	MinimapSize = PanelWidth - VStepGrp;
	ButtonHeight = 20, ControlWidth = Column2X - VStepGrp - VStep, ControlWidthL = Column2X - VStep, ControlWidthXL = Column3X;

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

	CurrentY += VStep+VStep;

	FixViewToRadar_ID = InsertElement(NULL, _T("BUTTON"), TEXT_CHECKBOX_FIXVIEWTORADAR, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX, Column1X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::Checkbox_FixViewToRadar);

	MeasureDistance_ID = InsertElement(NULL, _T("BUTTON"), TEXT_CHECKBOX_MEASURE_DISTANCE, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX, Column2X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::Checkbox_MeasureDistance);
	
	Test_ID = InsertElement(NULL, _T("BUTTON"), TEXT_BUTTON_TEST, WS_TABSTOP | WS_VISIBLE | WS_CHILD, Column3X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::Button_Test);

	CurrentY += VStepGrp;

	Button_Connect_ID = InsertElement(NULL, _T("BUTTON"), TEXT_BUTTON_CONNECT, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, Column1X, CurrentY, ControlWidthL, ButtonHeight, &CUserInterface::Button_Connect);
	IsConnected_ID = InsertElement(NULL, _T("STATIC"), TEXT_LABEL_NOT_CONNECTED, WS_VISIBLE | WS_CHILD, Column2X, CurrentY, ControlWidthL, ButtonHeight, NULL);
	
	CurrentY += VStepGrp;

	InsertElement(NULL, _T("STATIC"), TEXT_LABEL_SHOW, WS_VISIBLE | WS_CHILD, Column1X, CurrentY, ControlWidth, ButtonHeight, NULL);
	InsertElement(NULL, _T("STATIC"), TEXT_LABEL_VIEW, WS_VISIBLE | WS_CHILD, Column2X, CurrentY, ControlWidth, ButtonHeight, NULL);
	InsertElement(NULL, _T("STATIC"), TEXT_LABEL_MARKUP, WS_VISIBLE | WS_CHILD, Column3X, CurrentY, ControlWidth, ButtonHeight, NULL);
	CurrentY += VStep;
	ObjOptions_ID[0] = InsertElement(NULL, _T("BUTTON"), TEXT_CHECKBOX_POINTS, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX, Column1X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::Checkbox_ObjOptions);
	MapOptions_ID[0] = InsertElement(NULL, _T("BUTTON"), TEXT_CHECKBOX_LANDSCAPE, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX, Column2X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::Checkbox_MapOptions);
	MarkupOptions_ID[0] = InsertElement(NULL, _T("BUTTON"), TEXT_CHECKBOX_MARKUP_LINES, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX, Column3X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::Checkbox_MarkupOptions);
	CurrentY += VStep;
	ObjOptions_ID[1] = InsertElement(NULL, _T("BUTTON"), TEXT_CHECKBOX_SERIES, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX, Column1X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::Checkbox_ObjOptions);
	MapOptions_ID[1] = InsertElement(NULL, _T("BUTTON"), TEXT_CHECKBOX_MAP, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX, Column2X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::Checkbox_MapOptions);
	MarkupOptions_ID[1] = InsertElement(NULL, _T("BUTTON"), TEXT_CHECKBOX_MARKUP_LABELS, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX, Column3X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::Checkbox_MarkupOptions);
	CurrentY += VStep;
	ObjOptions_ID[2] = InsertElement(NULL, _T("BUTTON"), TEXT_CHECKBOX_RLI, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX, Column1X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::Checkbox_ObjOptions);
	
	CurrentY += VStepGrp;

	InsertElement(NULL, _T("STATIC"), TEXT_LABEL_CAMERA_POSITION, WS_VISIBLE | WS_CHILD, Column1X, CurrentY, ControlWidth, ButtonHeight, NULL);
	CameraPosition_ID[0] = InsertElement(NULL, _T("BUTTON"), TEXT_RADIOBUTTON_CAMERA_POSITION_FROM_RADAR, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP, Column1X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::RadioGroup_CameraPosition);
	CurrentY += VStep;
	CameraPosition_ID[1] = InsertElement(NULL, _T("BUTTON"), TEXT_RADIOBUTTON_CAMERA_POSITION_FROM_100M_ABOVE_RADAR, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, Column1X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::RadioGroup_CameraPosition);
	CurrentY += VStep;
	CameraPosition_ID[2] = InsertElement(NULL, _T("BUTTON"), TEXT_RADIOBUTTON_CAMERA_POSITION_FROM_1000M_ABOVE_RADAR, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, Column1X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::RadioGroup_CameraPosition);
	CurrentY += VStep;
	//CameraDirectionValue_ID[0] = InsertElement(_T("STATIC"), _T(""), WS_VISIBLE | WS_CHILD, 10, 530, 100, 30, NULL);
	//CameraDirectionValue_ID[1] = InsertElement(_T("STATIC"), _T(""), WS_VISIBLE | WS_CHILD, 110, 530, 100, 30, NULL);

	int gridX = panelWidth;
	int gridY = vpControl->Height;
	RECT clientRect;
	GetClientRect(parentHWND, &clientRect);


	Grid_ID = InsertElement(NULL, _T("ZeeGrid"), TEXT_GRID_NAME, WS_BORDER | WS_TABSTOP | WS_VISIBLE | WS_CHILD, gridX, gridY, 2 * vpControl->Width / 3, clientRect.right - panelWidth, &CUserInterface::Grid);
	InfoGrid_ID = InsertElement(NULL, _T("ZeeGrid"), TEXT_INFOGRID_NAME, WS_BORDER | WS_TABSTOP | WS_VISIBLE | WS_CHILD, gridX + 2 * vpControl->Width / 3 + 2, gridY, vpControl->Width / 3 - 2, clientRect.right - panelWidth, &CUserInterface::InfoGrid);

	
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

	SetChecked(ObjOptions_ID[0], 0);
	SetChecked(ObjOptions_ID[1], 1);
	SetChecked(MapOptions_ID[0], 1);
	SetChecked(MapOptions_ID[1], 1);
	SetChecked(MarkupOptions_ID[0], 1);
	SetChecked(MarkupOptions_ID[1], 1);

	SendMessage(GetDlgItem(parentHWND, CameraDirection_ID[0]), TBM_SETPOS, 1, 50);
	SendMessage(GetDlgItem(parentHWND, CameraDirection_ID[1]), TBM_SETPOS, 1, 50);

	//ControlEnable(ObjOptions_ID[1], false);
	//ControlEnable(ObjOptions_ID[2], false);


	//////////font setting:
	/*NONCLIENTMETRICS ncm;
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
	hDlgFont = CreateFontIndirect(&(ncm.lfMessageFont));*/

	// Set the dialog to use the system message box font
	//SetFont(m_DlgFont, TRUE);
	Font = GetFont();

	for (ElementsMap::iterator it = Elements.begin(); it != Elements.end(); ++it) {
		SendMessage(it->second->hWnd, WM_SETFONT, (WPARAM)Font, TRUE);
	}

	GridHWND = InfoGridHWND = NULL;
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

void CUserInterface::ConnectionStateChanged(bool IsConnected) const
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

bool CUserInterface::GetCheckboxState_Map()
{
	return Button_GetCheck(GetDlgItem(ParentHWND, MapOptions_ID[1]));
}

bool CUserInterface::GetCheckboxState_AltitudeMap()
{
	return Button_GetCheck(GetDlgItem(ParentHWND, MapOptions_ID[0]));
}

bool CUserInterface::GetCheckboxState_MarkupLines()
{
	return Button_GetCheck(GetDlgItem(ParentHWND, MarkupOptions_ID[0]));
}

bool CUserInterface::GetCheckboxState_MarkupLabels()
{
	return Button_GetCheck(GetDlgItem(ParentHWND, MarkupOptions_ID[1]));
}

bool CUserInterface::GetCheckboxState_Points()
{
	return Button_GetCheck(GetDlgItem(ParentHWND, ObjOptions_ID[0]));
}

bool CUserInterface::GetCheckboxState_Tracks()
{
	return Button_GetCheck(GetDlgItem(ParentHWND, ObjOptions_ID[1]));
}

bool CUserInterface::GetCheckboxState_Images()
{
	return Button_GetCheck(GetDlgItem(ParentHWND, ObjOptions_ID[2]));
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

void CUserInterface::ControlEnable(int ID, bool enable) const
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

void CUserInterface::Resize() const
{
	RECT clientRect;
	GetClientRect(ParentHWND, &clientRect);
	HWND gridHwnd = GetDlgItem(ParentHWND, Grid_ID);
	MoveWindow(gridHwnd, PanelWidth, VPControl->Height + 2, 2* VPControl->Width / 3, clientRect.bottom - VPControl->Height - 2, TRUE);

	gridHwnd = GetDlgItem(ParentHWND, InfoGrid_ID);
	MoveWindow(gridHwnd, PanelWidth + 2* VPControl->Width / 3 + 2, VPControl->Height + 2, VPControl->Width / 3 - 2, clientRect.bottom - VPControl->Height - 2, TRUE);
	//SetWindowPos(gridHwnd, NULL, PanelWidth, VPControl->Height + 2, VPControl->Width, clientRect.bottom - VPControl->Height - 2, 0);
}

void CUserInterface::InitGrid()
{
	if (!GridHWND)
		GridHWND = GetDlgItem(ParentHWND, Grid_ID);
	if (!InfoGridHWND)
		InfoGridHWND = GetDlgItem(ParentHWND, InfoGrid_ID);

	SendMessage(GridHWND, ZGM_SETFONT, 2, (LPARAM)Font);


	SendMessage(GridHWND, ZGM_DIMGRID, 7, 0);
	SendMessage(GridHWND, ZGM_SHOWROWNUMBERS, TRUE, 0);

	//set column header titles
	SendMessage(GridHWND, ZGM_SETCELLTEXT, 1, (LPARAM)"ID");
	SendMessage(GridHWND, ZGM_SETCELLTEXT, 2, (LPARAM)"Кол-во точек");
	SendMessage(GridHWND, ZGM_SETCELLTEXT, 3, (LPARAM)"Начальная точка");
	SendMessage(GridHWND, ZGM_SETCELLTEXT, 4, (LPARAM)"Конечная точка");
	SendMessage(GridHWND, ZGM_SETCELLTEXT, 5, (LPARAM)"Скорость");
	SendMessage(GridHWND, ZGM_SETCELLTEXT, 6, (LPARAM)"Азимут");
	SendMessage(GridHWND, ZGM_SETCELLTEXT, 7, (LPARAM)"Время");


	//make column 3 editable by the user
	//SendMessage(GridHWND, ZGM_SETCOLEDIT, 3, 1);

	//auto size all columns
	SendMessage(GridHWND, ZGM_AUTOSIZE_ALL_COLUMNS, 0, 0);



	SendMessage(InfoGridHWND, ZGM_SETFONT, 2, (LPARAM)Font);
	SendMessage(InfoGridHWND, ZGM_DIMGRID, 2, 0);
	SendMessage(InfoGridHWND, ZGM_SHOWROWNUMBERS, FALSE, 0);
	SendMessage(InfoGridHWND, ZGM_SETCELLTEXT, 1, (LPARAM)"Параметр");
	SendMessage(InfoGridHWND, ZGM_SETCELLTEXT, 2, (LPARAM)"Значение");
	SendMessage(InfoGridHWND, ZGM_AUTOSIZE_ALL_COLUMNS, 0, 0);
	
	FillInfoGrid(NULL);
}

HFONT CUserInterface::GetFont()
{
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
	return CreateFontIndirect(&(ncm.lfMessageFont));
}

void CUserInterface::FillGrid(vector<TRK*> *tracks)
{
	if (!GridHWND)
		GridHWND = GetDlgItem(ParentHWND, Grid_ID);
	int nrows = SendMessage(GridHWND, ZGM_GETROWS, 0, 0);
	int ncols = SendMessage(GridHWND, ZGM_GETCOLS, 0, 0);
	int i = 0;
	int npoints;
	std::stringstream ss;
	for (i = 0; i < tracks->size(); i++) {
		if (i>nrows)
			SendMessage(GridHWND, ZGM_APPENDROW, 0, 0);
		int offset = ncols*(i + 1);
		SendMessage(GridHWND, ZGM_SETCELLINT, offset + 1, (LPARAM)&(tracks->at(i)->id));

		npoints = tracks->at(i)->P.size();
		SendMessage(GridHWND, ZGM_SETCELLINT, offset + 2, (LPARAM)&npoints);

		ss.str(std::string());
		ss << std::fixed << std::setprecision(4) << tracks->at(i)->P.at(0).X << "," << tracks->at(i)->P.at(0).Y;
		SendMessage(GridHWND, ZGM_SETCELLTEXT, offset + 3, (LPARAM)ss.str().c_str());
		
		ss.str(std::string());
		ss << std::fixed << std::setprecision(4) << tracks->at(i)->P.at(npoints-1).X << "," << tracks->at(i)->P.at(npoints - 1).Y;
		SendMessage(GridHWND, ZGM_SETCELLTEXT, offset + 4, (LPARAM)ss.str().c_str());

		ss.str(std::string());
		glm::vec3 speed(tracks->at(i)->P.at(npoints - 1).vX, tracks->at(i)->P.at(npoints - 1).vY, tracks->at(i)->P.at(npoints - 1).vZ);
		ss << std::fixed << std::setprecision(4) <<  glm::length(speed);
		SendMessage(GridHWND, ZGM_SETCELLTEXT, offset + 5, (LPARAM)ss.str().c_str());

		/*
		SendMessage(GridHWND, ZGM_SETCELLTEXT, offset + 6, (LPARAM)"Азимут");
		SendMessage(GridHWND, ZGM_SETCELLTEXT, offset + 7, (LPARAM)"Время");*/
	}
	for (; i < nrows; i++) {
		SendMessage(GridHWND, ZGM_DELETEROW, i, 0);
	}
	SendMessage(GridHWND, ZGM_AUTOSIZE_ALL_COLUMNS, 0, 0);
}

void CUserInterface::FillInfoGrid(CScene* scene)
{
	if (!scene)
	{
		return;
	}
	if (!InfoGridHWND)
		InfoGridHWND = GetDlgItem(ParentHWND, InfoGrid_ID);
	int nrows = SendMessage(InfoGridHWND, ZGM_GETROWS, 0, 0);
	int ncols = SendMessage(InfoGridHWND, ZGM_GETCOLS, 0, 0);

	SendMessage(InfoGridHWND, ZGM_EMPTYGRID, 1, 0);

	/*int N = 2;

	if (scene->Socket->IsConnected)
	{
		N += 7;
	}

	N += scene->Selection.size();*/
	
	/*for (int i = nrows; i < N; i++)
	{
		SendMessage(InfoGridHWND, ZGM_APPENDROW, 0, 0);
	}*/
	
	std::stringstream ss;
	int r = 1;
	SendMessage(InfoGridHWND, ZGM_SETCELLTEXT, ncols * r + 1, (LPARAM)"Местоположение радара");
	ss.str(std::string());
	ss << std::fixed << std::setprecision(4) << scene->geocenter.x << ", " << scene->geocenter.y;
	SendMessage(InfoGridHWND, ZGM_SETCELLTEXT, ncols * r + 2, (LPARAM)ss.str().c_str());

	if (scene->Socket->IsConnected) {
		r++;
		SendMessage(InfoGridHWND, ZGM_SETCELLTEXT, ncols * r + 1, (LPARAM)"Угол места, от");
		ss.str(std::string());
		ss << std::fixed << std::setprecision(4) << scene->minE;
		SendMessage(InfoGridHWND, ZGM_SETCELLTEXT, ncols * r + 2, (LPARAM)ss.str().c_str());

		r++;
		SendMessage(InfoGridHWND, ZGM_SETCELLTEXT, ncols * r + 1, (LPARAM)"Угол места, до");
		ss.str(std::string());
		ss << std::fixed << std::setprecision(4) << scene->maxE;
		SendMessage(InfoGridHWND, ZGM_SETCELLTEXT, ncols * r + 2, (LPARAM)ss.str().c_str());

		
	}

	SendMessage(InfoGridHWND, ZGM_AUTOSIZE_ALL_COLUMNS, 0, 0);
}

bool CUserInterface::MeasureDistance() const
{
	HWND hWnd = GetDlgItem(ParentHWND, MeasureDistance_ID);
	return Button_GetCheck(hWnd);
}
