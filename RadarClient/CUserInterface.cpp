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
HWND CUserInterface::ParentHWND;
HWND CUserInterface::ToolboxHWND;
CXColorSpectrumCtrl CUserInterface::m_ColorSpectrum;
CScene * CUserInterface::Scene;

bool CUserInterface::_checkboxState_Points = true;
bool CUserInterface::_checkboxState_PointsFadeout = false;
bool CUserInterface::_checkboxState_Tracks = true;
bool CUserInterface::_checkboxState_TracksFadeout = false;

bool CUserInterface::_checkboxState_Images = true;
bool CUserInterface::_checkboxState_ImagesFadeout = false;

bool CUserInterface::_checkboxState_Map = false;
bool CUserInterface::_checkboxState_BlindZones = false;
bool CUserInterface::_checkboxState_AltitudeMap = false;
bool CUserInterface::_checkboxState_MarkupLines = true;
bool CUserInterface::_checkboxState_MarkupLabels = true;
bool CUserInterface::_checkboxState_ViewFromTop = false;
bool CUserInterface::_checkboxState_MeasureDistance = false;

LRESULT Button1_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	return 0;
}

//in this method ID is retrieved as LOWORD(wParam)
LRESULT CUserInterface::Wnd_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
	{
		ToolboxHWND = hwnd;
		OnInitDialog();
		return LRESULT();
	}
	CUserInterface *ui = this;
	if (!ui) 
	{
		return NULL;
	}
	WORD ctrlID = 0;
	if (uMsg == WM_COMMAND)
	{
		ctrlID = LOWORD(wParam);
	}
	if (uMsg == WM_HSCROLL || uMsg == WM_VSCROLL)
	{
		ctrlID = GetDlgCtrlID((HWND)lParam);
	}
	
	for (ElementsMap::iterator it = Elements.begin(); it != Elements.end(); ++it) 
	{
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

LRESULT CUserInterface::Button_CameraReset(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (VPControl && VPControl->Camera)
	{
		VPControl->Camera->Reset();
	}

	return LRESULT();
}

LRESULT CUserInterface::Button_Test(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	const clock_t begin_time = clock();

//	boost::detail::win32::sleep(1000);

	CSector::flight_start_stop(true);

	LOG_INFO("Test", "Test", "Finished, time=%f", float(clock() - begin_time) / CLOCKS_PER_SEC);

	return LRESULT();
}

HWND CUserInterface::SettingsHWND = nullptr;
LRESULT CUserInterface::Button_Settings(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	/*HWND hwnd1 = RCDialog(hInstance, IDD_DIALOG4, nullptr, DLGPROC(&CUserInterface::Dialog_Settings));
	if (hwnd1)
	{
		ShowWindow(hwnd1, SW_SHOW);
	}*/
	SettingsHWND = CreateDialog(hInstance,
		MAKEINTRESOURCE(IDD_DIALOG4),
		hwnd,
		(DLGPROC)&CUserInterface::Dialog_Settings);
	ShowWindow(SettingsHWND, SW_SHOW);

	//auto ret = DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG4), ParentHWND, DLGPROC(&CUserInterface::Dialog_Settings));
	return LRESULT();
}

int CUserInterface::iItem_ColorListView = -1; 
int CUserInterface::iItem_DistancesListView = -1;
int CUserInterface::iItem_RImageColorListView = -1;
glm::vec4 CUserInterface::oldColor_ColorListView = glm::vec4(0, 0, 0, 0);
POINT CUserInterface::rclick_point;
float CUserInterface::rclick_value;
int CUserInterface::rclick = 0;
POINT CUserInterface::prev_point;
RECT CUserInterface::settings_dialog_rect_;
LRESULT CALLBACK CUserInterface::Dialog_Settings(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	std::string context("Dialog_Settings");

	if ((HIWORD(wParam) == EN_CHANGE) && //notification
		(LOWORD(wParam) == IDC_EDIT3))   //your edit control ID
	{
		auto hw = GetDlgItem(hDlg, IDC_EDIT3);
		auto val = float(GetDoubleValue(hw));
		auto index = CSettings::GetIndex(GetDistanceForSettingsDialog(iItem_DistancesListView));
		if (val != CSettings::GetFloat(index))
		{
			CSettings::SetFloat(Settings(index), val);
			auto hwndListView = GetDlgItem(hDlg, IDC_LIST2);
			ListView_RedrawItems(hwndListView, iItem_DistancesListView, iItem_DistancesListView);
		}
	}

	if ((HIWORD(wParam) == EN_CHANGE) && //notification
		(LOWORD(wParam) == IDC_EDIT_RIMAGE_COLOR))   //your edit control ID
	{
		auto hw = GetDlgItem(hDlg, IDC_EDIT_RIMAGE_COLOR);
		auto val = float(GetDoubleValue(hw));
		auto index = CSettings::GetIndex(GetCRImageColorForSettingsDialog(iItem_RImageColorListView));
		if (val != CSettings::GetFloat(index))
		{
			CSettings::SetFloat(Settings(index), val);
			CSector::RefreshColorSettings();
			auto hwndListView = GetDlgItem(hDlg, IDC_LIST_RIMAGE_COLOR);
			ListView_RedrawItems(hwndListView, iItem_RImageColorListView, iItem_RImageColorListView);
		}
	}
	switch (uMsg)
	{
	case WM_INITDIALOG: {
		if (CUserInterface_Dialog_Settings_Log) LOG_INFO__("WM_INITDIALOG");
		GetWindowRect(hDlg, &settings_dialog_rect_);

		//colors:

		auto hwndListView = GetDlgItem(hDlg, IDC_LIST1);
		CRCListView::InitListView(hwndListView, 15, InitColorListViewColumns);
		RECT rect;
		GetClientRect(hwndListView, &rect);
		ListView_SetExtendedListViewStyle(hwndListView, LVS_EX_FULLROWSELECT);
		ListView_SetColumnWidth(hwndListView, 0, 150);
		ListView_SetColumnWidth(hwndListView, 1, 70);
		ListView_SetColumnWidth(hwndListView, 2, rect.right - 220);
		

		//distances:
		hwndListView = GetDlgItem(hDlg, IDC_LIST2);
		CRCListView::InitListView(hwndListView, 15, InitDistanceListViewColumns);
		GetClientRect(hwndListView, &rect);
		ListView_SetExtendedListViewStyle(hwndListView, LVS_EX_FULLROWSELECT);
		ListView_SetColumnWidth(hwndListView, 0, 50);
		ListView_SetColumnWidth(hwndListView, 1, rect.right - 50);

		//rimage colors:
		hwndListView = GetDlgItem(hDlg, IDC_LIST_RIMAGE_COLOR);
		CRCListView::InitListView(hwndListView, 15, InitCRImageColorListViewColumns);
		GetClientRect(hwndListView, &rect);
		ListView_SetExtendedListViewStyle(hwndListView, LVS_EX_FULLROWSELECT);
		ListView_SetColumnWidth(hwndListView, 0, 50);
		ListView_SetColumnWidth(hwndListView, 1, rect.right - 50);

		//URL combobox:
		auto hWndComboBox = GetDlgItem(hDlg, IDC_COMBO1);
		TCHAR urls[2][16] =
		{
			TEXT("localhost"), TEXT("rloc")
		};

		TCHAR A[16];
		int  k = 0;

		memset(&A, 0, sizeof(A));
		for (k = 0; k < 2; k ++)
		{
			wcscpy_s(A, sizeof(A) / sizeof(TCHAR), (TCHAR*)urls[k]);

			// Add string to combobox.
			SendMessage(hWndComboBox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)A);
		}

		// Send the CB_SETCURSEL message to display an initial item 
		//  in the selection field  
		SendMessage(hWndComboBox, CB_SETCURSEL, (WPARAM)2, (LPARAM)0);

		return true;
	}
		break;

	case WM_CTLCOLORSTATIC:
		if (CUserInterface_Dialog_Settings_Log) LOG_INFO__("WM_CTLCOLORSTATIC");
		break;
	case WM_COMMAND:
		if (CUserInterface_Dialog_Settings_Log) LOG_INFO__("WM_COMMAND");
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			if (LOWORD(wParam) == IDOK)
			{
				CSettings::Save();
			}			
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		if (LOWORD(wParam) == IDC_BUTTON1)
		{
			if (Scene)
			{
				Scene->Dump();
			}
		}
		break;
	case WM_SIZE:
		if (CUserInterface_Dialog_Settings_Log) LOG_INFO__("WM_SIZE");
		CRCListView::ResizeListView(GetDlgItem(hDlg, IDC_LIST1), hDlg);
		break;
	case WM_NOTIFY:
	{
		//if (CUserInterface_Dialog_Settings_Log) LOG_INFO__("WM_NOTIFY");
		if (((LPNMHDR)lParam)->idFrom == IDC_LIST1) // Colors
		{
			if (((LPNMHDR)lParam)->code == NM_CUSTOMDRAW) {
				SetWindowLong(hDlg, DWL_MSGRESULT,
					(LONG)ProcessColorListViewCustomDraw(lParam));
				return TRUE;
			}
			if (((LPNMHDR)lParam)->code == NM_DBLCLK)
			{
				LPNMITEMACTIVATE item = LPNMITEMACTIVATE(lParam);

				iItem_ColorListView = item->iItem;
				oldColor_ColorListView = CSettings::GetColor(CSettings::GetIndex(GetColorForSettingsDialog(iItem_ColorListView)));
				//item->iItem

				auto ret = DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG3), hDlg, DLGPROC(&CUserInterface::Dialog_SelectColor));
				if (LOWORD(ret) == IDCANCEL)
				{
					CSettings::SetColor((Settings)CSettings::GetIndex(GetColorForSettingsDialog(iItem_ColorListView)), oldColor_ColorListView);					
				}
				CSettings::Save();
				return LRESULT();
			}
			return CRCListView::ListViewNotify(hDlg, lParam, IDC_LIST1, GetColorListViewCellText);
		}
		if (((LPNMHDR)lParam)->idFrom == IDC_LIST2) // distances
		{
			auto tmp = LPNMHDR(lParam);			
			
			//LOG_INFO__("IDC_LIST2 %d %d %d %d", NM_RDOWN, tmp->code, tmp->idFrom, tmp->hwndFrom);

			if (((LPNMHDR)lParam)->code == LVN_BEGINRDRAG)
			{
				LPNMITEMACTIVATE item = LPNMITEMACTIVATE(lParam);				
				iItem_DistancesListView = item->iItem;
				if (GetCursorPos(&rclick_point))
				{
					prev_point = rclick_point;
					rclick_value = CSettings::GetFloat(CSettings::GetIndex(GetDistanceForSettingsDialog(iItem_DistancesListView)));
					rclick = 1;
					//LOG_INFO__("IDC_LIST2 LVN_BEGINRDRAG code=%d, iItem=%d, rclick_point=(%d, %d)", ((LPNMHDR)lParam)->code, iItem_DistancesListView, rclick_point.x, rclick_point.y);
					//cursor position now in p.x and p.y
				}
				SetCapture(hDlg);
			}
			if (((LPNMHDR)lParam)->code == NM_CLICK)
			{
				LPNMITEMACTIVATE item = LPNMITEMACTIVATE(lParam);
				iItem_DistancesListView = item->iItem;
				LOG_INFO__("IDC_LIST2 LVN_ITEMACTIVATE iItem=%d", iItem_DistancesListView);
				auto editHwnd = GetDlgItem(hDlg, IDC_EDIT3);
				SetDoubleValue(editHwnd, CSettings::GetFloat(CSettings::GetIndex(GetDistanceForSettingsDialog(iItem_DistancesListView))));
			}
			
			return CRCListView::ListViewNotify(hDlg, lParam, IDC_LIST2, GetDistanceListViewCellText);
		}
		if (((LPNMHDR)lParam)->idFrom == IDC_LIST_RIMAGE_COLOR) // 
		{
			auto tmp = LPNMHDR(lParam);

			//LOG_INFO__("IDC_LIST2 %d %d %d %d", NM_RDOWN, tmp->code, tmp->idFrom, tmp->hwndFrom);

			if (((LPNMHDR)lParam)->code == LVN_BEGINRDRAG)
			{
				LPNMITEMACTIVATE item = LPNMITEMACTIVATE(lParam);
				iItem_RImageColorListView = item->iItem;
				if (GetCursorPos(&rclick_point))
				{
					prev_point = rclick_point;
					rclick_value = CSettings::GetFloat(CSettings::GetIndex(GetCRImageColorForSettingsDialog(iItem_RImageColorListView)));
					rclick = 1;
					//LOG_INFO__("IDC_LIST2 LVN_BEGINRDRAG code=%d, iItem=%d, rclick_point=(%d, %d)", ((LPNMHDR)lParam)->code, iItem_DistancesListView, rclick_point.x, rclick_point.y);
					//cursor position now in p.x and p.y
				}
				SetCapture(hDlg);
			}
			if (((LPNMHDR)lParam)->code == NM_CLICK)
			{
				LPNMITEMACTIVATE item = LPNMITEMACTIVATE(lParam);
				iItem_RImageColorListView = item->iItem;
				LOG_INFO__("IDC_LIST_RIMAGE_COLOR LVN_ITEMACTIVATE iItem=%d", iItem_RImageColorListView);
				auto editHwnd = GetDlgItem(hDlg, IDC_EDIT_RIMAGE_COLOR);
				SetDoubleValue(editHwnd, CSettings::GetFloat(CSettings::GetIndex(GetCRImageColorForSettingsDialog(iItem_RImageColorListView))));
			}

			return CRCListView::ListViewNotify(hDlg, lParam, IDC_LIST_RIMAGE_COLOR, GetCRImageColorListViewCellText);
		}
	}
	case WM_MOUSEMOVE:
		//if (CUserInterface_Dialog_Settings_Log) LOG_INFO__("WM_MOUSEMOVE");
		POINT p;
		GetCursorPos(&p);
		if (rclick && iItem_DistancesListView>=0)
		{
			auto d_value = - rclick_value * (p.y - rclick_point.y) / (settings_dialog_rect_.bottom - settings_dialog_rect_.top);
			CSettings::SetFloat((Settings)CSettings::GetIndex(GetDistanceForSettingsDialog(iItem_DistancesListView)), rclick_value + d_value);
			prev_point = p;
			ListView_RedrawItems(GetDlgItem(hDlg, IDC_LIST2), iItem_DistancesListView, iItem_DistancesListView);
		}
		if (rclick && iItem_RImageColorListView >= 0)
		{
			auto d_value = -rclick_value * (p.y - rclick_point.y) / (settings_dialog_rect_.bottom - settings_dialog_rect_.top);
			CSettings::SetFloat((Settings)CSettings::GetIndex(GetCRImageColorForSettingsDialog(iItem_RImageColorListView)), rclick_value + d_value);
			CSector::RefreshColorSettings();
			prev_point = p;
			ListView_RedrawItems(GetDlgItem(hDlg, IDC_LIST_RIMAGE_COLOR), iItem_RImageColorListView, iItem_RImageColorListView);
		}
	break;
	case WM_RBUTTONDOWN:
		if (CUserInterface_Dialog_Settings_Log) LOG_INFO__("WM_RBUTTONDOWN");
		//LOG_INFO__("WM_RBUTTONDOWN");

		break;
	case WM_CONTEXTMENU:
		if (CUserInterface_Dialog_Settings_Log) LOG_INFO__("WM_CONTEXTMENU");
		//LOG_INFO__("WM_CONTEXTMENU");
		iItem_DistancesListView = -1;
		iItem_RImageColorListView = -1;
		rclick = 0;
		ReleaseCapture();
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

