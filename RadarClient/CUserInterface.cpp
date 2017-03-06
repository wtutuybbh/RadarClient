#include "stdafx.h"

#include "CUserInterface.h"
#include "CRCSocket.h"
#include "CViewPortControl.h"
#include "CCamera.h"
#include "CScene.h"

#include "ZeeGrid.h"

#include "Util.h"
#include "CMesh.h"
#include "CSettings.h"


#include "CRCLogger.h"

#include "CRCGridCell.h"
#include "resource1.h"

#include "CRCListView.h"


const std::string CUserInterface::requestID = "CUserInterface";
HINSTANCE CUserInterface::hInstance;
HFONT CUserInterface::Font;

LRESULT Button1_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	return 0;
}

//in this method ID is retrieved as LOWORD(wParam)
LRESULT CUserInterface::Wnd_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CUserInterface *ui = this;
	if (!ui) 
	{
		return NULL;
	}
	for (ElementsMap::iterator it = Elements.begin(); it != Elements.end(); ++it) 
	{
		auto ctrlID = LOWORD(wParam);
		if (ctrlID == it->second->ID) 
		{
			if (it->second->Action) 
			{
				CALL_MEMBER_FN(*ui, it->second->Action)(hwnd, uMsg, wParam, lParam);
			}
		}
	}
	return LRESULT();
}
//in this method ID is retrieved as GetDlgCtrlID((HWND)lParam)
LRESULT CUserInterface::Wnd_Proc2(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CUserInterface *ui = this;
	if (!ui)
	{
		return NULL;
	}

	auto ctrlID = GetDlgCtrlID((HWND)lParam);

	try
	{
		auto iel = Elements.at(ctrlID);
		if (iel && iel->Action)
		{
			return CALL_MEMBER_FN(*ui, iel->Action)(hwnd, uMsg, wParam, lParam);
		}		
	} 
	catch (const std::out_of_range& oor)
	{
		LOG_ERROR("CUserInterface", "Wnd_Proc2", "Interface element with ID=%d not found, exception's what=%s", ctrlID, oor.what());
		return NULL;
	}
	catch (const std::exception &ex) {
		LOG_WARN("exception", "CUserInterface::Wnd_Proc2", ex.what());
	}
	return LRESULT();
}

LRESULT CUserInterface::Button_Colors(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	std::string context = "CUserInterface::Button_Colors";
	
	if (GetMainTableMode()==0)
	{
		LOG_INFO(requestID, context, "Switched to color setup.");
		ShowWindow(Elements[Grid_ID]->hWnd, SW_HIDE);
		ShowWindow(Elements[ColorGrid_ID]->hWnd, SW_SHOWNOACTIVATE);
		SetMainTableMode(1);
		return LRESULT();
	}
	if (GetMainTableMode() == 1)
	{
		LOG_INFO(requestID, context, "Switched back to info.");
		ShowWindow(Elements[Grid_ID]->hWnd, SW_SHOWNOACTIVATE);
		ShowWindow(Elements[ColorGrid_ID]->hWnd, SW_HIDE);
		SetMainTableMode(0);
		return LRESULT();
	}
	return LRESULT();
}

LRESULT CUserInterface::Button_Load(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (Socket)
	{
		Socket->Initialized = true;
	}

	return LRESULT();
}

LRESULT CUserInterface::Button_Test(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	const clock_t begin_time = clock();
	
	LOG_INFO("Test", "Test", "Finished, time=%f", float(clock() - begin_time) / CLOCKS_PER_SEC);

	return LRESULT();
}

LRESULT CUserInterface::Button_Settings(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int g_nCmdShow = 1;
	HRSRC       hrsrc;
	HGLOBAL     hglobal;
	HINSTANCE hInstance = (HINSTANCE)GetWindowLong(ParentHWND, GWL_HINSTANCE);

	auto intres = MAKEINTRESOURCE(IDD_DIALOG4);

	

	auto ret = DialogBox(hInstance, intres, ParentHWND, DLGPROC(&CUserInterface::Dialog_Settings));

	//auto hDialog = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG2), ParentHWND, DLGPROC(&CUserInterface::Dialog_Settings));

	//auto err = GetLastError();

	//hglobal = ::LoadResource(hInstance, hrsrc);

	//HWND hwnd1 = CreateDialogIndirect(hInstance, (LPCDLGTEMPLATE)hglobal, ParentHWND, (DLGPROC)&CUserInterface::Dialog_Settings);

	//ShowWindow(hwnd1, g_nCmdShow);

	//SetWindowPos(hwnd1, HWND_TOP, 0, 720, 0, 0, SWP_NOSIZE);


	return LRESULT();
}

