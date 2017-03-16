#pragma once
#include "C3DObjectModel.h"

class CMiniMapPointer : public C3DObjectModel
{
public:
	CMiniMapPointer(CScene *scene);
	glm::mat4 GetRotateMatrix(CViewPortControl* vpControl);
	glm::mat4 GetTranslateMatrix(CViewPortControl* vpControl);
	void CreateBuffer(C3DObjectVBO *vbo_) override;
};