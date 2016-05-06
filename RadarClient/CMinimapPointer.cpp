#include "CMinimapPointer.h"
#include "glm/gtx/vector_angle.hpp"

CMinimapPointer::CMinimapPointer(CScene *scene) : C3DObject::C3DObject(true)
{	
	//MiniMapVBOReady = false;
	Scene = scene;
//	this->MiniMapDrawMethodsSequence[1] = (PtrToMethod)(&CMinimapPointer::BuildMinimapVBO); 
	MiniMapModel = glm::scale(glm::vec3(0.1, 0.05, 0.1));
	MiniMapView = Scene->Camera->GetMiniMapView();
	MiniMapProj = Scene->Camera->GetMiniMapProjection();
	//OutputDebugString("hello there");
	MiniMapVBOClearAfter = false;
}


CMinimapPointer::~CMinimapPointer()
{
}

void CMinimapPointer::MiniMapDraw(CCamera *cam)
{
	if (!MiniMapVBOReady) {
		MiniMapVBOReady = MiniMapPrepareAndBuildVBO("MinimapPointer.v.glsl", "MinimapPointer.f.glsl", "video.png");
	}

	float azimut = cam->GetAzimut();
	glm::mat4 rot = glm::rotate(azimut + (float)M_PI / 2, glm::vec3(0, 0, 1));

	MiniMapModel = glm::translate(glm::vec3(Scene->CameraXYForMiniMap(), 0)) * rot * glm::scale(glm::vec3(0.1, 0.05, 0.1));

	C3DObject::MiniMapDraw(cam);
}

void CMinimapPointer::MiniMapBuildVBO()
{
	float t = 0.5f;
	
	MiniMapVBOBuffer.push_back({ glm::vec4(-1, -1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, (1-t)/2) });
	MiniMapVBOBuffer.push_back({ glm::vec4(-1, 1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, (1-t)/2+t) });
	MiniMapVBOBuffer.push_back({ glm::vec4(1, 1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(1, (1 - t) / 2 + t) });

	MiniMapVBOBuffer.push_back({ glm::vec4(1, 1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(1, (1 - t) / 2 + t) });
	MiniMapVBOBuffer.push_back({ glm::vec4(1, -1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(1, (1 - t) / 2) });
	MiniMapVBOBuffer.push_back({ glm::vec4(-1, -1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, (1 - t) / 2) });

	MiniMapVBOBufferSize = MiniMapVBOBuffer.size();
}