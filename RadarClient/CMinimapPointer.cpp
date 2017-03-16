#include "stdafx.h"

#include "CMinimapPointer.h"

#include "C3DObjectVBO.h"
#include "C3DObjectTexture.h"
#include "C3DObjectProgram.h"
#include "CScene.h"
#include "CCamera.h"
#include "CMinimap.h"

CMiniMapPointer::CMiniMapPointer(CScene *scene)
{
	c3DObjectModel_TypeName = "CMiniMapPointer";
	this->Scene = scene;
	
	//vbo.insert_or_assign(Main, new C3DObjectVBO(false));
	//tex.insert_or_assign(Main, nullptr);
	//prog.insert_or_assign(Main, new C3DObjectProgram("MiniMapPointer.v.glsl", "MiniMapPointer.f.glsl", "vertex", nullptr, nullptr, "color"));
	//translateMatrix.insert_or_assign(Main, glm::mat4(1.0f));
	//scaleMatrix.insert_or_assign(Main, glm::mat4(1.0f));
	//rotateMatrix.insert_or_assign(Main, glm::mat4(1.0f));

	vbo.insert_or_assign(MiniMap, new C3DObjectVBO(false));
	tex.insert_or_assign(MiniMap, new C3DObjectTexture("video.png", "tex"));
	prog.insert_or_assign(MiniMap, new C3DObjectProgram("MiniMapPointer.v.glsl", "MiniMapPointer.f.glsl", "vertex", nullptr, nullptr, "color"));
	translateMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
	//scaleMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
	rotateMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));

	scaleMatrix.insert_or_assign(MiniMap, glm::scale(glm::vec3(0.03f, 0.015f, 0.03f)));
}
void CMiniMapPointer::CreateBuffer(C3DObjectVBO *vbo_) {

	if (vbo_ && Scene && Scene->MeshReady()) {
		vbo_->vertices = std::make_shared<C3DObjectVertices>(6);
		float y = 1.0f;
		auto meshSize = Scene->GetMeshSize();
		
		vbo_->vertices.get()->SetValues(0, glm::vec4(-meshSize.x, y, -meshSize.z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(1, 0));

		vbo_->vertices.get()->SetValues(1, glm::vec4(-meshSize.x, y, meshSize.z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(1, 1));

		vbo_->vertices.get()->SetValues(2, glm::vec4(meshSize.x, y, meshSize.z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 1));

		vbo_->vertices.get()->SetValues(3, glm::vec4(meshSize.x, y, meshSize.z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 1));

		vbo_->vertices.get()->SetValues(4, glm::vec4(meshSize.x, y, -meshSize.z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0));

		vbo_->vertices.get()->SetValues(5, glm::vec4(-meshSize.x, y, -meshSize.z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(1, 0));

		vbo_->vertices->usesCount = 1;
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
