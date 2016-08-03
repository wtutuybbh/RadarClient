//#include "stdafx.h"
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut.h>

#include "CRCPoint.h"
#include "ShaderUtils.h"
#include "glm/glm.hpp"
#include "glm/gtx/intersect.hpp"
//#include "CScene.h"
#include "CUserInterface.h"
#include "CViewPortControl.h"
#include <unordered_map>
#include "CSettings.h"

//GLuint old_CRCPoint::ProgramID_s;
//std::vector<VBOData> old_CRCPoint::VBOBuffer_s;
//unsigned int old_CRCPoint::VBOName_s, old_CRCPoint::VAOName_s;
//GLuint old_CRCPoint::MVPUniformLoc_s;
//GLuint old_CRCPoint::NormUniformLoc_s;
//int old_CRCPoint::VBOBufferSize_s;


/*void old_CRCPoint::CreateSphereR1(int level)
{
	if (level < 0)
		level = 0;

	VBOBuffer_s.push_back({ glm::vec4(1, 0, 0, 1), glm::vec3(1, 0, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer_s.push_back({ glm::vec4(0, 1, 0, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer_s.push_back({ glm::vec4(0, 0, 1, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	
	VBOBuffer_s.push_back({ glm::vec4(0, 0, 1, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer_s.push_back({ glm::vec4(0, 1, 0, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer_s.push_back({ glm::vec4(-1, 0, 0, 1), glm::vec3(-1, 0, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });

	VBOBuffer_s.push_back({ glm::vec4(-1, 0, 0, 1), glm::vec3(-1, 0, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer_s.push_back({ glm::vec4(0, 1, 0, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer_s.push_back({ glm::vec4(0, 0, -1, 1), glm::vec3(0, 0, -1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });

	VBOBuffer_s.push_back({ glm::vec4(0, 0, -1, 1), glm::vec3(0, 0, -1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer_s.push_back({ glm::vec4(0, 1, 0, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });	
	VBOBuffer_s.push_back({ glm::vec4(1, 0, 0, 1), glm::vec3(1, 0, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });

	VBOBuffer_s.push_back({ glm::vec4(1, 0, 0, 1), glm::vec3(1, 0, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer_s.push_back({ glm::vec4(0, 0, 1, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer_s.push_back({ glm::vec4(0, -1, 0, 1), glm::vec3(0, -1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	
	VBOBuffer_s.push_back({ glm::vec4(0, 0, 1, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer_s.push_back({ glm::vec4(-1, 0, 0, 1), glm::vec3(-1, 0, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer_s.push_back({ glm::vec4(0, -1, 0, 1), glm::vec3(0, -1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	
	VBOBuffer_s.push_back({ glm::vec4(-1, 0, 0, 1), glm::vec3(-1, 0, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer_s.push_back({ glm::vec4(0, 0, -1, 1), glm::vec3(0, 0, -1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer_s.push_back({ glm::vec4(0, -1, 0, 1), glm::vec3(0, -1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	
	VBOBuffer_s.push_back({ glm::vec4(0, 0, -1, 1), glm::vec3(0, 0, -1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer_s.push_back({ glm::vec4(1, 0, 0, 1), glm::vec3(1, 0, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer_s.push_back({ glm::vec4(0, -1, 0, 1), glm::vec3(0, -1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	
	
	if (level == 0) {
		VBOBufferSize_s = VBOBuffer_s.size();
		return;
	}

	glm::mat3 mat3_05 = glm::mat3(0.5);
	glm::mat4 mat4_05 = glm::mat4(0.5);

	int N = 8 * pow(4, level - 1);

	for (int i = 0; i < N; i++) {
		
		VBOData d01 = { mat4_05*(VBOBuffer_s[0].vert + VBOBuffer_s[1].vert), mat3_05*(VBOBuffer_s[0].norm + VBOBuffer_s[1].norm), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) };
		VBOData d12 = { mat4_05*(VBOBuffer_s[2].vert + VBOBuffer_s[1].vert), mat3_05*(VBOBuffer_s[2].norm + VBOBuffer_s[1].norm), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) };
		VBOData d20 = { mat4_05*(VBOBuffer_s[0].vert + VBOBuffer_s[2].vert), mat3_05*(VBOBuffer_s[0].norm + VBOBuffer_s[2].norm), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) };

		VBOBuffer_s.push_back(VBOBuffer_s[0]);
		VBOBuffer_s.push_back(d01);
		VBOBuffer_s.push_back(d20);

		VBOBuffer_s.push_back(d01);
		VBOBuffer_s.push_back(VBOBuffer_s[1]);
		VBOBuffer_s.push_back(d12);

		VBOBuffer_s.push_back(d12);
		VBOBuffer_s.push_back(VBOBuffer_s[2]);
		VBOBuffer_s.push_back(d20);

		VBOBuffer_s.push_back(d01);
		VBOBuffer_s.push_back(d12);
		VBOBuffer_s.push_back(d20);

		VBOBuffer_s.erase(VBOBuffer_s.begin(), VBOBuffer_s.begin()+3);
	}
	for (int i = 0; i < VBOBuffer_s.size(); i++) {
		VBOBuffer_s[i].vert = glm::vec4(glm::normalize(glm::vec3(VBOBuffer_s[i].vert)), 1);
		VBOBuffer_s[i].norm = glm::normalize(VBOBuffer_s[i].norm);
	}
	VBOBufferSize_s = VBOBuffer_s.size();
}

old_CRCPoint::old_CRCPoint(float y0, float mpph, float mppv, float r, float a, float e): ColorUniformLoc(0), r(0)
{	
#if defined(CRCPOINT_CONSTRUCTOR_USES_RADIANS)
	CartesianCoords = glm::vec3(- r * sin(a) * cos(e) / mpph, y0 + r * sin(e) / mppv, r * cos(a) * cos(e) / mpph); //we always add y0 (height of the radar relative to sea level) to all cartesian coordinates 
#else
	float re = glm::radians(e);
	float ra = glm::radians(a);
	CartesianCoords = glm::vec3(- r * sin(ra) * cos(re) / mpph, y0 + r * sin(re) / mppv, r * cos(ra) * cos(re) / mpph);
#endif

	SphericalCoords = glm::vec3(r, a, e); // Well, it's not true spherical coordinates. Vertical axis is Y, angle E is from horizon to R (not from vertical Y). A=0 means that point is on the X=0 plane.

	//glm::mat4 tr = glm::translate(CartesianCoords), sc = glm::scale(glm::vec3(POINT_SIZE, POINT_SIZE, POINT_SIZE));
	//Model = tr * sc;
	Translate = glm::translate(CartesianCoords);
	Rotate = glm::mat4(1.0f);
}


old_CRCPoint::~old_CRCPoint()
{
}

void old_CRCPoint::LoadShaders_s()
{
	//GLuint old_CRCPoint::program;
	ProgramID_s = create_program("old_CRCPoint.v.glsl", "old_CRCPoint.f.glsl");
	
}

void old_CRCPoint::UseProgram_s()
{
	glUseProgram(ProgramID_s);

	
}

void old_CRCPoint::PrepareVBO_s()
{
	CreateSphereR1(0);
}

void old_CRCPoint::BuildVBO_s()
{
	old_CRCPoint::LoadShaders_s();

	glGenVertexArrays(1, &VAOName_s);
	glBindVertexArray(VAOName_s);

	glGenBuffers(1, &VBOName_s);
	glBindBuffer(GL_ARRAY_BUFFER, VBOName_s);	
	glBufferData(GL_ARRAY_BUFFER, VBOBuffer_s.size() * sizeof(VBOData), &VBOBuffer_s[0], GL_STATIC_DRAW);
	
	//VBOBuffer_s.clear(); //destroy all vbo buffer objects
	//std::vector<VBOData>().swap(VBOBuffer_s); //free memory used by vector itself

	GLuint vertex_attr_loc;
	GLuint normal_attr_loc;

	

	vertex_attr_loc = glGetAttribLocation(ProgramID_s, "vertex");
	normal_attr_loc = glGetAttribLocation(ProgramID_s, "normal");

	


	glVertexAttribPointer(vertex_attr_loc, 4, GL_FLOAT, GL_FALSE, sizeof(VBOData), (void*)0);
	glVertexAttribPointer(normal_attr_loc, 3, GL_FLOAT, GL_FALSE, sizeof(VBOData), (void*)(sizeof(float) * 4));

	glEnableVertexAttribArray(vertex_attr_loc);
	glEnableVertexAttribArray(normal_attr_loc);

	
	

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void old_CRCPoint::Draw(CCamera *cam)
{
	glBindVertexArray(VAOName_s);
	
	glm::mat4 mv = cam->GetView() * GetModelMatrix(Scene);
	MVP = cam->GetProjection() * mv;
	norm = glm::mat3(glm::transpose(glm::inverse(mv)));
		//glm::mat3(1.0f);

	MVPUniformLoc_s = glGetUniformLocation(ProgramID_s, "mvp");
	NormUniformLoc_s = glGetUniformLocation(ProgramID_s, "norm");
	ColorUniformLoc = glGetUniformLocation(ProgramID_s, "color");

	glUniformMatrix4fv(old_CRCPoint::MVPUniformLoc_s, 1, GL_FALSE, glm::value_ptr(MVP));
	glUniformMatrix3fv(old_CRCPoint::NormUniformLoc_s, 1, GL_FALSE, glm::value_ptr(norm));
	glUniform4fv(ColorUniformLoc, 1, glm::value_ptr(Color));

	glDrawArrays(GL_TRIANGLES, 0, old_CRCPoint::VBOBufferSize_s);

	glBindVertexArray(0);
	
}

glm::vec3 * old_CRCPoint::GetBounds()
{
	return nullptr;
}

bool old_CRCPoint::IntersectLine(glm::vec3 & orig, glm::vec3 & dir, glm::vec3 & position)
{
	glm::vec3 vert0, vert1, vert2;
	for (int i = 0; i < VBOBuffer_s.size(); i += 3) {
		vert0 = glm::vec3(Model*VBOBuffer_s[i].vert);
		vert1 = glm::vec3(Model*VBOBuffer_s[i + 1].vert);
		vert2 = glm::vec3(Model*VBOBuffer_s[i + 2].vert);
		if (glm::intersectLineTriangle(orig, dir, vert0, vert1, vert2, position)) {
			return true;
		}
	}
	return false;
}

void old_CRCPoint::SelectObject(CUserInterface* ui)
{

}
glm::mat4 old_CRCPoint::GetModelMatrix(CScene* scn)
{
	glm::vec4 radarPoint = glm::vec4(CartesianCoords, 1);
	glm::vec4 radarPoint1 = radarPoint + glm::vec4(1, 0, 0, 0);
	glm::vec4 radarPoint2 = radarPoint + glm::vec4(0, 1, 0, 0);
	glm::vec4 radarPoint3 = radarPoint + glm::vec4(0, 0, 1, 0);
	glm::mat4 mvp = scn->Camera->GetProjection() * scn->Camera->GetView() * glm::mat4(1.0f);
	glm::vec4 screenPoint = mvp * radarPoint;
	glm::vec4 screenPoint1 = mvp * radarPoint1;
	glm::vec4 screenPoint2 = mvp * radarPoint2;
	glm::vec4 screenPoint3 = mvp * radarPoint3;
	screenPoint = screenPoint / screenPoint.w;
	screenPoint1 = screenPoint1 / screenPoint1.w;
	screenPoint2 = screenPoint2 / screenPoint2.w;
	screenPoint3 = screenPoint3 / screenPoint3.w;

	Scale = glm::scale(glm::vec3(
		1 / glm::distance(screenPoint, screenPoint1) / scn->height,
		1 / glm::distance(screenPoint, screenPoint2) / scn->height,
		1 / glm::distance(screenPoint, screenPoint3) / scn->height));

	return old_C3DObject::GetModelMatrix(scn);
}
*/
C3DObjectVBO* CRCPointModel::vbo_s = NULL;
C3DObjectProgram* CRCPointModel::prog_s = NULL;

void CRCPointModel::InitStructure()
{
	CRCPointModel::vbo_s = new C3DObjectVBO(false);
	vector<VBOData>* buffer = CRCPointModel::CreateSphereR1(1);
	vbo_s->SetBuffer(buffer, &(*buffer)[0], buffer->size());

	CRCPointModel::prog_s = new C3DObjectProgram("CRCPoint.v.glsl", "CRCPoint.f.glsl", "vertex", NULL, "normal", NULL);
}

CRCPointModel::CRCPointModel(int vpId, float y0, float mpph, float mppv, float r, float a, float e): C3DObjectModel(vpId, CRCPointModel::vbo_s, NULL, CRCPointModel::prog_s)
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

	vector<VBOData>* buffer = NULL;
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
