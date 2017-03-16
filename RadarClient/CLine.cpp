#include "stdafx.h"
#include "CLine.h"
#include "CViewPortControl.h"
#include "C3DObjectVBO.h"
#include "C3DObjectProgram.h"

CLine::CLine(int vpId, glm::vec4 a, glm::vec4 b, LineStyle style) : C3DObjectModel(Main,
	new C3DObjectVBO(false),
	nullptr,
	new C3DObjectProgram("CMarkup.v.glsl", "CMarkup.f.glsl", "vertex", nullptr, nullptr, "color", 13 * sizeof(float)))
{
	vector<VBOData> *buffer = new vector<VBOData>(2);
	buffer->at(0).vert = a;
	buffer->at(0).color = glm::vec4(1, 0, 0, 1);
	buffer->at(1).vert = b;
	buffer->at(1).color = glm::vec4(1, 0, 0, 1);

	C3DObjectVBO *mmvbo = new C3DObjectVBO(false);

	vbo.at(Main)->SetVBuffer(buffer);
	mmvbo->SetVBuffer(buffer);

	vertices = std::make_shared<C3DObjectVertices>(2);
	vertices.get()->SetValues(0, a, glm::vec3(0, 0, 1), glm::vec4(1, 0, 0, 1), glm::vec2(0, 0));
	vertices.get()->SetValues(1, b, glm::vec3(0, 0, 1), glm::vec4(1, 0, 0, 1), glm::vec2(0, 1));
	vertices.get()->needsReload = true;

	vertices->AddIndexArray(2, 0);

	vbo.at(Main)->vertices = vertices;
	mmvbo->vertices = vertices;
	vertices.get()->usesCount = 2;

	vbo.insert_or_assign(MiniMap, mmvbo);

	prog.insert_or_assign(MiniMap, new C3DObjectProgram("CMarkup.v.glsl", "CMarkup.f.glsl", "vertex", nullptr, nullptr, "color", 13 * sizeof(float)));

	tex.insert_or_assign(MiniMap, nullptr);


	scaleMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
	rotateMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
	translateMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));

	c3DObjectModel_TypeName = "CLine";
}

void CLine::SetPoints(glm::vec4 a, glm::vec4 b, LineStyle style)
{
	C3DObjectVBO *vbo_ = vbo.at(Main);

	vector<VBOData> *buffer = (vector<VBOData> *)vbo_->GetVBuffer();

	if (!buffer || buffer->size() == 0)
		return;

	buffer->at(0).vert = a;
	buffer->at(1).vert = b;
	
	if (vertices)
	{
		vertices.get()->SetValues(0, a, glm::vec3(0, 0, 1), glm::vec4(1, 0, 0, 1), glm::vec2(0, 0));
		vertices.get()->SetValues(1, b, glm::vec3(0, 0, 1), glm::vec4(1, 0, 0, 1), glm::vec2(0, 1));
		vertices.get()->needsReload = true;
	}


	vbo_->NeedsReload = true;
	vbo_ = vbo.at(MiniMap);
	if (vbo_) {
		vbo_->NeedsReload = true;
	}	
}