LRESULT CALLBACK CUserInterface::Dialog_Settings(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG: {

				
		auto s = GetWStringFromResourceID(IDS_STRING108);

		auto hwndListView = GetDlgItem(hDlg, IDC_LIST1);

		CRCListView::InitListView(hwndListView, 12, InitColorListViewColumns);
		RECT rect;
		GetClientRect(hwndListView, &rect);
		ListView_SetExtendedListViewStyle(hwndListView, LVS_EX_FULLROWSELECT);
		ListView_SetColumnWidth(hwndListView, 0, 150);
		ListView_SetColumnWidth(hwndListView, 1, 70);
		ListView_SetColumnWidth(hwndListView, 2, rect.right - 220);
		//s = GetWStringFromResourceID(IDS_STRING109);
		
		auto color = CSettings::GetColorString(ColorAxis);
	}
		break;

	case WM_CTLCOLORSTATIC:
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	case WM_SIZE:
		CRCListView::ResizeListView(GetDlgItem(hDlg, IDC_LIST1), hDlg);
		break;
	case WM_NOTIFY:
	{
		if (((LPNMHDR)lParam)->code == NM_CUSTOMDRAW) {
			SetWindowLong(hDlg, DWL_MSGRESULT,
				(LONG)ProcessColorListViewCustomDraw(lParam));
			return TRUE;
		}
		return CRCListView::ListViewNotify(hDlg, lParam, IDC_LIST1, GetColorListViewCellText);
	}
	break;
	}
	return false;
}
LRESULT CUserInterface::ProcessColorListViewCustomDraw(LPARAM lParam) {
	LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)lParam;
	switch (lplvcd->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT: //Before the paint cycle begins
						//request notifications for individual listview items
		return CDRF_NOTIFYITEMDRAW;

	case CDDS_ITEMPREPAINT: //Before an item is drawn
		/*if (((int)lplvcd->nmcd.dwItemSpec % 2) == 0)
		{
			//customize item appearance
			lplvcd->clrText = RGB(255, 0, 0);
			lplvcd->clrTextBk = RGB(200, 200, 200);
			return CDRF_NOTIFYSUBITEMDRAW;// CDRF_NEWFONT;
		}
		else {
			lplvcd->clrText = RGB(0, 0, 255);
			lplvcd->clrTextBk = RGB(255, 255, 255);

			return CDRF_NOTIFYSUBITEMDRAW; // CDRF_NEWFONT;
		}*/
		return CDRF_NOTIFYSUBITEMDRAW;
		break;

		//Before a subitem is drawn
	case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
		//(int)lplvcd->nmcd.dwItemSpec
		//lplvcd->iSubItem

		/*if (1 == (int)lplvcd->nmcd.dwItemSpec)
		{
			if (0 == lplvcd->iSubItem)
			{
				//customize subitem appearance for column 0
				lplvcd->clrText = RGB(255, 0, 0);
				lplvcd->clrTextBk = RGB(255, 255, 255);

				//To set a custom font:
				//SelectObject(lplvcd->nmcd.hdc, 
				//    <your custom HFONT>);

				return CDRF_NEWFONT;
			}
			else if (1 == lplvcd->iSubItem)
			{
				//customize subitem appearance for columns 1..n
				//Note: setting for column i 
				//carries over to columnn i+1 unless
				//      it is explicitly reset
				lplvcd->clrTextBk = RGB(255, 0, 0);
				lplvcd->clrTextBk = RGB(255, 255, 255);

				return CDRF_NEWFONT;
			}
		}*/
		SetColorListViewItemColor(lParam, int(lplvcd->nmcd.dwItemSpec), lplvcd->iSubItem);
		break;
	}
	return CDRF_DODEFAULT;
}
std::string CUserInterface::GetStringFromResourceID(int ID)
{
	wchar_t *p = nullptr;

	int len = LoadString(hInstance, IDS_STRING108, reinterpret_cast<LPWSTR>(&p), 0);
	wstring ws = wstring(p, size_t(len));
	string s(ws.begin(), ws.end());
	return s;
}

std::wstring CUserInterface::GetWStringFromResourceID(int ID)
{
	wchar_t *p = nullptr;
	int len = LoadString(hInstance, IDS_STRING108, reinterpret_cast<LPWSTR>(&p), 0);
	return wstring(p, size_t(len));
}

tstring CUserInterface::GetColorForSettingsDialog(int index)
{
	switch (index)
	{
	case 0:
		return TEXT("ColorBackgroud");
	case 1:
		return  TEXT("ColorAxis");
	case 2:
		return  TEXT("ColorNumbers");
	case 3:
		return  TEXT("ColorPointLowLevel");
	case 4:
		return  TEXT("ColorPointHighLevel");
	case 5:
		return  TEXT("ColorPointSelected");
	case 6:
		return  TEXT("ColorTrack");
	case 7:
		return  TEXT("ColorTrackSelected");
	case 8:
		return  TEXT("ColorMeasureLine");
	case 9:
		return  TEXT("ColorAltitudeLowest");
	case 10:
		return  TEXT("ColorAltitudeHighest");
	case 11:
		return  TEXT("ColorBlankZones");
	default:
		return  TEXT("");
	}
}