tstring CUserInterface::GetDistanceForSettingsDialog(int index)
{
	if (index < distancesSettings.size())
		return distancesSettings[index];
	return  TEXT("");
}

tstring CUserInterface::GetDistanceListViewCellText(int iItem, int iSubItem)
{
	if (iSubItem == 0)
		return GetDistanceForSettingsDialog(iItem);
	if (iSubItem == 1)
		return to_tstring(to_string( CSettings::GetFloat(CSettings::GetIndex(GetDistanceForSettingsDialog(iItem)))));
	return TEXT("");
}

void CUserInterface::InitDistanceListViewColumns(HWND hwndListView, LVCOLUMNW lvColumn)
{
	if (CUserInterface_InitDistanceListViewColumns_Log) LOG_INFO_("InitDistanceListViewColumns", "WM_CONTEXTMENU");
	TCHAR       szString[2][20] = { TEXT("��������"), TEXT("��������") };
	for (auto i = 0; i < 2; i++)
	{
		lvColumn.pszText = szString[i];
		ListView_InsertColumn(hwndListView, i, &lvColumn);
	}
}

tstring CUserInterface::GetCRImageColorForSettingsDialog(int index)
{
	if (index < crimagecolorSettings.size())
		return crimagecolorSettings[index];
	return  TEXT("");
}

