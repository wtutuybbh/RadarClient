#include "stdafx.h"
#include "CLine.h"
#include "CViewPortControl.h"
#include "C3DObjectVBO.h"
#include "C3DObjectProgram.h"

CLine::CLine(int vpId, glm::vec4 a, glm::vec4 b, LineStyle style)
{
	vbo.insert_or_assign(Main, new C3DObjectVBO(false));
	tex.insert_or_assign(Main, nullptr);
	prog.insert_or_assign(Main, new C3DObjectProgram("CMarkup.v.glsl", "CMarkup.f.glsl", "vertex", nullptr, nullptr, "color"));
	translateMatrix.insert_or_assign(Main, glm::mat4(1.0f));
	scaleMatrix.insert_or_assign(Main, glm::mat4(1.0f));
	rotateMatrix.insert_or_assign(Main, glm::mat4(1.0f));

	vbo.insert_or_assign(MiniMap, new C3DObjectVBO(false));
	tex.insert_or_assign(MiniMap, nullptr);
	prog.insert_or_assign(MiniMap, new C3DObjectProgram("CMarkup.v.glsl", "CMarkup.f.glsl", "vertex", nullptr, nullptr, "color"));
	translateMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
	scaleMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
	rotateMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));



	vertices = std::make_shared<C3DObjectVertices>(2);
	vertices.get()->SetValues(0, a, glm::vec3(0, 0, 1), glm::vec4(1, 0, 0, 1), glm::vec2(0, 0));
	vertices.get()->SetValues(1, b, glm::vec3(0, 0, 1), glm::vec4(1, 0, 0, 1), glm::vec2(0, 1));
	//vertices.get()->needsReload = true;

	auto idx = vertices->AddIndexArray(2, GL_LINES);
	idx[0] = 0;
	idx[1] = 1;

	vbo.at(Main)->vertices = vertices;
	vbo.at(MiniMap)->vertices = vertices;
	vertices.get()->usesCount = 2;

	c3DObjectModel_TypeName = "CLine";
}

void CLine::SetPoints(glm::vec4 a, glm::vec4 b, LineStyle style)
{
	if (vertices)
	{
		vertices.get()->SetValues(0, a, glm::vec3(0, 0, 1), glm::vec4(1, 0, 0, 1), glm::vec2(0, 0));
		vertices.get()->SetValues(1, b, glm::vec3(0, 0, 1), glm::vec4(1, 0, 0, 1), glm::vec2(0, 1));
		vertices.get()->needsReload = true;
	}

	vbo.at(Main)->NeedsReload = true;
	vbo.at(MiniMap)->NeedsReload = true;
}