tstring CUserInterface::GetColorListViewCellText(int iItem, int iSubItem)
{
	if (iSubItem == 0)
		return GetColorForSettingsDialog(iItem);
	if (iSubItem == 1)
		return CSettings::GetColorString(CSettings::GetIndex(GetColorForSettingsDialog(iItem)));
	return TEXT("");
}

void CUserInterface::SetColorListViewItemColor(LPARAM lParam, int iItem, int iSubItem)
{
	if (iSubItem == 2)
	{
		LPNMLVCUSTOMDRAW lplvcd = LPNMLVCUSTOMDRAW(lParam);

		auto c = CSettings::GetColorRGB(CSettings::GetIndex(GetColorForSettingsDialog(iItem)));

		auto cR = GetRValue(c);
		auto cG = GetGValue(c);
		auto cB = GetBValue(c);


		lplvcd->clrTextBk = c;

		const float gamma = 2.2;
		float L = 0.2126 * pow(GetRValue(c)/255.0, gamma)
			+ 0.7152 * pow(GetGValue(c) / 255.0, gamma)
			+ 0.0722 * pow(GetBValue(c) / 255.0, gamma);

		boolean use_black = (L > pow(0.5, gamma));

		if (use_black)
		{
			lplvcd->clrText = RGB(0, 0, 0);
		}
		else
		{
			lplvcd->clrText = RGB(255, 255, 255);
		}
	}
}

void CUserInterface::InitColorListViewColumns(HWND hwndListView, LV_COLUMN lvColumn)
{
	TCHAR       szString[3][20] = { TEXT("Параметр"), TEXT("Значение"), TEXT("Цвет") };
	for (auto i = 0; i < 3; i++)
	{
		lvColumn.pszText = szString[i];
		ListView_InsertColumn(hwndListView, i, &lvColumn);
	}
}

LRESULT CUserInterface::Grid(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return LRESULT();
}

LRESULT CUserInterface::ColorGrid(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return LRESULT();
}

LRESULT CUserInterface::InfoGrid(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return LRESULT();
}

LRESULT CUserInterface::IDD_DIALOG1_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

float CUserInterface::GetBegAzm()
{
	return CSettings::GetFloat(FloatMinBegAzm) + (CSettings::GetFloat(FloatMaxBegAzm) - CSettings::GetFloat(FloatMinBegAzm)) * GetTrackbarValue(BegAzm_ID)/100.0;
}

float CUserInterface::GetZeroElevation()
{
	return CSettings::GetFloat(FloatMinZeroElevation) + (CSettings::GetFloat(FloatMaxZeroElevation) - CSettings::GetFloat(FloatMinZeroElevation)) * GetTrackbarValue(ZeroElevation_ID) / 100.0;
}

float CUserInterface::GetHeight()
{
	return GetTrackbarValue_VTilt(); //here elevation counts from top-up direction
}

int CUserInterface::InsertElement(DWORD xStyle, LPCWSTR _class, LPCWSTR text, DWORD style, int x, int y, int width, int height, UIWndProc action)
{
	Elements.insert({ CurrentID, new InterfaceElement{ CurrentID, xStyle, _class, text, style, x, y, width, height, nullptr, action } });
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
		if (VPControl) {			
			VPControl->DisplayLandscape = Checked;
		}
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
	if (VPControl && VPControl->Camera)
		this->VPControl->Camera->FixViewOnRadar = Button_GetCheck(hWnd);
	return LRESULT();
}

LRESULT CUserInterface::Checkbox_MeasureDistance(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int ButtonID = LOWORD(wParam);
	HWND hWnd = GetDlgItem(hwnd, ButtonID);

	return LRESULT();
}