tstring CUserInterface::GetCRImageColorListViewCellText(int iItem, int iSubItem)
{
	if (iSubItem == 0)
		return GetCRImageColorForSettingsDialog(iItem);
	if (iSubItem == 1)
		return to_tstring(to_string(CSettings::GetFloat(CSettings::GetIndex(GetCRImageColorForSettingsDialog(iItem)))));
	return TEXT("");
}

void CUserInterface::InitCRImageColorListViewColumns(HWND hwndListView, LVCOLUMNW lvColumn)
{
	if (CUserInterface_InitRImageColorListViewColumns_Log) LOG_INFO_("InitCRImageColorListViewColumns", "WM_CONTEXTMENU");
	TCHAR       szString[2][20] = { TEXT("��������"), TEXT("��������") };
	for (auto i = 0; i < 2; i++)
	{
		lvColumn.pszText = szString[i];
		ListView_InsertColumn(hwndListView, i, &lvColumn);
	}
}

LRESULT CUserInterface::Button_Reload(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:
		if (Scene && Scene->MeshReady())
		{
			Scene->Mesh->Refresh();
		}
		break;
	}
	return LRESULT();
}

LRESULT CUserInterface::Dialog_SelectColor(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_XCOLORPICKER_SELCHANGE || uMsg == WM_XCOLORPICKER_SELENDOK)
	{
		auto hEditRGB = GetDlgItem(hDlg, IDC_EDIT3);
		auto hEditHSL = GetDlgItem(hDlg, IDC_EDIT1);

		auto rgb = m_ColorSpectrum.GetRGB();
		auto a = int(oldColor_ColorListView.a * 255);
		SetWindowText(hEditRGB, HTMLColorFormat(GetRValue(rgb), GetGValue(rgb), GetBValue(rgb), a).c_str());

		BYTE h, s, l;
		m_ColorSpectrum.GetHSL(&h, &s, &l);

		SetWindowText(hEditHSL, (to_wstring(h) + TEXT(", ") + to_wstring(s) + TEXT(", ") + to_wstring(l)).c_str());

		CSettings::SetColor((Settings)CSettings::GetIndex(GetColorForSettingsDialog(iItem_ColorListView)), glm::vec4(GetRValue(rgb) / 255.0, GetGValue(rgb) / 255.0, GetBValue(rgb) / 255.0, a / 255.0));

		if (uMsg == WM_XCOLORPICKER_SELENDOK)
		{
			EndDialog(hDlg, LOWORD(TRUE));			
		}
		return true;
	}
	
	switch (uMsg)
	{
	case WM_INITDIALOG: {
		auto hWnd = GetDlgItem(hDlg, IDC_STATIC_COLOR_SPECTRUM);

		RECT rect;
		GetClientRect(hWnd, &rect);
		
		rect.left = 0;
		rect.top = 0;
		rect.right = rect.right - rect.left;
		rect.bottom = rect.bottom - rect.top;

		//ShowWindow(hWnd, SW_HIDE);         // hide placeholder
		m_ColorSpectrum.Create(hInstance,
			WS_CHILD | WS_VISIBLE | WS_TABSTOP,             // styles
			rect,                                           // control rect
			hDlg,                                         // parent window
			9001,                                           // control id
			RGB(0, 255, 0));                                  // initial color
			//CXColorSpectrumCtrl::XCOLOR_TOOLTIP_HTML);     // tooltip format

															// call SetWindowPos to insert control in proper place in tab order
		m_ColorSpectrum.SetRGB(RGB(255 * oldColor_ColorListView.r, 255 * oldColor_ColorListView.g, 255 * oldColor_ColorListView.b));
		SetWindowPos(m_ColorSpectrum.m_hWnd, hWnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		ShowWindow(m_ColorSpectrum.m_hWnd, SW_SHOW);
	}
						break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return INT_PTR(TRUE);
		}
		break;
	case WM_NOTIFY:
	{

	}
	break;
	}
	return false;
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
	if (index < colorSettings.size())
		return colorSettings[index];	
	return  TEXT("");
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

		lplvcd->clrTextBk = c;

		/*
		//uncomment to set font color

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
		}*/
	}
}

