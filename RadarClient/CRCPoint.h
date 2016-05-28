#pragma once

#include <GL/GL.h>
#include "glm/glm.hpp"
#include <vector>
#include "Util.h"
#include "CCamera.h"
#include "C3DObject.h"

#define _USE_MATH_DEFINES 

#define CRCPOINT_CONSTRUCTOR_USES_RADIANS

#define POINT_SIZE 5.0f

class CScene;

class old_CRCPoint : public old_C3DObject
{
private:
	static void CreateSphereR1(int level);
	static std::vector<VBOData> VBOBuffer_s;
	static int VBOBufferSize_s;
	static unsigned int VBOName_s, VAOName_s;
	static GLuint ProgramID_s;
	static GLuint MVPUniformLoc_s;
	static GLuint NormUniformLoc_s;

	GLuint ColorUniformLoc;

	//glm::mat4 Model, MVP;
	glm::mat3 norm;
	
public:
	
	float r;
	
	old_CRCPoint(float y0, float mpph, float mppv, float r, float a, float e);
	~old_CRCPoint();
	static void LoadShaders_s();
	static void UseProgram_s();
	static void PrepareVBO_s();
	static void BuildVBO_s();
	void Draw(CCamera *cam) override;

	glm::vec3 * GetBounds() override;

	bool IntersectLine(glm::vec3 & orig, glm::vec3 & dir, glm::vec3 & position) override;
	void SelectObject(CUserInterface* ui);
	glm::mat4 GetModelMatrix(CScene* scn) override;
};



class CRCPointModel : public C3DObjectModel
{
public:
	static C3DObjectVBO* vbo_s;
	static C3DObjectProgram* prog_s;

	float pixelSize;

	static void InitStructure();
	CRCPointModel(int vpId, float y0, float mpph, float mppv, float r, float a, float e);
	static std::vector<VBOData>* CreateSphereR1(int level);
	glm::mat4 GetScaleMatrix(CViewPortControl *vpControl) override;
	glm::mat4 GetTranslateMatrix(CViewPortControl *vpControl) override;
	void BindUniforms(CViewPortControl *vpControl) override;
};