#pragma once
#include "C3DObjectModel.h"
struct RPOINTS;
struct RPOINT;
struct RDR_INITCL;

class CViewPortControl;

class CFlightPoint
{
public :
	CFlightPoint(double lon, double lat, int dir, int time);
	double lon;
	double lat;
	int dir;
	int time;
};

class CSector :
	public C3DObjectModel
{
	static float maxAmp;
	static float minAmp;
	float index {-1.0f};
	static FIBITMAP *palette;
	static int paletteWidth;
protected:
	static const std::string requestID;
	DWORD		start_tick_{ 0 };

	

	int lifetime{ 6 };
	float residual_alpha_{ 0.03f };
	
	static float x0, x1, x2, f0, f1, f2;

	
public:

	static DWORD		flight_start_;
	static std::vector<CFlightPoint> flight_points_;
	static int flight_time0;
	static void get_flight_point(DWORD t, double &lon, double &lat, double& dlon, double& dlat);
	static void flight_start_stop(bool start);
	static DWORD prev_image_tick_;


	static void RefreshColorSettings();
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
	int life_counter{ 0 };

};

