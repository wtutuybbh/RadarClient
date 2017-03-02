#pragma once

class CRCListView
{
public:
	//static HWND CreateListView(HINSTANCE, HWND);
	static void ResizeListView(HWND hwndListView, HWND hwndParent);
	static BOOL InitListView(HWND hwndListView, int ITEM_COUNT);
	static BOOL InsertListViewItems(HWND hwndListView, int ITEM_COUNT);
	static LRESULT ListViewNotify(HWND hWnd, LPARAM lParam, int ID_LISTVIEW);
	static void SwitchView(HWND hwndListView, DWORD dwView);
};
