#pragma once
#include "C3DObject.h"
#include "Util.h"
#include <vector>

class CMinimapPointer : C3DObject
{/*
private:
	VBOBuffer*/
public:

	void Draw();
	bool PrepareAndBuildMinimapVBO();

	CMinimapPointer();
	~CMinimapPointer();
};

