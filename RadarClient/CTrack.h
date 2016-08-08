#pragma once
#include <vector>
#include "CSector.h"
//#include #include "glm/glm.hpp"

class CViewPortControl;
struct RDRTRACK;

class CTrack :	public CSector
{
public:
	int ID;
	bool Found;
	bool Selected;
	CTrack(int id, bool selected);
	~CTrack();
	void Refresh(glm::vec4 origin, float mpph, float mppv, std::vector<RDRTRACK*>* trackPoints);
	void SelectTrack(int vpId, bool selectState);
};