LRESULT CUserInterface::RadioGroup_CameraPosition(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int ButtonID = LOWORD(wParam);
	HWND hWnd = GetDlgItem(hwnd, ButtonID);
	if (VPControl && VPControl->Camera)
	{
		if (ButtonID == CameraPosition_ID[0]) { // FROM_RADAR
			if (this->VPControl->Scene && this->VPControl->Scene->MeshReady()) this->VPControl->Camera->SetPosition(0, this->VPControl->Scene->GetY0() + 1.0f, 0);
		}
		if (ButtonID == CameraPosition_ID[1]) { // FROM_100M_ABOVE_RADAR
			if (this->VPControl->Scene && this->VPControl->Scene->MeshReady()) this->VPControl->Camera->SetPosition(0, this->VPControl->Scene->GetY0() + 100.0f / this->VPControl->Scene->MPPv, 0);
		}
		if (ButtonID == CameraPosition_ID[2]) { // FROM_1000M_ABOVE_RADAR
			if (this->VPControl->Scene && this->VPControl->Scene->MeshReady()) this->VPControl->Camera->SetPosition(0, this->VPControl->Scene->GetY0() + 1000.0f / this->VPControl->Scene->MPPv, 0);
		}
	}
	return LRESULT();
}

LRESULT CUserInterface::Checkbox_MapType(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int ButtonID = LOWORD(wParam);
	HWND hWnd = GetDlgItem(hwnd, ButtonID);
	int Checked = !Button_GetCheck(hWnd);
	SendMessage(hWnd, BM_SETCHECK, Checked, 0);
	if (VPControl && VPControl->Camera)
	{
		if (ButtonID == MapType_ID[0]) { // Map
			VPControl->DisplayMap = Checked;
		}
	}
	return LRESULT();
}

LRESULT CUserInterface::Checkbox_BlindZones(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int ButtonID = LOWORD(wParam);
	HWND hWnd = GetDlgItem(hwnd, ButtonID);
	int Checked = !Button_GetCheck(hWnd);
	SendMessage(hWnd, BM_SETCHECK, Checked, 0);
	if (VPControl && VPControl->Camera)
	{
		if (ButtonID == MapType_ID[0]) { // Map
			VPControl->DisplayMap = Checked;
		}
	}
	return LRESULT();
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
		
		if (VPControl->Scene->MeshReady()) VPControl->Camera->SetPosition(p.x, VPControl->Scene->GetY0() + (5000 - VPControl->Scene->GetY0())*(100 - val)/100/ VPControl->Scene->MPPv, p.z);
	}

	return LRESULT();
}
LRESULT CUserInterface::Trackbar_CameraDirection_Turn(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int ID = GetDlgCtrlID((HWND)lParam);

	int val = SendMessage(GetDlgItem(hwnd, ID), TBM_GETPOS, 0, 0);

	SetDlgItemText(ParentHWND, CameraDirectionValue_ID[1], std::to_wstring(val).c_str());
	
	if (VPControl && VPControl->Camera)
		VPControl->Camera->Direction = GetDirection();

	return LRESULT();
}

LRESULT CUserInterface::Trackbar_BegAzm(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	/*int ID = GetDlgCtrlID((HWND)lParam);

	int val = SendMessage(GetDlgItem(hwnd, ID), TBM_GETPOS, 0, 0);*/

	if (VPControl && VPControl->Scene)
	{
		VPControl->Scene->SetBegAzm(glm::radians(GetBegAzm()));
	}
	Trackbar_BegAzm_SetText(BegAzmValue_ID);	
	return LRESULT();
}

void CUserInterface::Trackbar_BegAzm_SetText(int labelID)
{
	float val = GetBegAzm();
	wstringstream stream;
	stream << fixed << setprecision(2) << val;
	wstring s = stream.str();
	SetDlgItemText(ParentHWND, BegAzmValue_ID, s.c_str());
}

LRESULT CUserInterface::Trackbar_ZeroElevation(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CSettings::SetFloat(FloatZeroElevation, GetZeroElevation());
	CSettings::SetFloat(FloatCTrackRefresh_e0, GetZeroElevation());
	Trackbar_ZeroElevation_SetText(ZeroElevationValue_ID);
	return LRESULT();
}

void CUserInterface::Trackbar_ZeroElevation_SetText(int labelID)
{
	float val = GetZeroElevation();
	wstringstream stream;
	stream << fixed << setprecision(2) << val;
	wstring s = stream.str();
	SetDlgItemText(ParentHWND, ZeroElevationValue_ID, s.c_str());
}

