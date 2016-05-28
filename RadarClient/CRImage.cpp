//#include "stdafx.h"
#include "CRImage.h"


CRImage::CRImage() : C3DObjectModel(new C3DObjectVBO(false), NULL, new C3DObjectProgram("CRImage.v.glsl", "CRImage.f.glsl", "vertex", NULL, NULL, "color"))
{
}


CRImage::~CRImage()
{
}

void CRImage::Refresh(RIMAGE* info, void* pixels)
{
	float *px = (float *)pixels;
	for (int i = 0; i < info->N; i++)
	{
		
	}
}
