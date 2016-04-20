//#include "stdafx.h"
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut.h>

#include "CRCPoint.h"
#include "ShaderUtils.h"

//#include "CScene.h"

GLuint CRCPoint::ProgramID_s;
std::vector<VBOData> CRCPoint::VBOBuffer_s;
unsigned int CRCPoint::VBOName_s, CRCPoint::VAOName_s;
GLuint CRCPoint::MVPUniformLoc_s;
GLuint CRCPoint::NormUniformLoc_s;
int CRCPoint::VBOBufferSize_s;


void CRCPoint::CreateSphereR1(int level)
{
	if (level < 0)
		level = 0;

	/*VBOBuffer.push_back({ glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f) });
	VBOBuffer.push_back({ glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f) });
	VBOBuffer.push_back({ glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f) });*/

	
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
	
	
	if (level == 0)
		return;

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

CRCPoint::CRCPoint(float y0, float mpph, float mppv, float r, float a, float e)
{	
#if defined(CRCPOINT_CONSTRUCTOR_USES_RADIANS)
	CartesianCoords = glm::vec3(- r * sin(a) * cos(e) / mpph, y0 + r * sin(e) / mppv, r * cos(a) * cos(e) / mpph); //we always add y0 (height of the radar relative to sea level) to all cartesian coordinates 
#else
	float re = glm::radians(e);
	float ra = glm::radians(a);
	CartesianCoords = glm::vec3(- r * sin(ra) * cos(re) / mpph, y0 + r * sin(re) / mppv, r * cos(ra) * cos(re) / mpph);
#endif

	SphericalCoords = glm::vec3(r, a, e); // Well, it's not true spherical coordinates. Vertical axis is Y, angle E is from horizon to R (not from vertical Y). A=0 means that point is on the X=0 plane.

	glm::mat4 tr = glm::translate(CartesianCoords), sc = glm::scale(glm::vec3(POINT_SIZE, POINT_SIZE, POINT_SIZE));
	model = tr * sc;
}


CRCPoint::~CRCPoint()
{
}

void CRCPoint::LoadShaders()
{
	//GLuint CRCPoint::program;
	ProgramID_s = create_program("CRCPoint.v.glsl", "CRCPoint.f.glsl");
	
}

void CRCPoint::UseProgram()
{
	glUseProgram(ProgramID_s);

	
}

void CRCPoint::PrepareVBO()
{
	CreateSphereR1(4);
	//CVec 
}

void CRCPoint::BuildVBO()
{
	CRCPoint::LoadShaders();

	glGenVertexArrays(1, &VAOName_s);
	glBindVertexArray(VAOName_s);

	glGenBuffers(1, &VBOName_s);
	glBindBuffer(GL_ARRAY_BUFFER, VBOName_s);	
	glBufferData(GL_ARRAY_BUFFER, VBOBuffer_s.size() * sizeof(VBOData), &VBOBuffer_s[0], GL_STATIC_DRAW);
	
	VBOBuffer_s.clear(); //destroy all vbo buffer objects
	std::vector<VBOData>().swap(VBOBuffer_s); //free memory used by vector itself

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

void CRCPoint::Draw(CCamera *cam)
{
	glBindVertexArray(VAOName_s);
	
	glm::mat4 mv = cam->GetView() * model;
	mvp = cam->GetProjection() * mv;
	norm = glm::mat3(glm::transpose(glm::inverse(mv)));
		//glm::mat3(1.0f);

	MVPUniformLoc_s = glGetUniformLocation(ProgramID_s, "mvp");
	NormUniformLoc_s = glGetUniformLocation(ProgramID_s, "norm");

	glUniformMatrix4fv(CRCPoint::MVPUniformLoc_s, 1, GL_FALSE, glm::value_ptr(mvp));
	glUniformMatrix3fv(CRCPoint::NormUniformLoc_s, 1, GL_FALSE, glm::value_ptr(norm));

	glDrawArrays(GL_TRIANGLES, 0, CRCPoint::VBOBufferSize_s);

	glBindVertexArray(0);
	
}

CVec * CRCPoint::GetBounds()
{
	return nullptr;
}