//TRACKBAR_CLASS
CUserInterface::CUserInterface(HWND parentHWND, CViewPortControl *vpControl, CRCSocket *socket, int panelWidth)
{
	string context = "CUserInterface::CUserInterface";
	LOG_INFO(requestID, context, (boost::format("Start... parentHWND=%1%, vpControl=%2%, socket=%3%, panelWidth=%4%...") % parentHWND % vpControl % socket % panelWidth).str().c_str());

	this->ParentHWND = parentHWND;
	hInstance = HINSTANCE(GetWindowLong(parentHWND, GWL_HINSTANCE));
	this->VPControl = vpControl;
	this->Socket = socket;

	PanelWidth = panelWidth;
	Column2X = panelWidth / 3;
	Column3X = Column2X * 2;
	MinimapSize = PanelWidth - VStepGrp;
	ControlWidth = Column2X - VStepGrp - VStep;
	ControlWidthL = Column2X - VStep;
	ControlWidthXL = Column3X;

	int CurrentY = 0;

	hgridmod = LoadLibrary(L"ZeeGrid.dll");
	if (!hgridmod)
	{
		std::string error_string = "Unable to load ZeeGrid.DLL";
		LOG_ERROR(requestID, context, error_string.c_str());
		throw std::exception(error_string.c_str());
	}

	//new view port control should be here

	CameraDirection_ID[0] = InsertElement(NULL, TRACKBAR_CLASS, _T("Высота камеры:"), WS_VISIBLE | WS_CHILD | TBS_VERT | TBS_RIGHT, MinimapSize, CurrentY, VStepGrp, MinimapSize, &CUserInterface::Trackbar_CameraDirection_VTilt);
	CurrentY += MinimapSize;
	CameraDirection_ID[1] = InsertElement(NULL, TRACKBAR_CLASS, _T("Поворот камеры:"), WS_VISIBLE | WS_CHILD | TBS_HORZ | TBS_BOTTOM, 0, CurrentY, MinimapSize, VStepGrp, &CUserInterface::Trackbar_CameraDirection_Turn);

	CurrentY += VStep+VStep;

	//FixViewToRadar_ID = InsertElement(NULL, _T("BUTTON"), _T("Вид на радар"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX, Column1X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::Checkbox_FixViewToRadar);

	MeasureDistance_ID = InsertElement(NULL, _T("BUTTON"), _T("Измерения"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX, Column2X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::Checkbox_MeasureDistance);
	
	BtnColors_ID = InsertElement(NULL, _T("BUTTON"), _T("Цвета"), WS_TABSTOP | WS_VISIBLE | WS_CHILD, Column3X, CurrentY, ControlWidth/2, ButtonHeight, &CUserInterface::Button_Colors);
	BtnLoad_ID = InsertElement(NULL, _T("BUTTON"), _T("Загр. карту"), WS_TABSTOP | WS_VISIBLE | WS_CHILD, Column3X + ControlWidth / 2 + Column1X / 2, CurrentY, ControlWidth/4 * 3, ButtonHeight, &CUserInterface::Button_Load);

	CurrentY += VStepGrp;


	Button_Connect_ID = InsertElement(NULL, _T("BUTTON"), _T("Cоединить"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, Column1X, CurrentY, ControlWidthL, ButtonHeight, &CUserInterface::Button_Connect);
	IsConnected_ID = InsertElement(NULL, _T("STATIC"), _T("Нет соединения"), WS_VISIBLE | WS_CHILD, Column2X, CurrentY, ControlWidthL, ButtonHeight, NULL);
	BtnTest_ID = InsertElement(NULL, _T("BUTTON"), _T("Тест"), WS_TABSTOP | WS_VISIBLE | WS_CHILD, Column3X, CurrentY, ControlWidth / 2, ButtonHeight, &CUserInterface::Button_Test);

	
	CurrentY += VStepGrp;

	InsertElement(NULL, _T("STATIC"), _T("Выводить:"), WS_VISIBLE | WS_CHILD, Column1X, CurrentY, ControlWidth, ButtonHeight, NULL);
	InsertElement(NULL, _T("STATIC"), _T("Вид местности:"), WS_VISIBLE | WS_CHILD, Column2X, CurrentY, ControlWidth, ButtonHeight, NULL);
	InsertElement(NULL, _T("STATIC"), _T("Разметка:"), WS_VISIBLE | WS_CHILD, Column3X, CurrentY, ControlWidth, ButtonHeight, NULL);
	CurrentY += VStep;
	ObjOptions_ID[0] = InsertElement(NULL, _T("BUTTON"), _T("Точки"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX, Column1X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::Checkbox_ObjOptions);
	
	// Ландшафт
	MapOptions_ID[0] = InsertElement(NULL, _T("BUTTON"), _T("Ландшафт"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX, Column2X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::Checkbox_MapOptions);

	MarkupOptions_ID[0] = InsertElement(NULL, _T("BUTTON"), _T("Линии"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX, Column3X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::Checkbox_MarkupOptions);
	CurrentY += VStep;
	ObjOptions_ID[1] = InsertElement(NULL, _T("BUTTON"), _T("Траектории"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX, Column1X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::Checkbox_ObjOptions);

	// Карта
	MapType_ID[0] = InsertElement(NULL, _T("BUTTON"), _T("Карта"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX | WS_GROUP, Column2X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::Checkbox_MapType);


	MarkupOptions_ID[1] = InsertElement(NULL, _T("BUTTON"), _T("Числа"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX, Column3X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::Checkbox_MarkupOptions);
	CurrentY += VStep;
	ObjOptions_ID[2] = InsertElement(NULL, _T("BUTTON"), _T("RLI-изображения"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX, Column1X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::Checkbox_ObjOptions);
	
	// Слепые зоны
	MapType_ID[1] = InsertElement(NULL, _T("BUTTON"), _T("Слепые зоны"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX, Column2X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::Checkbox_BlindZones);

	CurrentY += VStepGrp;

	InsertElement(NULL, _T("STATIC"), _T("Начальный азимут"), WS_VISIBLE | WS_CHILD, Column2X, CurrentY, ControlWidth, ButtonHeight, NULL);
	BegAzm_ID = InsertElement(NULL, TRACKBAR_CLASS, _T("Положение камеры:"), WS_VISIBLE | WS_CHILD | TBS_HORZ | TBS_BOTTOM, Column2X + ControlWidth + VStep / 2, CurrentY, ControlWidth, VStepGrp, &CUserInterface::Trackbar_BegAzm);
	BegAzmValue_ID = InsertElement(NULL, _T("STATIC"), _T("BegAzmValue_ID"), WS_VISIBLE | WS_CHILD, Column2X + 2 * ControlWidth + VStep, CurrentY, ControlWidth / 2, ButtonHeight, NULL);

	InsertElement(NULL, _T("STATIC"), _T("Положение камеры:"), WS_VISIBLE | WS_CHILD, Column1X, CurrentY, ControlWidth, ButtonHeight, NULL);
	CameraPosition_ID[0] = InsertElement(NULL, _T("BUTTON"), _T("От радара"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP, Column1X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::RadioGroup_CameraPosition);
		
	

	CurrentY += VStep;
	InsertElement(NULL, _T("STATIC"), _T("Начальный угол места"), WS_VISIBLE | WS_CHILD, Column2X, CurrentY, ControlWidth, ButtonHeight, NULL);
	ZeroElevation_ID = InsertElement(NULL, TRACKBAR_CLASS, _T("Положение камеры:"), WS_VISIBLE | WS_CHILD | TBS_HORZ | TBS_BOTTOM, Column2X + ControlWidth + VStep / 2, CurrentY, ControlWidth, VStepGrp, &CUserInterface::Trackbar_ZeroElevation);
	ZeroElevationValue_ID = InsertElement(NULL, _T("STATIC"), _T("ZeroElevationValue_ID"), WS_VISIBLE | WS_CHILD, Column2X + 2 * ControlWidth + VStep, CurrentY, ControlWidth / 2, ButtonHeight, NULL);

	CameraPosition_ID[1] = InsertElement(NULL, _T("BUTTON"), _T("100м над радаром"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, Column1X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::RadioGroup_CameraPosition);
	

	CurrentY += VStep;
	
	CameraPosition_ID[2] = InsertElement(NULL, _T("BUTTON"), _T("1км над радаром"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, Column1X, CurrentY, ControlWidth, ButtonHeight, &CUserInterface::RadioGroup_CameraPosition);
	CurrentY += VStep;
	
	int gridX = panelWidth;
	int gridY = vpControl->GetHeight();
	RECT clientRect;
	GetClientRect(parentHWND, &clientRect);


	Grid_ID = InsertElement(NULL, _T("ZeeGrid"), _T("Список траекторий"), WS_BORDER | WS_TABSTOP | WS_VISIBLE | WS_CHILD, gridX, gridY, 2 * vpControl->GetWidth() / 3, clientRect.right - panelWidth, &CUserInterface::Grid);
	ColorGrid_ID = InsertElement(NULL, _T("ZeeGrid"), _T("Цвета"), WS_BORDER | WS_TABSTOP | WS_CHILD, gridX, gridY, 2 * vpControl->GetWidth() / 3, clientRect.right - panelWidth, &CUserInterface::ColorGrid);
	InfoGrid_ID = InsertElement(NULL, _T("ZeeGrid"), _T("Информация"), WS_BORDER | WS_TABSTOP | WS_VISIBLE | WS_CHILD, gridX + 2 * vpControl->GetWidth() / 3 + 2, gridY, vpControl->GetWidth() / 3 - 2, clientRect.right - panelWidth, &CUserInterface::InfoGrid);

	
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
			GetModuleHandle(nullptr),
			this);
	}

	Elements.insert({ IDD_DIALOG1, new InterfaceElement{ IDD_DIALOG1, NULL, _T("IDD_DIALOG1"), _T("IDD_DIALOG1"), DS_SETFONT | DS_FIXEDSYS | DS_CONTROL | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, nullptr, &CUserInterface::IDD_DIALOG1_Proc } });
	Elements.insert({ IDC_CHECK1, new InterfaceElement{ IDC_CHECK1, NULL, _T("IDC_CHECK1"), _T("IDC_CHECK1"), TBS_BOTH | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0, nullptr, &CUserInterface::Checkbox_FixViewToRadar } });
	
		
	Elements.insert({ IDC_CHECK2, new InterfaceElement{ IDC_CHECK2, NULL, _T("IDC_CHECK2"), _T("IDC_CHECK2"), TBS_BOTH | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0, nullptr, &CUserInterface::Checkbox_MeasureDistance } });
	Elements.insert({ IDC_BUTTON2, new InterfaceElement{ IDC_BUTTON2, NULL, _T("IDC_BUTTON2"), _T("IDC_BUTTON2"), TBS_BOTH | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0, nullptr, &CUserInterface::Button_Settings } });
	


	SetChecked(ObjOptions_ID[0], 0);
	SetChecked(ObjOptions_ID[1], 1);
	SetChecked(ObjOptions_ID[2], 1);
	SetChecked(MapOptions_ID[0], vpControl->DisplayLandscape);
	SetChecked(MapType_ID[0], vpControl->DisplayMap);
	SetChecked(MapType_ID[1], vpControl->DisplayBlindZones);
	SetChecked(MarkupOptions_ID[0], 1);
	SetChecked(MarkupOptions_ID[1], 1);

	SendMessage(GetDlgItem(parentHWND, CameraDirection_ID[0]), TBM_SETPOS, 1, 50);
	SendMessage(GetDlgItem(parentHWND, CameraDirection_ID[1]), TBM_SETPOS, 1, 50);

	Font = GetFont();

	for (ElementsMap::iterator it = Elements.begin(); it != Elements.end(); ++it) {
		SendMessage(it->second->hWnd, WM_SETFONT, (WPARAM)Font, TRUE);
	}

	GridHWND = InfoGridHWND = nullptr;
	InitGrid();

	Trackbar_BegAzm_SetText(BegAzmValue_ID);
	Trackbar_ZeroElevation_SetText(ZeroElevationValue_ID);
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
	std::string context = "CUserInterface::ConnectionStateChanged";
	if (IsConnected) 
	{
		SetDlgItemText(ParentHWND, IsConnected_ID, _T("Есть коннект"));
		SetDlgItemText(ParentHWND, Button_Connect_ID, _T("Стоп"));
	}
	else 
	{
		SetDlgItemText(ParentHWND, IsConnected_ID, _T("Нет коннекта"));
		SetDlgItemText(ParentHWND, Button_Connect_ID, _T("Соединить"));
	}
}

bool CUserInterface::GetCheckboxState_Map()
{
	bool res = Button_GetCheck(GetDlgItem(ParentHWND, MapType_ID[0]));
	return res;
}

bool CUserInterface::GetCheckboxState_BlindZones()
{
	bool res = Button_GetCheck(GetDlgItem(ParentHWND, MapType_ID[1]));
	return res;
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

int CUserInterface::GetTrackbarValue(int id) const
{
	HWND hwnd = GetDlgItem(ParentHWND, id);
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

void CUserInterface::SetTrackbarValue(int id, int val) const
{
	SendMessage(GetDlgItem(ParentHWND, id), TBM_SETPOS, 1, val);
}

void CUserInterface::SetTrackbarValue_BegAzm(int val)
{
	SetTrackbarValue(BegAzm_ID, val);
	Trackbar_BegAzm_SetText(BegAzmValue_ID);
}

void CUserInterface::SetTrackbarValue_ZeroElevation(int val)
{
	SetTrackbarValue(ZeroElevation_ID, val);
	Trackbar_ZeroElevation_SetText(ZeroElevationValue_ID);
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
	MoveWindow(gridHwnd, PanelWidth, VPControl->GetHeight() + 2, 2* VPControl->GetWidth() / 3, clientRect.bottom - VPControl->GetHeight() - 2, TRUE);

	gridHwnd = GetDlgItem(ParentHWND, InfoGrid_ID);
	MoveWindow(gridHwnd, PanelWidth + 2* VPControl->GetWidth() / 3 + 2, VPControl->GetHeight() + 2, VPControl->GetWidth() / 3 - 2, clientRect.bottom - VPControl->GetHeight() - 2, TRUE);

	gridHwnd = GetDlgItem(ParentHWND, ColorGrid_ID);
	MoveWindow(gridHwnd, PanelWidth, VPControl->GetHeight() + 2, 2 * VPControl->GetWidth() / 3, clientRect.bottom - VPControl->GetHeight() - 2, TRUE);
	//SetWindowPos(gridHwnd, NULL, PanelWidth, VPControl->Height + 2, VPControl->Width, clientRect.bottom - VPControl->Height - 2, 0);
}

void CUserInterface::InitGrid()
{
	if (!GridHWND)
		GridHWND = GetDlgItem(ParentHWND, Grid_ID);
	if (!InfoGridHWND)
		InfoGridHWND = GetDlgItem(ParentHWND, InfoGrid_ID);

	

	SendMessage(GridHWND, ZGM_SETFONT, 0, (LPARAM)Font);
	SendMessage(GridHWND, ZGM_DIMGRID, 7, 0);
	SendMessage(GridHWND, ZGM_SHOWROWNUMBERS, TRUE, 0);

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
		if (i > nrows)
		{
			SendMessage(GridHWND, ZGM_APPENDROW, 0, 0);
		}
		int offset = ncols*(i + 1);
		GRIDCELL(GridHWND, offset + 1) = tracks->at(i)->id;

		npoints = tracks->at(i)->P.size();
		GRIDCELL(GridHWND, offset + 2) = npoints;

		GRIDCELL(GridHWND, offset + 3) = format("%.4f,%.4f", tracks->at(i)->P.at(0)->X, tracks->at(i)->P.at(0)->Y);
		
		GRIDCELL(GridHWND, offset + 4) = format("%.4f,%.4f", tracks->at(i)->P.at(npoints - 1)->X, tracks->at(i)->P.at(npoints - 1)->Y);

		glm::vec3 speed(tracks->at(i)->P.at(npoints - 1)->vX, tracks->at(i)->P.at(npoints - 1)->vY, tracks->at(i)->P.at(npoints - 1)->vZ);
		GRIDCELL(GridHWND, offset + 5) = format("%.4f", glm::length(speed));
	}
	for (; i < nrows; i++) {
		SendMessage(GridHWND, ZGM_DELETEROW, i, 0);
	}
	SendMessage(GridHWND, ZGM_AUTOSIZE_ALL_COLUMNS, 0, 0);
}

void CUserInterface::FillInfoGrid(CScene* scene)
{
	std::string context = "CUserInterface::FillInfoGrid";
	if (!scene)
	{
		LOG_WARN__("Called with parameter scene=nullptr");
		return;
	}
	if (!InfoGridHWND)
	{
		InfoGridHWND = GetDlgItem(ParentHWND, InfoGrid_ID);
	}

	int nrows = SendMessage(InfoGridHWND, ZGM_GETROWS, 0, 0);
	int ncols = SendMessage(InfoGridHWND, ZGM_GETCOLS, 0, 0);

	SendMessage(InfoGridHWND, ZGM_EMPTYGRID, 1, 0);
	
	std::stringstream ss;
	int r = 1;
	//SendMessage(InfoGridHWND, ZGM_SETCELLTEXT, ncols * r + 1, formatmsg("Местоположение радара"));

	//*(&(CRCGridCell::CRCGridCell(InfoGridHWND, ncols, r, 1))) = "test";

	GRIDCELL(InfoGridHWND, ncols * r + 1) = "Местоположение радара";
	GRIDCELL(InfoGridHWND, ncols * r + 2) = format("%.4f, %.4f", scene->position.x, scene->position.y);

	if (scene->Socket->IsConnected) {
		r++;
		GRIDCELL(InfoGridHWND, ncols * r + 1) = "Угол места, от";
		GRIDCELL(InfoGridHWND, ncols * r + 2) = format("%.4f", scene->minE);
		r++;
		GRIDCELL(InfoGridHWND, ncols * r + 1) = "Угол места, до";
		GRIDCELL(InfoGridHWND, ncols * r + 2) = format("%.4f", scene->maxE);
	}

	if (scene->Selection.size()>0)
	{
		C3DObjectModel *last = scene->Selection.at(scene->Selection.size() - 1);

		glm::vec3 geoCoords = last->GetGeoCoords();

		r++;
		SendMessage(InfoGridHWND, ZGM_SETCELLTEXT, ncols * r + 1, (LPARAM)"Точка ");
		ss.str(std::string());
		ss << std::fixed << std::setprecision(4) << geoCoords.x << ", " << geoCoords.y;
		SendMessage(InfoGridHWND, ZGM_SETCELLTEXT, ncols * r + 2, (LPARAM)ss.str().c_str());
	}

	SendMessage(InfoGridHWND, ZGM_AUTOSIZE_ALL_COLUMNS, 0, 0);
}

bool CUserInterface::MeasureDistance() const
{
	HWND hWnd = GetDlgItem(ParentHWND, MeasureDistance_ID);
	return Button_GetCheck(hWnd);
}

int CUserInterface::GetMainTableMode() const
{
	return mainTableMode;
}

void CUserInterface::SetMainTableMode(int value)
{
	mainTableMode = value;
}