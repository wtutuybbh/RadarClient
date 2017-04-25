#pragma once
#include "stdafx.h"
#include "CTrack.h"

class CPath : public CTrack
{	
public:
	CPath();
	void AddPoint(glm::vec4 point);
};
