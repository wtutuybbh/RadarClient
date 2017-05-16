#pragma once

#include "stdafx.h"

#include "Util.h"
#include <CommCtrl.h>
#include "XColorSpectrumCtrl/XColorSpectrumCtrl.h"

class CScene;
class CViewPortControl;
class CRCSocket;

class CUserInterface;
typedef LRESULT (CUserInterface::*UIWndProc)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

struct _BGCELL;
class TRK;



typedef struct {	
	int ID;
	DWORD XStyle;
	LPCWSTR Class;
	LPCWSTR Text;
	DWORD Style;
	int X, Y, Width, Height;
	HWND hWnd;	
	UIWndProc Action;
} InterfaceElement;

typedef std::unordered_map<int, InterfaceElement *> ElementsMap;

class CUserInterface
{
private:
	static const std::string requestID;
	int mainTableMode {0};
	int CurrentID {1}; // incremental ID counter

	//single elements
	int IsConnected_ID;
	int Button_Connect_ID;
	int Grid_ID;
	int InfoGrid_ID;
	int ColorGrid_ID;
	int FixViewToRadar_ID;
	int MeasureDistance_ID;
	int BtnTest_ID;
	int BtnColors_ID;
	int BtnLoad_ID;
	int BegAzm_ID;
	int ZeroElevation_ID;
	int BegAzmValue_ID;
	int ZeroElevationValue_ID;

	int IDD_DIALOG1_ID;

	//group elements
	int ObjOptions_ID[3], MapOptions_ID[2], MarkupOptions_ID[2], CameraPosition_ID[3], CameraDirection_ID[2], CameraDirectionValue_ID[2], MapType_ID[2];
	static HINSTANCE hInstance;
	void SetChecked(HWND hwnd, int id, bool checked);

	glm::vec3 GetDirection();
	float GetBegAzm();
	float GetZeroElevation();

	float GetHeight();

	HMODULE hgridmod; //ZeeGrid's dll handle

	static HFONT Font; // user interface font

	HWND GridHWND, InfoGridHWND; //ZeeGrid's window handle
	int Column1X{ 10 }, Column2X, Column3X;
	int VStep{ 21 }, VStepGrp{ 30 };
	int PanelWidth;
	int ButtonHeight{ 19 }, ControlWidth, ControlWidthL, ControlWidthXL;
	int GridX, GridY, InfoGridX, InfoGridY;
	int GridWidth, InfoGridWidth;

	virtual LRESULT Button_Connect(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Button_Colors(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Button_Load(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Button_Test(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual LRESULT Button_Settings(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);	
	static LRESULT CALLBACK Dialog_Settings(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static std::string GetStringFromResourceID(int ID);
	static std::wstring GetWStringFromResourceID(int ID);
	static tstring GetColorForSettingsDialog(int index);
	static tstring GetColorListViewCellText(int iItem, int iSubItem);
	static void SetColorListViewItemColor(LPARAM lParam, int iItem, int iSubItem);
	static void InitColorListViewColumns(HWND hwndListView, LV_COLUMN lvColumn);
	static LRESULT CALLBACK ProcessColorListViewCustomDraw(LPARAM lParam);

	static tstring GetDistanceForSettingsDialog(int index);
	static tstring GetDistanceListViewCellText(int iItem, int iSubItem);
	static void InitDistanceListViewColumns(HWND hwndListView, LV_COLUMN lvColumn);

	virtual LRESULT Button_Reload(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


	static LRESULT CALLBACK Dialog_SelectColor(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static CXColorSpectrumCtrl m_ColorSpectrum;

	static int iItem_ColorListView;
	static int iItem_DistancesListView;
	static int rclick;
	static glm::vec4 oldColor_ColorListView;
	static POINT rclick_point;
	static POINT prev_point;
	static int dy;
	static RECT settings_dialog_rect_;
	static float rclick_value;



	virtual LRESULT Checkbox_ObjOptions(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Checkbox_MapOptions(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Checkbox_MarkupOptions(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Checkbox_FixViewToRadar(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Checkbox_MeasureDistance(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT RadioGroup_CameraPosition(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Checkbox_MapType(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Checkbox_BlindZones(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Trackbar_CameraDirection_VTilt(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Trackbar_CameraDirection_Turn(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Trackbar_BegAzm(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Trackbar_BegAzm_SetText(HWND hwnd, int labelID);
	virtual LRESULT Trackbar_ZeroElevation(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Trackbar_ZeroElevation_SetText(HWND hwnd, int labelID);
	virtual LRESULT Grid(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT ColorGrid(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT InfoGrid(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT LonLat(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


	virtual LRESULT IDD_DIALOG1_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static std::vector<tstring> colorSettings;
	static std::vector<tstring> distancesSettings;
	static std::vector<tstring> anglesSettings;
public:
	int MinimapSize;
	static HWND ParentHWND;
	static HWND SettingsHWND;	
	static HWND ToolboxHWND;
	static CScene *Scene;
	HWND GetSettingsHWND() const;
	HFONT hDlgFont {nullptr};
	
	ElementsMap Elements;

	CViewPortControl *VPControl { nullptr };
	CRCSocket *Socket { nullptr };

	double GetDoubleValue(HWND hw);
	void SetDoubleValue(HWND hw, double val);

	int InsertElement(DWORD xStyle, LPCWSTR Class, LPCWSTR Text, DWORD Style, int X, int Y, int Width, int Height, UIWndProc Action);
	void InsertHandlerElement(int ID, UIWndProc Action);
	
	virtual LRESULT Wnd_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Wnd_Proc2(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


	CUserInterface(HWND parentHWND, CViewPortControl *vpControl,	CRCSocket *Socket, int panelWidth);
	virtual ~CUserInterface();	

	void ConnectionStateChanged(bool IsConnected) const;

	static bool GetCheckboxState_Points();
	static bool GetCheckboxState_Tracks();
	static bool GetCheckboxState_Images();
	static bool GetCheckboxState_Map();
	static bool GetCheckboxState_BlindZones();
	static bool GetCheckboxState_AltitudeMap();
	static bool GetCheckboxState_MarkupLines();
	static bool GetCheckboxState_MarkupLabels();

	int GetTrackbarValue_VTilt();
	int GetTrackbarValue_Turn();
	int GetTrackbarValue(HWND hWnd, int id) const;
	void SetTrackbarValue_VTilt(int val);
	void SetTrackbarValue_Turn(int val);
	void SetTrackbarValue(int id, int val) const;
	void SetTrackbarValue_BegAzm(int val);
	void SetTrackbarValue_ZeroElevation(int val);

	void ControlEnable(int ID, bool enable) const;

	static bool IsVistaOrLater();

	void Resize() const;

	void InitGrid();

	static HFONT GetFont();

	void FillGrid(std::vector<TRK*> *tracks);
	void FillInfoGrid(CScene *scene);

	bool MeasureDistance() const;

	int GetMainTableMode() const;
	void SetMainTableMode(int value);

	void OnInitDialog();
#ifdef _DEBUG
	DebugWindowInfo *dwi;
#endif
};

/*
GRID(InfoGrid, r, c) = formatmsg("Местоположение радара");

must be translated into:


SendMessage(InfoGridHWND, ZGM_SETCELLTEXT, ncols * r + c, formatmsg("Местоположение радара"));
*/

