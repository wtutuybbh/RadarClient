#pragma once
#include "C3DObjectModel.h"

class CViewPortControl;
class CRImage;
struct RDR_INITCL;
struct RIMAGE;
class CRImageSet: public C3DObjectModel
{
public:
	std::vector<CRImage*> *Images{ nullptr };
	float prevAzimuth {0.0f}; //azimuth from previous scan
	CRImageSet();
	~CRImageSet();
	void Refresh(glm::vec4 origin, float mpph, float mppv, RDR_INITCL * rdrinit, RIMAGE* info, void* pixels);
	void Draw(CViewPortControl *vpControl, GLenum mode) override;
	void SetAlphaBehaviour(AlphaBehaviour ab) override;
	void SetRotateMatrix(glm::mat4 m);
};

