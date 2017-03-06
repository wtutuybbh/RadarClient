#pragma once
#include "stdafx.h"
#include <CommCtrl.h>



typedef tstring(*CellTextCallback)(int iItem, int iSubItem);
typedef void(InitListViewColumnsCallback)(HWND hwndListView, LV_COLUMN lvColumn);

class CRCListView
{
public:
	//static HWND CreateListView(HINSTANCE, HWND);
	static void ResizeListView(HWND hwndListView, HWND hwndParent);
	static BOOL InitListView(HWND hwndListView, int ITEM_COUNT, InitListViewColumnsCallback initListViewColumns);
	static BOOL InsertListViewItems(HWND hwndListView, int ITEM_COUNT);
	static LRESULT ListViewNotify(HWND hWnd, LPARAM lParam, int ID_LISTVIEW, CellTextCallback cellText);
	static void SwitchView(HWND hwndListView, DWORD dwView);
};
