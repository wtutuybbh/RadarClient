#pragma once

#include "stdafx.h"
#include "C3DObjectModel.h"

#define _USE_MATH_DEFINES 



#define POINT_SIZE 5.0f

class CScene;

class CRCPointModel : public C3DObjectModel
{
public:
	static C3DObjectVBO* vbo_s;
	static C3DObjectProgram* prog_s;

	float pixelSize;


	CRCPointModel(int vpId, float y_0, float mpph, float mppv, float r, float a, float e);
	void BindUniforms(CViewPortControl *vpControl) override;
};