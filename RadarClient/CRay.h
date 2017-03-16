#pragma once
#include "C3DObjectModel.h"
class CRay :
	public C3DObjectModel
{
	float rayWidth, maxDist, y_0, MPPh, MPPv;
public:
	CRay(float rayWidth, float maxDist, float y_0);
	~CRay();
};

