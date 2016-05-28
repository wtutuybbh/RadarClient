#pragma once
#include "C3DObject.h"
class CRImage :
	public C3DObjectModel
{
public:
	CRImage();
	virtual ~CRImage();
	void Refresh(RIMAGE* info, void* pixels);
};

