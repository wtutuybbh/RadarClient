#pragma once
#include "C3DObject.h"
#include <vector>

class old_CMinimapPointer : public old_C3DObject
{/*
private:
	VBOBuffer*/
private:
	//bool MiniMapVBOReady;

public:
	glm::vec2 Position;
	float Direction; //azimuth angle
	void old_CMinimapPointer::MiniMapDraw(CCamera *cam) override;
	void old_CMinimapPointer::MiniMapBuildVBO() override;
	explicit old_CMinimapPointer(CScene *scene);
	~old_CMinimapPointer();

};

class CMiniMapPointer : public C3DObjectModel
{
public:
	CMiniMapPointer(int vpId, CScene *scene);
	glm::mat4 GetRotateMatrix(CViewPortControl* vpControl);
	glm::mat4 GetTranslateMatrix(CViewPortControl* vpControl);
};