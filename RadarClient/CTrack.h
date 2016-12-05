#pragma once
#include "stdafx.h"
#include "CSector.h"
//#include #include "glm/glm.hpp"

class CViewPortControl;
struct RDRTRACK;

class CTrack :	public CSector
{
protected:
	static const std::string requestID;
public:
	int ID {-1};
	bool Found;
	bool Selected;
	CTrack(int id, bool selected);
	~CTrack() override;
	void Refresh(glm::vec4 origin, float mpph, float mppv, std::vector<RDRTRACK*>* trackPoints);
	void SelectTrack(int vpId, bool selectState);
};

