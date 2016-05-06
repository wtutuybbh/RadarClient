#pragma once
#include "C3DObject.h"
#include <vector>
class CTrack :
	public C3DObject
{
public:
	vector<C3DObject*> Points;
	CTrack();
	~CTrack();
};

