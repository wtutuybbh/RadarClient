#pragma once
#include "C3DObjectModel.h"

class CMiniMapPointer : public C3DObjectModel
{
public:
	CMiniMapPointer(CScene *scene);
	glm::mat4 GetRotateMatrix(int vpId) override;
	glm::mat4 GetTranslateMatrix(int vpId) override;
	void CreateBuffer(C3DObjectVBO *vbo_) override;
};