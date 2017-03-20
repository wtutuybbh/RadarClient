#pragma once
#include "C3DObjectModel.h"
struct RPOINTS;
struct RPOINT;
struct RDR_INITCL;

class CViewPortControl;

class CSector :
	public C3DObjectModel
{
	std::mutex m;
	static float maxAmp;
	static float minAmp;
	float index {-1.0f};
	static FIBITMAP *palette;
	static int paletteWidth;
protected:
	static const std::string requestID;
public:
	float PointSize {3};
	explicit CSector(int index);
	virtual void Refresh(glm::vec4 origin, float mpph, float mppv, RPOINTS * info_p, RPOINT * pts, RDR_INITCL* init);
	virtual void Dump(CViewPortControl* vpControl, std::ofstream *outfile);
	void BindUniforms(CViewPortControl* vpControl) override;
	int GetPoint(CViewPortControl *vpControl, glm::vec2 screenPoint);
	glm::vec3 GetPointCoords(CViewPortControl *vpControl, int index);
	void SelectPoint(int vpId, int pointIndex);
	void UnselectAll(int vpId);
	static glm::vec4 GetColor(float level);
	static bool InitPalette(std::string fileName);
};