void CUserInterface::InitColorListViewColumns(HWND hwndListView, LV_COLUMN lvColumn)
{
	if (CUserInterface_InitColorListViewColumns_Log) LOG_INFO_("InitColorListViewColumns", "WM_CONTEXTMENU");
	TCHAR       szString[3][20] = { TEXT("��������"), TEXT("��������"), TEXT("����") };
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

LRESULT CUserInterface::LonLat(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	LOG_INFO("msgtest", "CUserInterface::LonLat", "hwnd=%d, uMsg=%d, LOWORD(wParam)=%d, HIWORD(wParam)=%d, lParam=%d", hwnd, uMsg, LOWORD(wParam), HIWORD(wParam), lParam);

	if ((HIWORD(wParam) == EN_CHANGE) && //notification
		(LOWORD(wParam) == IDC_EDIT_LON))   //your edit control ID
	{
		auto hw = GetDlgItem(hwnd, IDC_EDIT_LON);
		auto lon = float(GetDoubleValue(hw));
		if (lon != CSettings::GetFloat(FloatPositionLon))
		{
			CSettings::SetFloat(FloatPositionLon, lon);
			CSettings::Save();
		}
	}
	if ((HIWORD(wParam) == EN_CHANGE) && //notification
		(LOWORD(wParam) == IDC_EDIT_LAT))   //your edit control ID
	{
		auto hw = GetDlgItem(hwnd, IDC_EDIT_LAT);
		auto lat = float(GetDoubleValue(hw));
		if (lat != CSettings::GetFloat(FloatPositionLat))
		{
			CSettings::SetFloat(FloatPositionLat, lat);
			CSettings::Save();
		}
	}
	return LRESULT();
}

LRESULT CUserInterface::IDD_DIALOG1_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return LRESULT();
}

HWND CUserInterface::GetSettingsHWND() const
{
	return SettingsHWND;
}

void CUserInterface::SetChecked(HWND hwnd, int id, bool checked)
{
	HWND hWnd = GetDlgItem(hwnd, id);
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
	return CSettings::GetFloat(FloatMinBegAzm) + (CSettings::GetFloat(FloatMaxBegAzm) - CSettings::GetFloat(FloatMinBegAzm)) * GetTrackbarValue(ToolboxHWND, IDC_SLIDER1)/100.0;
}

float CUserInterface::GetZeroElevation()
{
	return CSettings::GetFloat(FloatMinZeroElevation) + (CSettings::GetFloat(FloatMaxZeroElevation) - CSettings::GetFloat(FloatMinZeroElevation)) * GetTrackbarValue(ToolboxHWND, IDC_SLIDER2) / 100.0;
}

float CUserInterface::GetHeight()
{
	return GetTrackbarValue_VTilt(); //here elevation counts from top-up direction
}

double CUserInterface::GetDoubleValue(HWND hw)
{
	if (!hw)
	{
		return 0;
		//throw std::exception("CUserInterface::GetDoubleValue - hw is nullptr");
	}
	auto l = GetWindowTextLength(hw) + 1;
	auto buf = new TCHAR[l];
	GetWindowText(hw, buf, l);
	TCHAR *stop;
	auto ret = _tcstod(buf, &stop);
	delete[] buf;
	return ret;
}

void CUserInterface::SetDoubleValue(HWND hw, double val)
{
	tstring t = to_tstring(to_string(val));
	SetWindowText(hw, t.c_str());
}

int CUserInterface::InsertElement(DWORD xStyle, LPCWSTR _class, LPCWSTR text, DWORD style, int x, int y, int width, int height, UIWndProc action)
{
	Elements.insert({ CurrentID, new InterfaceElement{ CurrentID, xStyle, _class, text, style, x, y, width, height, nullptr, action } });
	CurrentID++;
	return CurrentID-1;
}

void CUserInterface::InsertHandlerElement(int ID, UIWndProc Action)
{
	Elements.insert({ ID, new InterfaceElement{ ID, NULL, _T("IDC_BUTTON_CONNECT"), _T("IDC_BUTTON_CONNECT"), TBS_BOTH | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0, nullptr, &CUserInterface::Button_Connect } });

}

LRESULT CUserInterface::Button_Connect(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (Socket && !Socket->IsConnected)
		Socket->Connect();
	if (Socket && Socket->IsConnected)
		Socket->Close();	
	return LRESULT();
}

LRESULT CUserInterface::Checkbox_Points(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int ButtonID = LOWORD(wParam);
	HWND hWnd = GetDlgItem(hwnd, ButtonID);
	_checkboxState_Points = Button_GetCheck(hWnd);		
	return LRESULT();
}

LRESULT CUserInterface::Checkbox_PointsFadeout(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	_checkboxState_PointsFadeout = GetCheckboxState(LOWORD(wParam));
	return LRESULT();
}

LRESULT CUserInterface::Checkbox_Tracks(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	_checkboxState_Tracks = GetCheckboxState(LOWORD(wParam));
	return LRESULT();
}

LRESULT CUserInterface::Checkbox_TracksFadeout(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	_checkboxState_TracksFadeout = GetCheckboxState(LOWORD(wParam));
	return LRESULT();
}

LRESULT CUserInterface::Checkbox_Images(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	_checkboxState_Images = GetCheckboxState(LOWORD(wParam));
	return LRESULT();
}

