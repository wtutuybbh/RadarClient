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
	static FIBITMAP *palette;
	static int paletteWidth;
public:
	float Azemuth;
	CRImage(float azemuth, glm::vec4 origin, float mpph, float mppv, RDR_INITCL * rdrinit, RIMAGE* info, void* pixels);
	~CRImage() override;
	static bool InitPalette(std::string fileName);
};

