#pragma once

#include "stdafx.h"

#include "Util.h"



typedef struct {									// Structure For Keyboard Stuff
	BOOL keyDown[256];								// Holds TRUE / FALSE For Each Key
} Keys;												// Keys

typedef struct {									// Contains Information Vital To Applications
	HINSTANCE		hInstance;						// Application Instance
	const wchar_t*		className;						// Application ClassName
} Application;										// Application

typedef struct {									// Window Creation Info
	Application*		application;				// Application Structure
	wchar_t*				title;						// Window Title
	int					width;						// Width
	int					height;						// Height
	int					bitsPerPixel;				// Bits Per Pixel
													//BOOL				isFullScreen;				// FullScreen?
} GL_WindowInit;									// GL_WindowInit

typedef struct {									// Contains Information Vital To A Window
	Keys*				keys;						// Key Structure
	HWND				hWnd;						// Window Handle
	HDC					hDC;						// Device Context
	HGLRC				hRC;						// Rendering Context
	GL_WindowInit		init;						// Window Init
	BOOL				isVisible;					// Window Visible?
	DWORD				lastTickCount;				// Tick Counter
} GL_Window;

										// GL_Window

void TerminateApplication(GL_Window* window);		// Terminate The Application

void ToggleFullscreen(GL_Window* window);			// Toggle Fullscreen / Windowed Mode

													// These Are The Function You Must Provide
BOOL Initialize();	// Performs All Your Initialization

int Deinitialize(void);							// Performs All Your DeInitialization

void Update(DWORD milliseconds);					// Perform Motion Updates

void Draw(void);									// Perform All Your Scene Drawing

void GLProc();
//typedef glm::tvec3<GLdouble, glm::precision::defaultp> 

void LookAtCallback_(double eyex, double eyey, double eyez, double centerx, double centery, double centerz, double upx, double upy, double upz);
												// GL_FRAMEWORK__INCLUDED