LRESULT CUserInterface::Checkbox_ImagesFadeout(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	_checkboxState_ImagesFadeout = GetCheckboxState(LOWORD(wParam));
	return LRESULT();
}

LRESULT CUserInterface::Checkbox_AltitudeMap(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	_checkboxState_AltitudeMap = GetCheckboxState(LOWORD(wParam));
	return LRESULT();
}

LRESULT CUserInterface::Checkbox_Map(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	_checkboxState_Map = GetCheckboxState(LOWORD(wParam));
	return LRESULT();
}

LRESULT CUserInterface::Checkbox_ObjOptions(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// points, tracks, rli's...
	return LRESULT();
}

LRESULT CUserInterface::Checkbox_MapOptions(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return LRESULT();
}

LRESULT CUserInterface::Checkbox_MarkupOptions(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return LRESULT();
}

LRESULT CUserInterface::Checkbox_ViewFromTop(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int ButtonID = LOWORD(wParam);
	HWND hWnd = GetDlgItem(hwnd, ButtonID);
	if (VPControl && VPControl->Camera)
	{
		if (Button_GetCheck(hWnd)) 
		{
			Button_CameraReset(hwnd, uMsg, wParam, lParam);
			VPControl->Camera->SetPosition(0, 500, 0);		
			auto dir = VPControl->Camera->GetDirection();
			VPControl->Camera->SetDirection(0, -1, 0);
			VPControl->Camera->SetUp(dir.x, dir.y, dir.z);
			_checkboxState_ViewFromTop = true;
		}
		else
		{
			_checkboxState_ViewFromTop = false;
		}
	}
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
	_checkboxState_MeasureDistance = GetCheckboxState(LOWORD(wParam));
	if (Scene) 
	{
		
		if (!_checkboxState_MeasureDistance)
		{
			Scene->ClearMeasure();
			FillInfoGrid(Scene);
		}
	}
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
	_checkboxState_BlindZones = GetCheckboxState(LOWORD(wParam));
	return LRESULT();
}

LRESULT CUserInterface::Checkbox_MarkupLines(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	_checkboxState_MarkupLines = GetCheckboxState(LOWORD(wParam));
	return LRESULT();
}

LRESULT CUserInterface::Checkbox_MarkupLabels(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	_checkboxState_MarkupLabels = GetCheckboxState(LOWORD(wParam));
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
	Trackbar_BegAzm_SetText(ToolboxHWND, IDC_STATIC6);
	return LRESULT();
}

void CUserInterface::Trackbar_BegAzm_SetText(HWND hwnd, int labelID)
{
	float val = GetBegAzm();
	wstringstream stream;
	stream << fixed << setprecision(2) << val;
	wstring s = stream.str();
	SetDlgItemText(hwnd, labelID, s.c_str());
}

LRESULT CUserInterface::Trackbar_ZeroElevation(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CSettings::SetFloat(FloatZeroElevation, GetZeroElevation());
	CSettings::SetFloat(FloatCTrackRefresh_e0, GetZeroElevation());
	Trackbar_ZeroElevation_SetText(ToolboxHWND, IDC_STATIC7);
	return LRESULT();
}

void CUserInterface::Trackbar_ZeroElevation_SetText(HWND hwnd, int labelID)
{
	float val = GetZeroElevation();
	wstringstream stream;
	stream << fixed << setprecision(2) << val;
	wstring s = stream.str();
	SetDlgItemText(hwnd, labelID, s.c_str());
}

