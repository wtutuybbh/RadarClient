#include "stdafx.h"

#include "CRCListView.h"
#include <CommCtrl.h>

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