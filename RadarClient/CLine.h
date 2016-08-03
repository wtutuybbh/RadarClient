#pragma once
#include "C3DObject.h"
enum LineStyle
{
	Simple,
	Endpoints
};
class CLine :
	public C3DObjectModel
{
	LineStyle lineStyle{ Simple };
public:
	CLine(int vpId, glm::vec4 a, glm::vec4 b, LineStyle style);
	~CLine() override;
	void SetPoints(glm::vec4 a, glm::vec4 b, LineStyle style);
};

