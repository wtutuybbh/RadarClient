#pragma once
#include "C3DObject.h"
#include <vector>

class CMinimapPointer : public C3DObject
{/*
private:
	VBOBuffer*/
private:
	//bool MiniMapVBOReady;

public:
	glm::vec2 Position;
	float Direction; //azimuth angle
	void CMinimapPointer::DrawMiniMap();
	void CMinimapPointer::BuildMinimapVBO();
	CMinimapPointer(CScene *scene);
	~CMinimapPointer();

};