//TRACKBAR_CLASS
std::vector<tstring> CUserInterface::colorSettings;
std::vector<tstring> CUserInterface::distancesSettings;
std::vector<tstring> CUserInterface::crimagecolorSettings;
std::vector<tstring> CUserInterface::anglesSettings; 
CUserInterface::CUserInterface(HWND parentHWND, CViewPortControl *vpControl, CRCSocket *socket, int panelWidth)
{
	colorSettings.push_back(TEXT("ColorBackground"));
	colorSettings.push_back(TEXT("ColorMarkup"));
	colorSettings.push_back(TEXT("ColorMarkupInvisible"));
	colorSettings.push_back(TEXT("ColorNumbers"));
	colorSettings.push_back(TEXT("ColorPointLowLevel"));
	colorSettings.push_back(TEXT("ColorPointHighLevel"));
	colorSettings.push_back(TEXT("ColorPointSelected"));
	colorSettings.push_back(TEXT("ColorTrack"));
	colorSettings.push_back(TEXT("ColorTrackSelected"));
	colorSettings.push_back(TEXT("ColorMeasureLine"));
	colorSettings.push_back(TEXT("ColorMeasureLineInvisible"));
	colorSettings.push_back(TEXT("ColorAltitudeLowest"));
	colorSettings.push_back(TEXT("ColorAltitudeHighest"));
	colorSettings.push_back(TEXT("ColorBlindzoneLowest"));
	colorSettings.push_back(TEXT("ColorBlindzoneHighest"));	 		

	distancesSettings.push_back(TEXT("FloatMaxDistance"));
	distancesSettings.push_back(TEXT("FloatBlankR1"));
	distancesSettings.push_back(TEXT("FloatBlankR2"));
	distancesSettings.push_back(TEXT("FloatPositionRadarHeight"));
	distancesSettings.push_back(TEXT("FloatMarkDistance"));
	distancesSettings.push_back(TEXT("FloatMaxDist"));
	
	anglesSettings.push_back(TEXT("FloatZeroElevation"));
	anglesSettings.push_back(TEXT("FloatMinZeroElevation"));
	anglesSettings.push_back(TEXT("FloatMaxZeroElevation"));
	anglesSettings.push_back(TEXT("FloatMinBegAzm"));
	anglesSettings.push_back(TEXT("FloatMaxBegAzm"));

	
	crimagecolorSettings.push_back(TEXT("FloatAmp_00"));
	crimagecolorSettings.push_back(TEXT("FloatAmpPalettePosition_00"));
	crimagecolorSettings.push_back(TEXT("FloatAmp_01"));
	crimagecolorSettings.push_back(TEXT("FloatAmpPalettePosition_01"));
	crimagecolorSettings.push_back(TEXT("FloatAmp_02"));
	crimagecolorSettings.push_back(TEXT("FloatAmpPalettePosition_02"));




	string context = "CUserInterface::CUserInterface";
	LOG_INFO(requestID, context, (boost::format("Start... parentHWND=%1%, vpControl=%2%, socket=%3%, panelWidth=%4%...") % parentHWND % vpControl % socket % panelWidth).str().c_str());

	ParentHWND = parentHWND;
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

	CameraDirection_ID[0] = InsertElement(NULL, TRACKBAR_CLASS, _T("������ ������:"), WS_VISIBLE | WS_CHILD | TBS_VERT | TBS_RIGHT, MinimapSize, CurrentY, VStepGrp, MinimapSize, &CUserInterface::Trackbar_CameraDirection_VTilt);
	CurrentY += MinimapSize;
	CameraDirection_ID[1] = InsertElement(NULL, TRACKBAR_CLASS, _T("������� ������:"), WS_VISIBLE | WS_CHILD | TBS_HORZ | TBS_BOTTOM, 0, CurrentY, MinimapSize, VStepGrp, &CUserInterface::Trackbar_CameraDirection_Turn);

	
	int gridX = panelWidth;
	int gridY = vpControl->GetHeight();
	RECT clientRect;
	GetClientRect(parentHWND, &clientRect);


	Grid_ID = InsertElement(NULL, _T("ZeeGrid"), _T("������ ����������"), WS_BORDER | WS_TABSTOP | WS_VISIBLE | WS_CHILD, gridX, gridY, 2 * vpControl->GetWidth() / 3, clientRect.right - panelWidth, &CUserInterface::Grid);
	ColorGrid_ID = InsertElement(NULL, _T("ZeeGrid"), _T("�����"), WS_BORDER | WS_TABSTOP | WS_CHILD, gridX, gridY, 2 * vpControl->GetWidth() / 3, clientRect.right - panelWidth, &CUserInterface::ColorGrid);
	InfoGrid_ID = InsertElement(NULL, _T("ZeeGrid"), _T("����������"), WS_BORDER | WS_TABSTOP | WS_VISIBLE | WS_CHILD, gridX + 2 * vpControl->GetWidth() / 3 + 2, gridY, vpControl->GetWidth() / 3 - 2, clientRect.right - panelWidth, &CUserInterface::InfoGrid);

	
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
	Elements.insert({ IDC_CHECK_VIEW_FROM_TOP, new InterfaceElement{ IDC_CHECK_VIEW_FROM_TOP, NULL, _T("IDC_CHECK_VIEW_FROM_TOP"), _T("IDC_CHECK_VIEW_FROM_TOP"), TBS_BOTH | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0, nullptr, &CUserInterface::Checkbox_ViewFromTop } });
	Elements.insert({ IDC_CHECK2, new InterfaceElement{ IDC_CHECK2, NULL, _T("IDC_CHECK2"), _T("IDC_CHECK2"), TBS_BOTH | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0, nullptr, &CUserInterface::Checkbox_MeasureDistance } });
	
	Elements.insert({ IDC_CHECK3, new InterfaceElement{ IDC_CHECK3, NULL, _T("IDC_CHECK3"), _T("IDC_CHECK3"), TBS_BOTH | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0, nullptr, &CUserInterface::Checkbox_Points } });
	Elements.insert({ IDC_CHECK_RPOINT_FADEOUT, new InterfaceElement{ IDC_CHECK_RPOINT_FADEOUT, NULL, _T("IDC_CHECK_RPOINT_FADEOUT"), _T("IDC_CHECK_RPOINT_FADEOUT"), TBS_BOTH | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0, nullptr, &CUserInterface::Checkbox_PointsFadeout } });

	Elements.insert({ IDC_CHECK4, new InterfaceElement{ IDC_CHECK4, NULL, _T("IDC_CHECK4"), _T("IDC_CHECK4"), TBS_BOTH | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0, nullptr, &CUserInterface::Checkbox_Tracks } });
	Elements.insert({ IDC_CHECK_RDRTRACK_FADEOUT, new InterfaceElement{ IDC_CHECK_RDRTRACK_FADEOUT, NULL, _T("IDC_CHECK_RDRTRACK_FADEOUT"), _T("IDC_CHECK_RDRTRACK_FADEOUT"), TBS_BOTH | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0, nullptr, &CUserInterface::Checkbox_TracksFadeout } });

	Elements.insert({ IDC_CHECK5, new InterfaceElement{ IDC_CHECK5, NULL, _T("IDC_CHECK5"), _T("IDC_CHECK5"), TBS_BOTH | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0, nullptr, &CUserInterface::Checkbox_Images} });
	Elements.insert({ IDC_CHECK_RIMAGE_FADEOUT, new InterfaceElement{ IDC_CHECK_RIMAGE_FADEOUT, NULL, _T("IDC_CHECK_RIMAGE_FADEOUT"), _T("IDC_CHECK_RIMAGE_FADEOUT"), TBS_BOTH | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0, nullptr, &CUserInterface::Checkbox_ImagesFadeout } });

	Elements.insert({ IDC_CHECK6, new InterfaceElement{ IDC_CHECK6, NULL, _T("IDC_CHECK6"), _T("IDC_CHECK6"), TBS_BOTH | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0, nullptr, &CUserInterface::Checkbox_AltitudeMap } });
	Elements.insert({ IDC_CHECK7, new InterfaceElement{ IDC_CHECK7, NULL, _T("IDC_CHECK7"), _T("IDC_CHECK7"), TBS_BOTH | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0, nullptr, &CUserInterface::Checkbox_Map } });
	Elements.insert({ IDC_CHECK8, new InterfaceElement{ IDC_CHECK8, NULL, _T("IDC_CHECK8"), _T("IDC_CHECK8"), TBS_BOTH | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0, nullptr, &CUserInterface::Checkbox_BlindZones } });
	Elements.insert({ IDC_CHECK9, new InterfaceElement{ IDC_CHECK9, NULL, _T("IDC_CHECK9"), _T("IDC_CHECK9"), TBS_BOTH | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0, nullptr, &CUserInterface::Checkbox_MarkupLines } });
	Elements.insert({ IDC_CHECK10, new InterfaceElement{ IDC_CHECK10, NULL, _T("IDC_CHECK10"), _T("IDC_CHECK10"), TBS_BOTH | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0, nullptr, &CUserInterface::Checkbox_MarkupLabels } });

	Elements.insert({ IDC_BUTTON_CONNECT, new InterfaceElement{ IDC_BUTTON_CONNECT, NULL, _T("IDC_BUTTON_CONNECT"), _T("IDC_BUTTON_CONNECT"), TBS_BOTH | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0, nullptr, &CUserInterface::Button_Connect } });
	Elements.insert({ IDC_BUTTON_CAMERA_RESET, new InterfaceElement{ IDC_BUTTON_CAMERA_RESET, NULL, _T("IDC_BUTTON_CAMERA_RESET"), _T("IDC_BUTTON_CAMERA_RESET"), TBS_BOTH | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0, nullptr, &CUserInterface::Button_CameraReset } });
	/*BtnColors_ID = InsertElement(NULL, _T("BUTTON"), _T("�����"), WS_TABSTOP | WS_VISIBLE | WS_CHILD, Column3X, CurrentY, ControlWidth / 2, ButtonHeight, &CUserInterface::Button_Colors);
	BtnLoad_ID = InsertElement(NULL, _T("BUTTON"), _T("����. �����"), WS_TABSTOP | WS_VISIBLE | WS_CHILD, Column3X + ControlWidth / 2 + Column1X / 2, CurrentY, ControlWidth / 4 * 3, ButtonHeight, &CUserInterface::Button_Load);
	*/

	Elements.insert({ IDC_BUTTON2, new InterfaceElement{ IDC_BUTTON2, NULL, _T("IDC_BUTTON2"), _T("IDC_BUTTON2"), TBS_BOTH | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0, nullptr, &CUserInterface::Button_Settings } });
	
	Elements.insert({ IDC_SLIDER1, new InterfaceElement{ IDC_SLIDER1, NULL, _T("IDC_SLIDER1"), _T("IDC_SLIDER1"), TBS_BOTH | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0, nullptr, &CUserInterface::Trackbar_BegAzm } });
	Elements.insert({ IDC_SLIDER2, new InterfaceElement{ IDC_SLIDER2, NULL, _T("IDC_SLIDER2"), _T("IDC_SLIDER2"), TBS_BOTH | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0, nullptr, &CUserInterface::Trackbar_ZeroElevation } });
	
	//Elements.insert({ IDC_CHECK3, new InterfaceElement{ IDC_CHECK3, NULL, _T("IDC_CHECK3"), _T("IDC_CHECK3"), TBS_BOTH | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0, nullptr, &CUserInterface::Checkbox_ObjOptions } });

	Elements.insert({ IDC_EDIT_LON, new InterfaceElement{ IDC_EDIT_LON, NULL, _T("IDC_EDIT_LON"), _T("IDC_EDIT_LON"), TBS_BOTH | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0, nullptr, &CUserInterface::LonLat } });
	Elements.insert({ IDC_EDIT_LAT, new InterfaceElement{ IDC_EDIT_LAT, NULL, _T("IDC_EDIT_LAT"), _T("IDC_EDIT_LAT"), TBS_BOTH | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0, nullptr, &CUserInterface::LonLat } });

	
		
	Elements.insert({ IDC_BUTTON1, new InterfaceElement{ IDC_BUTTON1, NULL, _T("IDC_BUTTON1"), _T("IDC_BUTTON1"), TBS_BOTH | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0, nullptr, &CUserInterface::Button_Reload } });

	Elements.insert({ IDC_BUTTON5, new InterfaceElement{ IDC_BUTTON5, NULL, _T("IDC_BUTTON5"), _T("IDC_BUTTON5"), TBS_BOTH | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0, nullptr, &CUserInterface::Button_Test } });




	SendMessage(GetDlgItem(parentHWND, CameraDirection_ID[0]), TBM_SETPOS, 1, 50);
	SendMessage(GetDlgItem(parentHWND, CameraDirection_ID[1]), TBM_SETPOS, 1, 50);

	Font = GetFont();

	for (ElementsMap::iterator it = Elements.begin(); it != Elements.end(); ++it) {
		SendMessage(it->second->hWnd, WM_SETFONT, (WPARAM)Font, TRUE);
	}

	GridHWND = InfoGridHWND = nullptr;
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
	std::string context = "CUserInterface::ConnectionStateChanged";
	if (IsConnected) 
	{
		SetDlgItemText(ParentHWND, IsConnected_ID, _T("���� �������"));
		SetDlgItemText(ParentHWND, Button_Connect_ID, _T("����"));
	}
	else 
	{
		SetDlgItemText(ParentHWND, IsConnected_ID, _T("��� ��������"));
		SetDlgItemText(ParentHWND, Button_Connect_ID, _T("���������"));
	}
}

