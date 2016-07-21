#include "CMinimap.h"
#include "CScene.h"
#include "CCamera.h"

bool CMinimap::IsCameraHere(int x, int y) const
{
	if (!Camera || !Scene)
		return false;
	glm::vec4 viewport = glm::vec4(0, 0, Width, Height);
	glm::vec3 wincoord0 = glm::vec3(x, Height - y - 1, 0.0f);
	glm::vec3 p0 = glm::unProject(wincoord0, Camera->GetMiniMapView(), Camera->GetMiniMapProjection(), viewport);
	glm::vec3 wincoord1 = glm::vec3(x, Height - y - 1, 1.0f);
	glm::vec3 p1 = glm::unProject(wincoord1, Camera->GetMiniMapView(), Camera->GetMiniMapProjection(), viewport);

	if (Scene->GetObjectAtMiniMapPosition(Id, p0, p1)) {
		return true;
	}
	return false;
}

void CMinimap::Draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (Scene) {
		Scene->MiniMapDraw(this);
	}
}


CMinimap::~CMinimap()
{
}

LRESULT CMinimap::ViewPortControlProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {

	case WM_PAINT: {
		Paint();
		return 0;
	}
	case WM_SIZE: {
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
		 }

	case WM_MOUSEMOVE:
	{
		if (wParam == MK_LBUTTON) {

		}
		if (IsCameraHere((int)LOWORD(lParam), (int)HIWORD(lParam))) {
			SetCursor(LoadCursor(NULL, IDC_HAND));
		}
		else {
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		}
		if (CameraDrag) {

			
			if (Scene && UI && Camera) {
				Scene->SetCameraPositionFromMiniMapXY(2.0f*(float)LOWORD(lParam) / (float)Width - 1, 1 - 2.0f * (float)HIWORD(lParam) / (float)Height, 0);

				float r = glm::length(Camera->GetDirection());
				float e = acos(Camera->GetDirection().y / r);

				float a = Camera->GetAzimut();

				UI->SetTrackbarValue_Turn(50 * a / M_PI + 50);
				UI->SetTrackbarValue_VTilt(100 * e / M_PI);
			}
		}

		break;
		}
	case WM_LBUTTONDOWN:
	{
		if (IsCameraHere((int)LOWORD(lParam), (int)HIWORD(lParam))) {
			CameraDrag = true;
			DragStart.x = 2.0f*(float)LOWORD(lParam) / (float)Width - 1;
			DragStart.y = 1 - 2.0f * (float)HIWORD(lParam) / (float)Height;
		}

		SetFocus(hwnd);

#ifdef _DEBUG
		std::stringstream s;
		s << "LOWORD(lParam)=" << LOWORD(lParam) << ", HIWORD(lParam)=" << HIWORD(lParam) << ",LOWORD(wParam)=" << LOWORD(wParam) << ", HIWORD(wParam)=" << HIWORD(wParam);
		DebugMessage(dwi, s.str());

#endif // _DEBUG

		//Scene->SetCameraPositionFromMiniMapXY(2.0f*(float)LOWORD(lParam) / (float)Width - 1, 1 - 2.0f * (float)HIWORD(lParam) / (float)Height, 0);
		break;
	}
	case WM_LBUTTONUP: {
		CameraDrag = false;
		break;
	}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CMinimap::ReshapeGL(int width, int height)
{
	Height = height;
	Width = width;
	if (hDC && hRC) {
		MakeCurrent();
		glViewport(0, 0, (GLsizei)(width), (GLsizei)(height));
	}
}

glm::mat4 CMinimap::GetProjMatrix() const
{
	if (Scene && Scene->Camera)
	{
		return Scene->Camera->GetMiniMapProjection();
	}
	return glm::mat4(1.0f);
}

glm::mat4 CMinimap::GetViewMatrix() const
{
	if (Scene && Scene->Camera)
	{
		return Scene->Camera->GetMiniMapView();
	}
	return glm::mat4(1.0f);
}
