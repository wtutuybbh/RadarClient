#pragma once
#include "C3DObjectModel.h"

class CMiniMapPointer : public C3DObjectModel
{
public:
	CMiniMapPointer(int vpId, CScene *scene);
	~CMiniMapPointer() override;
	glm::mat4 GetRotateMatrix(CViewPortControl* vpControl);
	glm::mat4 GetTranslateMatrix(CViewPortControl* vpControl);
};