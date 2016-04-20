//#include "stdafx.h"
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut.h>

#include "CRCPoint.h"
#include "ShaderUtils.h"

//#include "CScene.h"

GLuint CRCPoint::program;
std::vector<VBOData> CRCPoint::VBOBuffer;
unsigned int CRCPoint::VBOName, CRCPoint::VAOName;
GLuint CRCPoint::mvp_unif_loc;
GLuint CRCPoint::norm_unif_loc;
int CRCPoint::VBOBufferSize;


void CRCPoint::CreateSphereR1(int level)
{
	if (level < 0)
		level = 0;

	/*VBOBuffer.push_back({ glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f) });
	VBOBuffer.push_back({ glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f) });
	VBOBuffer.push_back({ glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f) });*/

	
	VBOBuffer.push_back({ glm::vec4(1, 0, 0, 1), glm::vec3(1, 0, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer.push_back({ glm::vec4(0, 1, 0, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer.push_back({ glm::vec4(0, 0, 1, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	
	VBOBuffer.push_back({ glm::vec4(0, 0, 1, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer.push_back({ glm::vec4(0, 1, 0, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer.push_back({ glm::vec4(-1, 0, 0, 1), glm::vec3(-1, 0, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });

	VBOBuffer.push_back({ glm::vec4(-1, 0, 0, 1), glm::vec3(-1, 0, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer.push_back({ glm::vec4(0, 1, 0, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer.push_back({ glm::vec4(0, 0, -1, 1), glm::vec3(0, 0, -1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });

	VBOBuffer.push_back({ glm::vec4(0, 0, -1, 1), glm::vec3(0, 0, -1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer.push_back({ glm::vec4(0, 1, 0, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });	
	VBOBuffer.push_back({ glm::vec4(1, 0, 0, 1), glm::vec3(1, 0, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });

	VBOBuffer.push_back({ glm::vec4(1, 0, 0, 1), glm::vec3(1, 0, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer.push_back({ glm::vec4(0, 0, 1, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer.push_back({ glm::vec4(0, -1, 0, 1), glm::vec3(0, -1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	
	VBOBuffer.push_back({ glm::vec4(0, 0, 1, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer.push_back({ glm::vec4(-1, 0, 0, 1), glm::vec3(-1, 0, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer.push_back({ glm::vec4(0, -1, 0, 1), glm::vec3(0, -1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	
	VBOBuffer.push_back({ glm::vec4(-1, 0, 0, 1), glm::vec3(-1, 0, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer.push_back({ glm::vec4(0, 0, -1, 1), glm::vec3(0, 0, -1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer.push_back({ glm::vec4(0, -1, 0, 1), glm::vec3(0, -1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	
	VBOBuffer.push_back({ glm::vec4(0, 0, -1, 1), glm::vec3(0, 0, -1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer.push_back({ glm::vec4(1, 0, 0, 1), glm::vec3(1, 0, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	VBOBuffer.push_back({ glm::vec4(0, -1, 0, 1), glm::vec3(0, -1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	
	
	if (level == 0)
		return;

	glm::mat3 mat3_05 = glm::mat3(0.5);
	glm::mat4 mat4_05 = glm::mat4(0.5);

	int N = 8 * pow(4, level - 1);

	for (int i = 0; i < N; i++) {
		
		VBOData d01 = { mat4_05*(VBOBuffer[0].vert + VBOBuffer[1].vert), mat3_05*(VBOBuffer[0].norm + VBOBuffer[1].norm), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) };
		VBOData d12 = { mat4_05*(VBOBuffer[2].vert + VBOBuffer[1].vert), mat3_05*(VBOBuffer[2].norm + VBOBuffer[1].norm), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) };
		VBOData d20 = { mat4_05*(VBOBuffer[0].vert + VBOBuffer[2].vert), mat3_05*(VBOBuffer[0].norm + VBOBuffer[2].norm), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) };

		VBOBuffer.push_back(VBOBuffer[0]);
		VBOBuffer.push_back(d01);
		VBOBuffer.push_back(d20);

		VBOBuffer.push_back(d01);
		VBOBuffer.push_back(VBOBuffer[1]);
		VBOBuffer.push_back(d12);

		VBOBuffer.push_back(d12);
		VBOBuffer.push_back(VBOBuffer[2]);
		VBOBuffer.push_back(d20);

		VBOBuffer.push_back(d01);
		VBOBuffer.push_back(d12);
		VBOBuffer.push_back(d20);

		VBOBuffer.erase(VBOBuffer.begin(), VBOBuffer.begin()+3);
	}
	for (int i = 0; i < VBOBuffer.size(); i++) {
		VBOBuffer[i].vert = glm::vec4(glm::normalize(glm::vec3(VBOBuffer[i].vert)), 1);
		VBOBuffer[i].norm = glm::normalize(VBOBuffer[i].norm);
	}
	VBOBufferSize = VBOBuffer.size();
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
	program = create_program("CRCPoint.v.glsl", "CRCPoint.f.glsl");
	
}

void CRCPoint::UseProgram()
{
	glUseProgram(program);

	
}

void CRCPoint::PrepareVBO()
{
	CreateSphereR1(4);
	//CVec 
}

void CRCPoint::BuildVBO()
{
	CRCPoint::LoadShaders();

	glGenVertexArrays(1, &VAOName);
	glBindVertexArray(VAOName);

	glGenBuffers(1, &VBOName);
	glBindBuffer(GL_ARRAY_BUFFER, VBOName);	
	glBufferData(GL_ARRAY_BUFFER, VBOBuffer.size() * sizeof(VBOData), &VBOBuffer[0], GL_STATIC_DRAW);
	
	VBOBuffer.clear(); //destroy all vbo buffer objects
	std::vector<VBOData>().swap(VBOBuffer); //free memory used by vector itself

	GLuint vertex_attr_loc;
	GLuint normal_attr_loc;
	

	vertex_attr_loc = glGetAttribLocation(program, "vertex");
	normal_attr_loc = glGetAttribLocation(program, "normal");

	


	glVertexAttribPointer(vertex_attr_loc, 4, GL_FLOAT, GL_FALSE, sizeof(VBOData), (void*)0);
	glVertexAttribPointer(normal_attr_loc, 3, GL_FLOAT, GL_FALSE, sizeof(VBOData), (void*)(sizeof(float) * 4));

	glEnableVertexAttribArray(vertex_attr_loc);
	glEnableVertexAttribArray(normal_attr_loc);

	mvp_unif_loc = glGetUniformLocation(program, "mvp");
	norm_unif_loc = glGetUniformLocation(program, "norm");
	

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void CRCPoint::Draw(CCamera *cam)
{
	glBindVertexArray(VAOName);
	
	glm::mat4 mv = cam->GetView() * model;
	mvp = cam->GetProjection() * mv;
	norm = glm::mat3(glm::transpose(glm::inverse(mv)));
		//glm::mat3(1.0f);

	

	glUniformMatrix4fv(CRCPoint::mvp_unif_loc, 1, GL_FALSE, glm::value_ptr(mvp));
	glUniformMatrix3fv(CRCPoint::norm_unif_loc, 1, GL_FALSE, glm::value_ptr(norm));

	glDrawArrays(GL_TRIANGLES, 0, CRCPoint::VBOBufferSize);

	glBindVertexArray(0);
	
}

CVec * CRCPoint::GetBounds()
{
	return nullptr;
}
