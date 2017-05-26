#pragma once
#include "CMarkup.h"

class CRay :
	public CMarkup
{
	float rayWidth, maxDist, y_0, MPPh, MPPv;
public:
	CRay(float rayWidth, float maxDist, float y_0);
	~CRay();
};

