#pragma once
#include "C3DObject.h"
class CMarkup :
	public C3DObjectModel
{
public:
	CMarkup(glm::vec4 origin);
	virtual ~CMarkup();
	void BindUniforms(CViewPortControl* vpControl) override;

};

