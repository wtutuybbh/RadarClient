#include "stdafx.h"

#include "CRCListView.h"
#include <CommCtrl.h>

/*
LRESULT CALLBACK MainWndProc(HWND hWnd,
	UINT uMessage,
	WPARAM wParam,
	LPARAM lParam)
{
	static HWND hwndListView;

	switch (uMessage)
	{
	case WM_CREATE:
		// create the TreeView control
		hwndListView = CreateListView(g_hInst, hWnd);

		//initialize the TreeView control
		InitListView(hwndListView);

		break;

	case WM_NOTIFY:
		return ListViewNotify(hWnd, lParam);

	case WM_SIZE:
		ResizeListView(hwndListView, hWnd);
		break;

	case WM_INITMENUPOPUP:
		UpdateMenu(hwndListView, GetMenu(hWnd));
		break;

	case WM_CONTEXTMENU:
		if (DoContextMenu(hWnd, wParam, lParam))
			return FALSE;
		break;

	case WM_COMMAND:
		switch (GET_WM_COMMAND_ID(wParam, lParam))
		{
		case IDM_LARGE_ICONS:
			SwitchView(hwndListView, LVS_ICON);
			break;

		case IDM_SMALL_ICONS:
			SwitchView(hwndListView, LVS_SMALLICON);
			break;

		case IDM_LIST:
			SwitchView(hwndListView, LVS_LIST);
			break;

		case IDM_REPORT:
			SwitchView(hwndListView, LVS_REPORT);
			break;

		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;

		case IDM_ABOUT:
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutDlgProc);
			break;

		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		break;
	}
	return DefWindowProc(hWnd, uMessage, wParam, lParam);
}
*/

//HWND CRCListView::CreateListView(HINSTANCE, HWND)
//{
//}

void CRCListView::ResizeListView(HWND, HWND)
{
}

BOOL CRCListView::InitListView(HWND hwndListView, int ITEM_COUNT)
{
	LV_COLUMN   lvColumn;
	int         i;
	TCHAR       szString[5][20] = { TEXT("Main Column"), TEXT("Column 1"), TEXT("Column 2"), TEXT("Column 3"), TEXT("Column 4") };

	//empty the list
	ListView_DeleteAllItems(hwndListView);

	//initialize the columns
	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 120;
	for (i = 0; i < 5; i++)
	{
		lvColumn.pszText = szString[i];
		ListView_InsertColumn(hwndListView, i, &lvColumn);
	}

	InsertListViewItems(hwndListView, ITEM_COUNT);

	return TRUE;
}

BOOL CRCListView::InsertListViewItems(HWND hwndListView, int ITEM_COUNT)
{
	//empty the list
	ListView_DeleteAllItems(hwndListView);

	//set the number of items in the list
	ListView_SetItemCount(hwndListView, ITEM_COUNT);

	return TRUE;
}

LRESULT CRCListView::ListViewNotify(HWND hWnd, LPARAM lParam, int ID_LISTVIEW)
{
	LPNMHDR  lpnmh = (LPNMHDR)lParam;
	HWND     hwndListView = GetDlgItem(hWnd, ID_LISTVIEW);

	switch (lpnmh->code)
	{
	case LVN_GETDISPINFO:
	{
		LV_DISPINFO *lpdi = (LV_DISPINFO *)lParam;
		TCHAR szString[MAX_PATH];

		if (lpdi->item.iSubItem)
		{
			if (lpdi->item.mask & LVIF_TEXT)
			{
				_sntprintf_s(szString, _countof(szString), _TRUNCATE,
					TEXT("Элемент %d - Column %d"),
					lpdi->item.iItem + 1, lpdi->item.iSubItem);
				_tcsncpy_s(lpdi->item.pszText, lpdi->item.cchTextMax,
					szString, _TRUNCATE);
			}
		}
		else
		{
			if (lpdi->item.mask & LVIF_TEXT)
			{
				_sntprintf_s(szString, _countof(szString), _TRUNCATE,
					TEXT("Item %d"), lpdi->item.iItem + 1);
				_tcsncpy_s(lpdi->item.pszText, lpdi->item.cchTextMax,
					szString, _TRUNCATE);
			}

			if (lpdi->item.mask & LVIF_IMAGE)
			{
				lpdi->item.iImage = 0;
			}
		}
	}
	return 0;

	case LVN_ODCACHEHINT:
	{
		LPNMLVCACHEHINT   lpCacheHint = (LPNMLVCACHEHINT)lParam;
		/*
		This sample doesn't use this notification, but this is sent when the
		ListView is about to ask for a range of items. On this notification,
		you should load the specified items into your local cache. It is still
		possible to get an LVN_GETDISPINFO for an item that has not been cached,
		therefore, your application must take into account the chance of this
		occurring.
		*/
	}
	return 0;

	case LVN_ODFINDITEM:
	{
		LPNMLVFINDITEM lpFindItem = (LPNMLVFINDITEM)lParam;
		/*
		This sample doesn't use this notification, but this is sent when the
		ListView needs a particular item. Return -1 if the item is not found.
		*/
	}
	return 0;
	}

	return 0;
}
void CRCListView::SwitchView(HWND hwndListView, DWORD dwView)
{
	DWORD dwStyle = GetWindowLong(hwndListView, GWL_STYLE);

	SetWindowLong(hwndListView, GWL_STYLE, (dwStyle & ~LVS_TYPEMASK) | dwView);
//	ResizeListView(hwndListView, GetParent(hwndListView));
}