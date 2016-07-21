#pragma once
#include "C3DObject.h"
class CLine :
	public C3DObjectModel
{
public:
	CLine(int vpId, glm::vec4 a, glm::vec4 b);
	~CLine();
	void SetPoints(glm::vec4 a, glm::vec4 b);
};

