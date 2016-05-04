#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h>
//#include <CommCtrl.h>
#include <unordered_map>
#include <vector>
#include <tchar.h>

#include "glm/glm.hpp"

#define TEXT_LABEL_NOT_CONNECTED _T("��� ����������")
#define TEXT_LABEL_CONNECTED _T("���������� �����������")
#define TEXT_LABEL_SHOW _T("��������:")
#define TEXT_LABEL_VIEW _T("��� ���������:")
#define TEXT_LABEL_CAMERA_POSITION _T("��������� ������:")
#define TEXT_LABEL_CAMERA_DIRECTION _T("����������� ������:")

#define TEXT_BUTTON_CONNECT _T("���������� ����������")
#define TEXT_BUTTON_DISCONNECT _T("��������� ����������")

#define TEXT_RADIOBUTTON_CAMERA_POSITION_FROM_RADAR _T("�� ������")
#define TEXT_RADIOBUTTON_CAMERA_POSITION_FROM_100M_ABOVE_RADAR _T("� ������ 100 � ��� �������")
#define TEXT_RADIOBUTTON_CAMERA_POSITION_FROM_1000M_ABOVE_RADAR _T("� ������ 1 �� � ��� �������")

#define TEXT_RADIOBUTTON_CAMERA_DIRECTION_FROM_RADAR_TO_NORTH _T("�� ������ �� �����")
#define TEXT_RADIOBUTTON_CAMERA_DIRECTION_TO_RADAR _T("� ������")
#define TEXT_RADIOBUTTON_CAMERA_DIRECTION_TO_SOUTH _T("�� ��")


#define TEXT_CHECKBOX_POINTS _T("�����")
#define TEXT_CHECKBOX_SERIES _T("����������")
#define TEXT_CHECKBOX_RLI _T("RLI-�����������")
#define TEXT_CHECKBOX_FIXVIEWTORADAR _T("����������� ������ �� �����")

#define TEXT_CHECKBOX_LANDSCAPE _T("��������")
#define TEXT_CHECKBOX_MAP _T("�����")

#define TEXT_GRID_NAME _T("������ ����������")

class ViewPortControl;
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
	int IsConnected_ID, Button_Connect_ID, Grid_ID, FixViewToRadar_ID;
	int ObjOptions_ID[3], MapOptions_ID[2], CameraPosition_ID[3], CameraDirection_ID[2], CameraDirectionValue_ID[2];
	//int Info_ID[]
	void SetChecked(int id, bool checked);

	glm::vec3 GetDirection();

	HMODULE hgridmod; //ZeeGrid's dll handle

	HFONT Font; // user interface font

	HWND hg; //ZeeGrid's window handle
public:
	int Column1X, Column2X;
	int VStep, VStepGrp;
	int MinimapSize, PanelWidth;
	int ButtonHeight, ControlWidth, ControlWidthL, ControlWidthXL;
	HWND ParentHWND;
	HFONT hDlgFont;
	

	ElementsMap Elements;

	ViewPortControl *VPControl;
	CRCSocket *Socket;

	int InsertElement(DWORD xStyle, LPCSTR Class,LPCSTR Text, DWORD Style, int X, int Y, int Width, int Height, UIWndProc Action);
	

	virtual LRESULT Button_Connect(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual LRESULT Checkbox_ObjOptions(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Checkbox_MapOptions(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Checkbox_FixViewToRadar(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT RadioGroup_CameraPosition(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Trackbar_CameraDirection_VTilt(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Trackbar_CameraDirection_Turn(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual LRESULT Wnd_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT Wnd_Proc2(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual LRESULT Button_Test(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual LRESULT Grid(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	CUserInterface(HWND parentHWND, ViewPortControl *vpControl,	CRCSocket *Socket, int panelWidth);
	~CUserInterface();	

	void ConnectionStateChanged(bool IsConnected);

	bool GetCheckboxState_Points();

	int GetTrackbarValue_VTilt();
	int GetTrackbarValue_Turn();
	void SetTrackbarValue_VTilt(int val);
	void SetTrackbarValue_Turn(int val);

	void ControlEnable(int ID, bool enable);

	bool IsVistaOrLater();

	void Resize();

	void InitGrid();

	HFONT GetFont();

	void FillGrid(std::vector<TRK*> *tracks);
};

