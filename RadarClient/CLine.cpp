#include "stdafx.h"
#include "CLine.h"
#include "CViewPortControl.h"
#include "C3DObjectVBO.h"
#include "C3DObjectProgram.h"

CLine::CLine(int vpId, glm::vec4 a, glm::vec4 b, LineStyle style) : C3DObjectModel(Main,
	new C3DObjectVBO(false),
	nullptr,
	new C3DObjectProgram("CMarkup.v.glsl", "CMarkup.f.glsl", "vertex", nullptr, nullptr, "color"))
{
	vector<VBOData> *buffer = new vector<VBOData>(2);
	buffer->at(0).vert = a;
	buffer->at(0).color = glm::vec4(1, 0, 0, 1);
	buffer->at(1).vert = b;
	buffer->at(1).color = glm::vec4(1, 0, 0, 1);

	C3DObjectVBO *mmvbo = new C3DObjectVBO(false);

	vbo.at(Main)->SetBuffer(buffer, &(*buffer)[0], buffer->size());
	mmvbo->SetBuffer(buffer, &(*buffer)[0], buffer->size());

	vbo.insert_or_assign(MiniMap, mmvbo);

	prog.insert_or_assign(MiniMap, new C3DObjectProgram("CMarkup.v.glsl", "CMarkup.f.glsl", "vertex", nullptr, nullptr, "color"));

	tex.insert_or_assign(MiniMap, nullptr);


	scaleMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
	rotateMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
	translateMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
}

CLine::~CLine()
{
	auto buffer = (vector<VBOData>*)vbo.at(Main)->GetBuffer();
	if (buffer)
		delete buffer;
}

void CLine::SetPoints(glm::vec4 a, glm::vec4 b, LineStyle style)
{
	C3DObjectVBO *vbo_ = vbo.at(Main);

	vector<VBOData> *buffer = (vector<VBOData> *)vbo_->GetBuffer();

	if (!buffer || buffer->size() == 0)
		return;

	buffer->at(0).vert = a;
	buffer->at(1).vert = b;

	vbo_->NeedsReload = true;
	vbo_ = vbo.at(MiniMap);
	if (vbo_) {
		vbo_->NeedsReload = true;
	}
}
