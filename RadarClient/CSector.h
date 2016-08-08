#pragma once
#include "C3DObject.h"
struct RPOINTS;
struct RPOINT;
struct RDR_INITCL;

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
	virtual void Dump(CViewPortControl* vpControl, std::ofstream *outfile);
	void BindUniforms(CViewPortControl* vpControl) override;
	int GetPoint(CViewPortControl *vpControl, glm::vec2 screenPoint);
	glm::vec3 GetPointCoords(CViewPortControl *vpControl, int index);
	void SelectPoint(int vpId, int pointIndex);
	void UnselectAll(int vpId);
	glm::vec4 GetColor(float level);
};