bool CUserInterface::GetCheckboxState_Map()
{
	return _checkboxState_Map;
}

bool CUserInterface::GetCheckboxState_BlindZones()
{
	return _checkboxState_BlindZones;
}

bool CUserInterface::GetCheckboxState_AltitudeMap()
{
	return _checkboxState_AltitudeMap;
}

bool CUserInterface::GetCheckboxState_MarkupLines()
{
	return _checkboxState_MarkupLines;
}

bool CUserInterface::GetCheckboxState_MarkupLabels()
{
	return _checkboxState_MarkupLabels; 
}

bool CUserInterface::GetCheckboxState_ViewFromTop()
{
	return _checkboxState_ViewFromTop;
}

bool CUserInterface::GetCheckboxState(int id)
{
	return Button_GetCheck(GetDlgItem(ToolboxHWND, id));
}

bool CUserInterface::GetCheckboxState_Points()
{
	return _checkboxState_Points;
}

bool CUserInterface::GetCheckboxState_PointsFadeout()
{
	return _checkboxState_PointsFadeout;
}

bool CUserInterface::GetCheckboxState_Tracks()
{
	return _checkboxState_Tracks;
}

bool CUserInterface::GetCheckboxState_TracksFadeout()
{
	return _checkboxState_TracksFadeout;
}

bool CUserInterface::GetCheckboxState_Images()
{
	return _checkboxState_Images;
}

