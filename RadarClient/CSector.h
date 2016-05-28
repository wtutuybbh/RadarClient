#pragma once
#include "C3DObject.h"
class CViewPortControl;
class CSector :
	public C3DObjectModel
{
	static float maxAmp;
public:
	float PointSize {3};
	CSector();
	virtual ~CSector();
	virtual void Refresh(glm::vec4 origin, float mpph, float mppv, RPOINTS * info_p, RPOINT * pts, RDR_INITCL* init);
	void BindUniforms(CViewPortControl* vpControl) override;
	int GetPoint(CViewPortControl *vpControl, glm::vec2 screenPoint);
	void SelectPoint(int vpId, int pointIndex);
};

