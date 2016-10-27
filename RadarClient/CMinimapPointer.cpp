//#define WIN32_LEAN_AND_MEAN

#include "CMinimapPointer.h"

#include "C3DObjectVBO.h"
#include "C3DObjectTexture.h"
#include "C3DObjectProgram.h"
#include "CScene.h"
#include "CCamera.h"
#include "CMinimap.h"

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

CMiniMapPointer::~CMiniMapPointer()
{
	auto buffer = (vector<VBOData>*)vbo.at(MiniMap)->GetBuffer();
	if (buffer)
		delete buffer;
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
