#pragma once
#include <vector>
#include <GL/glew.h>
#include "C3DObject.h"
class CViewPortControl;
class CRImage;
class CRImageSet
{
public:
	std::vector<CRImage*> *Images;
	float prevAzimuth {0.0f}; //azimuth from previous scan
	CRImageSet();
	~CRImageSet();
	void Refresh(glm::vec4 origin, float mpph, float mppv, RDR_INITCL * rdrinit, RIMAGE* info, void* pixels);
	void Draw(CViewPortControl *vpControl, GLenum mode);
};

