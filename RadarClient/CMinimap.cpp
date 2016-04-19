#include "CMinimap.h"


void CMinimap::Draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}

/*CMinimap::CMinimap()
{
}*/


CMinimap::~CMinimap()
{
}

LRESULT CMinimap::ViewPortControlProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {

	case WM_PAINT:
		Paint();
		return 0;
	case WM_SIZE:
		switch (wParam)												// Evaluate Size Action
		{
		case SIZE_MINIMIZED:									// Was Window Minimized?
																//window->isVisible = FALSE;							// Set isVisible To False
			return 0;												// Return

		case SIZE_MAXIMIZED:									// Was Window Maximized?
																//window->isVisible = TRUE;							// Set isVisible To True
			ReshapeGL(LOWORD(lParam), HIWORD(lParam));		// Reshape Window - LoWord=Width, HiWord=Height
			return 0;												// Return

		case SIZE_RESTORED:										// Was Window Restored?
																//window->isVisible = TRUE;							// Set isVisible To True
			ReshapeGL(LOWORD(lParam), HIWORD(lParam));		// Reshape Window - LoWord=Width, HiWord=Height
			return 0;												// Return
		}
		return 0;


	case WM_MOUSEMOVE:
		
		if (wParam == MK_LBUTTON) {
				
		}
		break;
	case WM_LBUTTONDOWN:
		SetFocus(hwnd);
		
		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CMinimap::ReshapeGL(int width, int height)
{
	if (hDC && hRC) {
		MakeCurrent();
		glViewport(0, 0, (GLsizei)(width), (GLsizei)(height));
	}
}
