#include "stdafx.h"
#include "CRay.h"
#include "CViewPortControl.h"
#include "C3DObjectVBO.h"
#include "C3DObjectProgram.h"
#include "CSettings.h"

CRay::CRay(float rayWidth, float maxDist, float y_0) : CMarkup(glm::vec4(0, y_0, 0, 1))
{
	this->rayWidth = 0;// rayWidth;
	this->maxDist = maxDist;
	this->y_0 = y_0;

	MPPh = CSettings::GetFloat(FloatMPPh);
	MPPv = CSettings::GetFloat(FloatMPPv);

	//vbo.insert_or_assign(Main, new C3DObjectVBO(false));
	//vbo.insert_or_assign(MiniMap, new C3DObjectVBO(false));

	//prog.insert_or_assign(Main, new C3DObjectProgram("CMarkup.v.glsl", "CMarkup.f.glsl", "vertex", nullptr, nullptr, "color"));
	//prog.insert_or_assign(MiniMap, new C3DObjectProgram("CMarkup.v.glsl", "CMarkup.f.glsl", "vertex", nullptr, nullptr, "color"));
	rayWidth = 0;
	vertices = std::make_shared<C3DObjectVertices>(3);
	vertices.get()->SetValues(0, glm::vec4(0, y_0, 0, 1), glm::vec3(0, 1, 0), glm::vec4(1, 0, 0, 1), glm::vec2(1, 0));
	vertices.get()->SetValues(1, glm::vec4(maxDist * sin(-rayWidth / 2) / MPPh, y_0, maxDist * cos(-rayWidth / 2) / MPPh, 1), glm::vec3(0, 1, 0), glm::vec4(1, 0, 0, 1), glm::vec2(1, 1));
	vertices.get()->SetValues(2, glm::vec4(maxDist * sin(rayWidth / 2) / MPPh, y_0, maxDist * cos(rayWidth / 2) / MPPh, 1), glm::vec3(0, 1, 0), glm::vec4(1, 0, 0, 1), glm::vec2(0, 1));

	vbo.at(Main)->vertices = vertices;
	vbo.at(MiniMap)->vertices = vertices;
	vertices.get()->usesCount = 2;
}


CRay::~CRay()
{
}

void CRay::Refresh(float angle)
{
	if (vertices)
	{
		vertices.get()->SetValues(1, glm::vec4(maxDist * sin(angle) / MPPh, y_0, maxDist * cos(-angle) / MPPh, 1), glm::vec3(0, 1, 0), glm::vec4(1, 0, 0, 1), glm::vec2(1, 1));
		vertices.get()->SetValues(2, glm::vec4(maxDist * sin(angle) / MPPh, y_0, maxDist * cos(angle) / MPPh, 1), glm::vec3(0, 1, 0), glm::vec4(1, 0, 0, 1), glm::vec2(0, 1));
		vertices->needsReload = 2;
	}
}
