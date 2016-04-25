#pragma once
#include "C3DObject.h"
class CRect :
	public C3DObject
{
public:


	CRect();
	~CRect();

	glm::tvec3<float, glm::precision::defaultp> * GetBounds();

};

