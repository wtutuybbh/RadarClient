#include "stdafx.h"

#include "CRCPoint.h"

#include "CUserInterface.h"
#include "CViewPortControl.h"
#include "CSettings.h"
#include "C3DObjectVBO.h"
#include "C3DObjectProgram.h"


C3DObjectVBO* CRCPointModel::vbo_s = nullptr;
C3DObjectProgram* CRCPointModel::prog_s = nullptr;

void CRCPointModel::InitStructure()
{
	CRCPointModel::vbo_s = new C3DObjectVBO(false);
	std::vector<VBOData>* buffer = CRCPointModel::CreateSphereR1(1);
	vbo_s->SetBuffer(buffer, &(*buffer)[0], buffer->size());

	CRCPointModel::prog_s = new C3DObjectProgram("CRCPoint.v.glsl", "CRCPoint.f.glsl", "vertex", nullptr, "normal", nullptr);
}

CRCPointModel::CRCPointModel(int vpId, float y0, float mpph, float mppv, float r, float a, float e): C3DObjectModel(vpId, CRCPointModel::vbo_s, nullptr, CRCPointModel::prog_s)
{
#if defined(CRCPOINT_CONSTRUCTOR_USES_RADIANS)
	cartesianCoords = glm::vec3(-r * sin(a) * cos(e) / mpph, y0 + r * sin(e) / mppv, r * cos(a) * cos(e) / mpph); //we always add y0 (height of the radar relative to sea level) to all cartesian coordinates 
#else
	float re = glm::radians(e);
	float ra = glm::radians(a);
	CartesianCoords = glm::vec3(-r * sin(ra) * cos(re) / mpph, y0 + r * sin(re) / mppv, r * cos(ra) * cos(re) / mpph);
#endif

	sphericalCoords = glm::vec3(r, a, e); // Well, it's not true spherical coordinates. Vertical axis is Y, angle E is from horizon to R (not from vertical Y). A=0 means that point is on the X=0 plane.

	translateMatrix.insert_or_assign(vpId, glm::translate(cartesianCoords));
	rotateMatrix.insert_or_assign(vpId, glm::mat4(1.0f));
	scaleMatrix.insert_or_assign(vpId, glm::mat4(10.0f));
	pixelSize = 2;

	Color = CSettings::GetColor(ColorPointHighLevel);
}

CRCPointModel::~CRCPointModel()
{
	auto obj = vbo.at(Main);

	vector<VBOData>* buffer = nullptr;
	if (obj) 
		buffer = (vector<VBOData>*)obj->GetBuffer();

	if (buffer)
		delete buffer;
}

