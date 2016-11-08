#pragma once
#include "stdafx.h"

#define CAT( A, B ) A ## B
#define SELECT( NAME, NUM ) CAT( NAME ## _, NUM )

#define EXPAND(x) x

#define GET_COUNT( _1, _2, _3, _4, _5, _6, COUNT, ... ) COUNT
#define VA_SIZE( ... ) EXPAND(GET_COUNT( __VA_ARGS__ , 6, 5, 4, 3, 2, 1, 0 ))

#define VA_SELECT( NAME, ... ) SELECT( NAME, VA_SIZE(__VA_ARGS__) )(__VA_ARGS__)

#define GRIDCELL( ... ) VA_SELECT( MY_IMPL, __VA_ARGS__ )
#define MY_IMPL_1(...) not_implemented_1
#define MY_IMPL_2(...) *(&(CRCGridCell::CRCGridCell(__VA_ARGS__))) 
#define MY_IMPL_3(...) not_implemented_3
#define MY_IMPL_4(...) *(&(CRCGridCell::CRCGridCell(__VA_ARGS__))) 

//#define GRIDCELL(...) *(&(CRCGridCell::CRCGridCell(__VA_ARGS__))) 

class CRCGridCell
{	
	HWND hWnd;
	int index;
public:
	CRCGridCell(HWND hWnd, int index);
	CRCGridCell(HWND hWnd, int ncols, int r, int c);	
	CRCGridCell& operator=(const std::string value);
};
