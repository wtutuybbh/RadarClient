#include "CMinimapPointer.h"


CMinimapPointer::CMinimapPointer() : C3DObject::C3DObject(true)
{	
	//MiniMapVBOReady = false;
}


CMinimapPointer::~CMinimapPointer()
{
}

void CMinimapPointer::DrawMiniMap()
{
	if (!C3DObject::MiniMapVBOReady) {
		C3DObject::MiniMapVBOReady = PrepareAndBuildMinimapVBO("MinimapPointer.v.glsl", "MinimapPointer.f.glsl", "video.png");
	}

	C3DObject::DrawMiniMap();
}