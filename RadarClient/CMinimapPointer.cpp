#include "CMinimapPointer.h"
#include "glm/gtx/vector_angle.hpp"

old_CMinimapPointer::old_CMinimapPointer(CScene *scene) : old_C3DObject::old_C3DObject(true)
{	
	//MiniMapVBOReady = false;
	Scene = scene;
//	this->MiniMapDrawMethodsSequence[1] = (PtrToMethod)(&old_CMinimapPointer::BuildMinimapVBO); 
	MiniMapModel = glm::scale(glm::vec3(0.1, 0.05, 0.1));
	MiniMapView = Scene->Camera->GetMiniMapView();
	MiniMapProj = Scene->Camera->GetMiniMapProjection();
	//OutputDebugString("hello there");
	MiniMapVBOClearAfter = false;
}


old_CMinimapPointer::~old_CMinimapPointer()
{
}



void old_CMinimapPointer::MiniMapDraw(CCamera *cam)
{
	if (!MiniMapVBOReady) {
		MiniMapVBOReady = MiniMapPrepareAndBuildVBO("MinimapPointer.v.glsl", "MinimapPointer.f.glsl", "video.png");
	}

	float azimut = cam->GetAzimut();
	glm::mat4 rot = glm::rotate(azimut + (float)M_PI / 2, glm::vec3(0, 0, 1));

	MiniMapModel = glm::translate(glm::vec3(Scene->CameraXYForMiniMap(), 0)) * rot * glm::scale(glm::vec3(0.1, 0.05, 0.1));

	old_C3DObject::MiniMapDraw(cam);
}

void old_CMinimapPointer::MiniMapBuildVBO()
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


CMiniMapPointer::CMiniMapPointer(int vpId, CScene *scene) :
	C3DObjectModel(vpId, 
		new C3DObjectVBO(false), 
		new C3DObjectTexture("video.png", "tex"), 
		new C3DObjectProgram("MiniMapPointer.v.glsl", "MiniMapPointer.f.glsl", "vertex", "texcoor", NULL, NULL))
{
	this->Scene = scene;
	std::vector<VBOData> *buffer = new std::vector<VBOData>;
	float y = 1;// (m_Bounds[0].y + m_Bounds[1].y) / 2;
	buffer->push_back({ glm::vec4(-scene->MeshSize.x, y,-scene->MeshSize.z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(1, 0) });
	buffer->push_back({ glm::vec4(-scene->MeshSize.x, y, scene->MeshSize.z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(1, 1) });
	buffer->push_back({ glm::vec4(scene->MeshSize.x, y, scene->MeshSize.z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 1) });

	buffer->push_back({ glm::vec4(scene->MeshSize.x, y, scene->MeshSize.z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 1) });
	buffer->push_back({ glm::vec4(scene->MeshSize.x, y, -scene->MeshSize.z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	buffer->push_back({ glm::vec4(-scene->MeshSize.x, y, -scene->MeshSize.z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(1, 0) });

	vbo.at(vpId)->SetBuffer(buffer, &(*buffer)[0], buffer->size());

	scaleMatrix.insert_or_assign(vpId, glm::scale(glm::vec3(0.03, 0.015, 0.03)));
}

glm::mat4 CMiniMapPointer::GetRotateMatrix(CViewPortControl* vpControl)
{
	return glm::rotate(Scene->Camera->GetAzimut() + (float)M_PI / 2, glm::vec3(0, 1, 0));
}

glm::mat4 CMiniMapPointer::GetTranslateMatrix(CViewPortControl* vpControl)
{
	glm::vec3 pos = Scene->Camera->GetPosition();
	pos.y = 1;
	return glm::translate(pos);
}
