#pragma once
#include "stdafx.h"
#include "C3DObjectModel.h"
struct RDR_INITCL;
struct RIMAGE;
class CRImage :
	public C3DObjectModel
{
	static float maxAmp;
	static float minAmp;
	static const std::string requestID;
	static FIBITMAP *palette;
	static int paletteWidth;
public:
	int d1{ 0 };
	int d2{ 0 };
	float Azemuth;
	CRImage(float azemuth, glm::vec4 origin, float mpph, float mppv, RDR_INITCL * rdrinit, RIMAGE* info, void* pixels);
	static bool InitPalette(std::string fileName);
	void Refresh(float azemuth, glm::vec4 origin, float mpph, float mppv, RDR_INITCL * rdrinit, RIMAGE* info, void* pixels);
};

