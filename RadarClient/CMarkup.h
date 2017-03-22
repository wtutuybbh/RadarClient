#pragma once
#include "C3DObjectModel.h"
class CMarkup :
	public C3DObjectModel
{
public:
	CMarkup(glm::vec4 origin);
	void BindUniforms(CViewPortControl* vpControl) override;
};

