#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h>
//#include <CommCtrl.h>
#include <unordered_map>
#include <vector>
#include <tchar.h>

#include "Util.h"

#include "glm/glm.hpp"

#define TEXT_LABEL_NOT_CONNECTED _T("Нет соединения")
#define TEXT_LABEL_CONNECTED _T("Соединение установлено")
#define TEXT_LABEL_SHOW _T("Выводить:")
#define TEXT_LABEL_VIEW _T("Вид местности:")
#define TEXT_LABEL_MARKUP _T("Разметка:")
#define TEXT_LABEL_CAMERA_POSITION _T("Положение камеры:")
#define TEXT_LABEL_CAMERA_DIRECTION _T("Направление камеры:")

#define TEXT_BUTTON_CONNECT _T("Cоединить")
#define TEXT_BUTTON_DISCONNECT _T("Разъединить")
#define TEXT_BUTTON_TEST _T("Тест")
#define TEXT_BUTTON_DUMP _T("Дамп")

#define TEXT_RADIOBUTTON_CAMERA_POSITION_FROM_RADAR _T("От радара")
#define TEXT_RADIOBUTTON_CAMERA_POSITION_FROM_100M_ABOVE_RADAR _T("100м над радаром")
#define TEXT_RADIOBUTTON_CAMERA_POSITION_FROM_1000M_ABOVE_RADAR _T("1км над радаром")

#define TEXT_RADIOBUTTON_CAMERA_DIRECTION_FROM_RADAR_TO_NORTH _T("От радара на север")
#define TEXT_RADIOBUTTON_CAMERA_DIRECTION_TO_RADAR _T("К радару")
#define TEXT_RADIOBUTTON_CAMERA_DIRECTION_TO_SOUTH _T("На юг")


#define TEXT_CHECKBOX_POINTS _T("Точки")
#define TEXT_CHECKBOX_SERIES _T("Траектории")
#define TEXT_CHECKBOX_RLI _T("RLI-изображения")
#define TEXT_CHECKBOX_FIXVIEWTORADAR _T("Вид на радар")
#define TEXT_CHECKBOX_MEASURE_DISTANCE _T("Измерения")

#define TEXT_CHECKBOX_LANDSCAPE _T("Ландшафт")
#define TEXT_CHECKBOX_MAP _T("Карта")

#define TEXT_CHECKBOX_MARKUP_LINES _T("Линии")
#define TEXT_CHECKBOX_MARKUP_LABELS _T("Числа")

#define TEXT_GRID_NAME _T("Список траекторий")
#define TEXT_INFOGRID_NAME _T("Информация")

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
	LPCSTR Class;
	LPCSTR Text;
	DWORD Style;
	int X, Y, Width, Height;
	HWND hWnd;	
	UIWndProc Action;
} InterfaceElement;

typedef std::unordered_map<int, InterfaceElement *> ElementsMap;

class CUserInterface
{
private:
	int CurrentID;
	int IsConnected_ID, Button_Connect_ID, Grid_ID, InfoGrid_ID, FixViewToRadar_ID, MeasureDistance_ID, Test_ID, Dump_ID, BegAzm_ID, ZeroElevation_ID, BegAzmValue_ID, ZeroElevationValue_ID;
	int ObjOptions_ID[3], MapOptions_ID[2], MarkupOptions_ID[2], CameraPosition_ID[3], CameraDirection_ID[2], CameraDirectionValue_ID[2];
	//int Info_ID[]
	void SetChecked(int id, bool checked);

	glm::vec3 GetDirection();
	float GetBegAzm();
	float GetZeroElevation();

	float GetHeight();

	HMODULE hgridmod; //ZeeGrid's dll handle

	HFONT Font; // user interface font

	HWND GridHWND, InfoGridHWND; //ZeeGrid's window handle
	int Column1X, Column2X, Column3X;
	int VStep, VStepGrp;
	int PanelWidth;
	int ButtonHeight, ControlWidth, ControlWidthL, ControlWidthXL;
	int GridX, GridY, InfoGridX, InfoGridY;
	int GridWidth, InfoGridWidth;

	virtual LRESULT Button_Connect(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Button_Test(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Button_Dump(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Checkbox_ObjOptions(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Checkbox_MapOptions(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Checkbox_MarkupOptions(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Checkbox_FixViewToRadar(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Checkbox_MeasureDistance(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT RadioGroup_CameraPosition(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Trackbar_CameraDirection_VTilt(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Trackbar_CameraDirection_Turn(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Trackbar_BegAzm(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Trackbar_BegAzm_SetText(int labelID);
	virtual LRESULT Trackbar_ZeroElevation(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Trackbar_ZeroElevation_SetText(int labelID);
	virtual LRESULT Grid(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT InfoGrid(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	int MinimapSize;
	HWND ParentHWND;
	HFONT hDlgFont;
	

	ElementsMap Elements;

	CViewPortControl *VPControl;
	CRCSocket *Socket;

	int InsertElement(DWORD xStyle, LPCSTR Class,LPCSTR Text, DWORD Style, int X, int Y, int Width, int Height, UIWndProc Action);
	
	virtual LRESULT Wnd_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Wnd_Proc2(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


	CUserInterface(HWND parentHWND, CViewPortControl *vpControl,	CRCSocket *Socket, int panelWidth);
	virtual ~CUserInterface();	

	void ConnectionStateChanged(bool IsConnected) const;

	bool GetCheckboxState_Points();
	bool GetCheckboxState_Tracks();
	bool GetCheckboxState_Images();
	bool GetCheckboxState_Map();
	bool GetCheckboxState_AltitudeMap();
	bool GetCheckboxState_MarkupLines();
	bool GetCheckboxState_MarkupLabels();

	int GetTrackbarValue_VTilt();
	int GetTrackbarValue_Turn();
	int GetTrackbarValue(int id) const;
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
#ifdef _DEBUG
	DebugWindowInfo *dwi;
#endif
};