std::vector<VBOData>* CRCPointModel::CreateSphereR1(int level)
{
	if (level < 0)
		level = 0;

	std::vector<VBOData>* buffer = new std::vector<VBOData>;

	buffer->push_back({ glm::vec4(1, 0, 0, 1), glm::vec3(1, 0, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	buffer->push_back({ glm::vec4(0, 1, 0, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	buffer->push_back({ glm::vec4(0, 0, 1, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });

	buffer->push_back({ glm::vec4(0, 0, 1, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	buffer->push_back({ glm::vec4(0, 1, 0, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	buffer->push_back({ glm::vec4(-1, 0, 0, 1), glm::vec3(-1, 0, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });

	buffer->push_back({ glm::vec4(-1, 0, 0, 1), glm::vec3(-1, 0, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	buffer->push_back({ glm::vec4(0, 1, 0, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	buffer->push_back({ glm::vec4(0, 0, -1, 1), glm::vec3(0, 0, -1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });

	buffer->push_back({ glm::vec4(0, 0, -1, 1), glm::vec3(0, 0, -1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	buffer->push_back({ glm::vec4(0, 1, 0, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	buffer->push_back({ glm::vec4(1, 0, 0, 1), glm::vec3(1, 0, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });

	buffer->push_back({ glm::vec4(1, 0, 0, 1), glm::vec3(1, 0, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	buffer->push_back({ glm::vec4(0, 0, 1, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	buffer->push_back({ glm::vec4(0, -1, 0, 1), glm::vec3(0, -1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });

	buffer->push_back({ glm::vec4(0, 0, 1, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	buffer->push_back({ glm::vec4(-1, 0, 0, 1), glm::vec3(-1, 0, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	buffer->push_back({ glm::vec4(0, -1, 0, 1), glm::vec3(0, -1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });

	buffer->push_back({ glm::vec4(-1, 0, 0, 1), glm::vec3(-1, 0, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	buffer->push_back({ glm::vec4(0, 0, -1, 1), glm::vec3(0, 0, -1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	buffer->push_back({ glm::vec4(0, -1, 0, 1), glm::vec3(0, -1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });

	buffer->push_back({ glm::vec4(0, 0, -1, 1), glm::vec3(0, 0, -1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	buffer->push_back({ glm::vec4(1, 0, 0, 1), glm::vec3(1, 0, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	buffer->push_back({ glm::vec4(0, -1, 0, 1), glm::vec3(0, -1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });


	if (level == 0) {
		return buffer;
	}

	//glm::mat3 mat3_05 = glm::mat3(0.5);
	//glm::mat4 mat4_05 = glm::mat4(0.5);
	float mat3_05 = 0.5f;
	float mat4_05 = 0.5f;

	int N = 8 * pow(4, level - 1);

	for (int i = 0; i < N; i++) {

		VBOData d01 = { mat4_05*((*buffer)[0].vert + (*buffer)[1].vert), mat3_05*((*buffer)[0].norm + (*buffer)[1].norm), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) };
		VBOData d12 = { mat4_05*((*buffer)[2].vert + (*buffer)[1].vert), mat3_05*((*buffer)[2].norm + (*buffer)[1].norm), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) };
		VBOData d20 = { mat4_05*((*buffer)[0].vert + (*buffer)[2].vert), mat3_05*((*buffer)[0].norm + (*buffer)[2].norm), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) };

		buffer->push_back((*buffer)[0]);
		buffer->push_back(d01);
		buffer->push_back(d20);

		buffer->push_back(d01);
		buffer->push_back((*buffer)[1]);
		buffer->push_back(d12);

		buffer->push_back(d12);
		buffer->push_back((*buffer)[2]);
		buffer->push_back(d20);

		buffer->push_back(d01);
		buffer->push_back(d12);
		buffer->push_back(d20);

		buffer->erase(buffer->begin(), buffer->begin() + 3);
	}
	for (int i = 0; i < buffer->size(); i++) {
		(*buffer)[i].vert = glm::vec4(glm::normalize(glm::vec3((*buffer)[i].vert)), 1);
		(*buffer)[i].norm = glm::normalize((*buffer)[i].norm);
	}
	return buffer;
}

glm::mat4 CRCPointModel::GetScaleMatrix(CViewPortControl* vpControl)
{
	//return scaleMatrix.at(vpControl->Id);

	glm::vec4 viewport = glm::vec4(0, 0, vpControl->Width, vpControl->Height);

	//glm::vec3 p1 = glm::unProject(glm::vec3(x, Height - y - 1, 1.0f), Camera->GetView(), Camera->GetProjection(), viewport);

	glm::vec4 radarPoint = glm::vec4(cartesianCoords, 1);

	glm::mat4 mvp = vpControl->GetProjMatrix() * vpControl->GetViewMatrix();
	glm::vec4 screenPoint = mvp * radarPoint;

	screenPoint = screenPoint / screenPoint.w;

	glm::vec3 p00 = glm::vec3((screenPoint.x + 1)*vpControl->Width / 2 + 1, vpControl->Height + (screenPoint.y - 1)*vpControl->Height / 2, 0);
	glm::vec3 p01 = glm::vec3((screenPoint.x + 1)*vpControl->Width / 2 + 1, vpControl->Height + (screenPoint.y - 1)*vpControl->Height / 2, 1);

	glm::vec3 p10 = glm::unProject(p00, vpControl->GetViewMatrix(), vpControl->GetProjMatrix(), viewport);
	glm::vec3 p11 = glm::unProject(p01, vpControl->GetViewMatrix(), vpControl->GetProjMatrix(), viewport);


	float scaleFactor = pixelSize * glm::length(glm::cross(p10 - glm::vec3(radarPoint), p10 - p11)) / glm::length(p10 - p11);
	
	glm::mat4 scMatrix = glm::scale(glm::vec3(
		scaleFactor,
		scaleFactor,
		scaleFactor));

	scaleMatrix.insert_or_assign(vpControl->Id, scMatrix);

	return scMatrix;
}

glm::mat4 CRCPointModel::GetTranslateMatrix(CViewPortControl* vpControl)
{
	return glm::translate(cartesianCoords);
}

void CRCPointModel::BindUniforms(CViewPortControl* vpControl)
{
	glm::mat4 mv = vpControl->GetViewMatrix() * GetModelMatrix(vpControl);
	glm::mat4 mvp = vpControl->GetProjMatrix() * mv;
	glm::mat3 norm = glm::mat3(1.0f); // glm::mat3(glm::transpose(glm::inverse(mv)));
	//glm::mat3(1.0f);

	int mvp_loc = prog.at(vpControl->Id)->GetUniformLocation("mvp");
	int norm_loc = prog.at(vpControl->Id)->GetUniformLocation("norm");
	int color_loc = prog.at(vpControl->Id)->GetUniformLocation("color");

	glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, glm::value_ptr(mvp));
	glUniformMatrix3fv(norm_loc, 1, GL_FALSE, glm::value_ptr(norm));
	glUniform4fv(color_loc, 1, glm::value_ptr(Color));
}