bool CUserInterface::GetCheckboxState_ImagesFadeout()
{
	return _checkboxState_ImagesFadeout;
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

int CUserInterface::GetTrackbarValue(HWND hWnd, int id) const
{
	HWND hwnd = GetDlgItem(hWnd, id);
	int val = SendMessage(hwnd, TBM_GETPOS, 0, 0);
	return val;
}

void CUserInterface::SetTrackbarValue_VTilt(int val)
{
	SendMessage(GetDlgItem(ParentHWND, CameraDirection_ID[0]), TBM_SETPOS, 1, val);
}

void CUserInterface::SetTrackbarValue_Turn(int val)
{
	PostMessage(GetDlgItem(ParentHWND, CameraDirection_ID[1]), TBM_SETPOS, 1, val);
}

void CUserInterface::SetTrackbarValue(int id, int val) const
{
	SendMessage(GetDlgItem(ParentHWND, id), TBM_SETPOS, 1, val);
}

void CUserInterface::SetTrackbarValue_BegAzm(int val)
{
	SetTrackbarValue(IDC_SLIDER1, val);
	Trackbar_BegAzm_SetText(ToolboxHWND, IDC_STATIC6);
}

void CUserInterface::SetTrackbarValue_ZeroElevation(int val)
{
	SetTrackbarValue(ZeroElevation_ID, val);
	Trackbar_ZeroElevation_SetText(ToolboxHWND, IDC_STATIC7);
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
	SendMessage(GridHWND, ZGM_SETCELLTEXT, 2, (LPARAM)"���-�� �����");
	SendMessage(GridHWND, ZGM_SETCELLTEXT, 3, (LPARAM)"��������� �����");
	SendMessage(GridHWND, ZGM_SETCELLTEXT, 4, (LPARAM)"�������� �����");
	SendMessage(GridHWND, ZGM_SETCELLTEXT, 5, (LPARAM)"��������");
	SendMessage(GridHWND, ZGM_SETCELLTEXT, 6, (LPARAM)"������");
	SendMessage(GridHWND, ZGM_SETCELLTEXT, 7, (LPARAM)"�����");


	//make column 3 editable by the user
	//SendMessage(GridHWND, ZGM_SETCOLEDIT, 3, 1);

	//auto size all columns
	SendMessage(GridHWND, ZGM_AUTOSIZE_ALL_COLUMNS, 0, 0);
	SendMessage(InfoGridHWND, ZGM_SETFONT, 2, (LPARAM)Font);
	SendMessage(InfoGridHWND, ZGM_DIMGRID, 2, 0);
	SendMessage(InfoGridHWND, ZGM_SHOWROWNUMBERS, FALSE, 0);

	SendMessage(InfoGridHWND, ZGM_SETCELLTEXT, 1, (LPARAM)"��������");
	SendMessage(InfoGridHWND, ZGM_SETCELLTEXT, 2, (LPARAM)"��������");

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
	if (!scene && !Scene)
	{
		LOG_WARN__("Called with parameter scene=nullptr");
		return;
	}
	if (!scene)
	{
		scene = Scene;
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
	//SendMessage(InfoGridHWND, ZGM_SETCELLTEXT, ncols * r + 1, formatmsg("�������������� ������"));

	//*(&(CRCGridCell::CRCGridCell(InfoGridHWND, ncols, r, 1))) = "test";

	GRIDCELL(InfoGridHWND, ncols * r + 1) = "Radar position";
	GRIDCELL(InfoGridHWND, ncols * r + 2) = format("%.4f, %.4f", scene->position.x, scene->position.y);
	
	r++;
	GRIDCELL(InfoGridHWND, ncols * r + 1) = "Radar altitude";
	if (scene->MeshReady()) {
		GRIDCELL(InfoGridHWND, ncols * r + 2) = format("%.1f", scene->Mesh->GetCenterHeight());
	}



	r++;	
	GRIDCELL(InfoGridHWND, ncols * r + 1) = "Radar height";
	GRIDCELL(InfoGridHWND, ncols * r + 2) = format("%.1f", CSettings::GetFloat(FloatPositionRadarHeight));

	if (scene->Camera) {
		auto p = scene->Camera->GetPosition();
		r++;
		GRIDCELL(InfoGridHWND, ncols * r + 1) = "Camera position";
		GRIDCELL(InfoGridHWND, ncols * r + 2) = format("%.4f, %.4f, %.4f", p.x, p.y, p.z);
	}

	if (scene->Socket->IsConnected) {
		r++;
		GRIDCELL(InfoGridHWND, ncols * r + 1) = "Elevation min";
		GRIDCELL(InfoGridHWND, ncols * r + 2) = format("%.4f", scene->minE);
		r++;
		GRIDCELL(InfoGridHWND, ncols * r + 1) = "Elevation max";
		GRIDCELL(InfoGridHWND, ncols * r + 2) = format("%.4f", scene->maxE);
	}

	if (scene->Selection.size()>0)
	{
		C3DObjectModel *last = scene->Selection.at(scene->Selection.size() - 1);

		glm::vec3 geoCoords = last->GetGeoCoords();

		r++;
		SendMessage(InfoGridHWND, ZGM_SETCELLTEXT, ncols * r + 1, (LPARAM)"����� ");
		ss.str(std::string());
		ss << std::fixed << std::setprecision(4) << geoCoords.x << ", " << geoCoords.y;
		SendMessage(InfoGridHWND, ZGM_SETCELLTEXT, ncols * r + 2, (LPARAM)ss.str().c_str());
	}

	if (MeasureDistance() && Scene)
	{
		r++;
		SendMessage(InfoGridHWND, ZGM_SETCELLTEXT, ncols * r + 1, (LPARAM)"Distance (m)");
		ss.str(std::string());
		ss << std::fixed << std::setprecision(2) << Scene->GetMeasureLength();
		SendMessage(InfoGridHWND, ZGM_SETCELLTEXT, ncols * r + 2, (LPARAM)ss.str().c_str());
	}

	SendMessage(InfoGridHWND, ZGM_AUTOSIZE_ALL_COLUMNS, 0, 0);
}

bool CUserInterface::MeasureDistance() const
{
	return _checkboxState_MeasureDistance;
}

int CUserInterface::GetMainTableMode() const
{
	return mainTableMode;
}

void CUserInterface::SetMainTableMode(int value)
{
	mainTableMode = value;
}

void CUserInterface::OnInitDialog()
{
	LOG_INFO_("OnInitDialog", "ToolboxHWND=%d", ToolboxHWND);
	Trackbar_BegAzm_SetText(ToolboxHWND, IDC_STATIC6);
	Trackbar_ZeroElevation_SetText(ToolboxHWND, IDC_STATIC7);

	SetChecked(ToolboxHWND, IDC_CHECK2, _checkboxState_MeasureDistance);
	SetChecked(ToolboxHWND, IDC_CHECK3, _checkboxState_Points);	
	SetChecked(ToolboxHWND, IDC_CHECK4, _checkboxState_Tracks);
	SetChecked(ToolboxHWND, IDC_CHECK5, _checkboxState_Images);
	SetChecked(ToolboxHWND, IDC_CHECK6, _checkboxState_AltitudeMap);
	SetChecked(ToolboxHWND, IDC_CHECK7, _checkboxState_Map);
	SetChecked(ToolboxHWND, IDC_CHECK8, _checkboxState_BlindZones);
	SetChecked(ToolboxHWND, IDC_CHECK9, _checkboxState_MarkupLines);
	SetChecked(ToolboxHWND, IDC_CHECK10, _checkboxState_MarkupLabels);

	SetDoubleValue(GetDlgItem(ToolboxHWND, IDC_EDIT_LON), CSettings::GetFloat(FloatPositionLon));
	SetDoubleValue(GetDlgItem(ToolboxHWND, IDC_EDIT_LAT), CSettings::GetFloat(FloatPositionLat));
}
