#pragma once

#include "stdafx.h"
#include "C3DObjectModel.h"

#define _USE_MATH_DEFINES 



#define POINT_SIZE 5.0f

class CScene;
struct VBOData;

class CRCPointModel : public C3DObjectModel
{
public:
	static C3DObjectVBO* vbo_s;
	static C3DObjectProgram* prog_s;

	float pixelSize;

	static void InitStructure();
	CRCPointModel(int vpId, float y0, float mpph, float mppv, float r, float a, float e);
	~CRCPointModel() override;
	static std::vector<VBOData>* CreateSphereR1(int level);
	glm::mat4 GetScaleMatrix(CViewPortControl *vpControl) override;
	glm::mat4 GetTranslateMatrix(CViewPortControl *vpControl) override;
	void BindUniforms(CViewPortControl *vpControl) override;
};