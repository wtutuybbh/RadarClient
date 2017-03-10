#include "stdafx.h"

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
		new C3DObjectProgram("MiniMapPointer.v.glsl", "MiniMapPointer.f.glsl", "vertex", "texcoor", nullptr, nullptr))
{
	c3DObjectModel_TypeName = "CMiniMapPointer";
	this->Scene = scene;
	

	scaleMatrix.insert_or_assign(vpId, glm::scale(glm::vec3(0.03f, 0.015f, 0.03f)));
}
void CMiniMapPointer::CreateBuffer(C3DObjectVBO *vbo_) {
	if (vbo_ && Scene && Scene->MeshReady()) {
		std::vector<VBOData> *buffer = new std::vector<VBOData>;
		float y = 1.0f;
		auto meshSize = Scene->GetMeshSize();
		
		buffer->push_back({ glm::vec4(-meshSize.x, y,-meshSize.z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(1, 0) });
		buffer->push_back({ glm::vec4(-meshSize.x, y, meshSize.z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(1, 1) });
		buffer->push_back({ glm::vec4(meshSize.x, y, meshSize.z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 1) });

		buffer->push_back({ glm::vec4(meshSize.x, y, meshSize.z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 1) });
		buffer->push_back({ glm::vec4(meshSize.x, y, -meshSize.z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
		buffer->push_back({ glm::vec4(-meshSize.x, y, -meshSize.z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(1, 0) });

		vbo_->SetVBuffer(buffer);
	}
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
