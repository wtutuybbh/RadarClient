#pragma once
#include "C3DObject.h"
struct RDR_INITCL;
struct RIMAGE;
class CRImage :
	public C3DObjectModel
{
	static float maxAmp;
public:
	float Azemuth;
	CRImage(float azemuth, glm::vec4 origin, float mpph, float mppv, RDR_INITCL * rdrinit, RIMAGE* info, void* pixels);
	~CRImage() override;

};

