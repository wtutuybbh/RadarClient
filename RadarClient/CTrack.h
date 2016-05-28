#pragma once
#include <vector>
#include "CSector.h"

class CViewPortControl;

class CTrack :	public CSector
{
public:
	int ID;
	bool Found;
	CTrack(int id);
	~CTrack();
	void Refresh(glm::vec4 origin, float mpph, float mppv, vector<RDRTRACK>* trackPoints);
};

