#pragma once
#include "CRCDataFile.h"

class CRCAltitudeDataFile :
	public CRCDataFile
{
	int size[9]; //for compatibility with GeoDataProvider.dll!gdpAltitudeMap() function:
	//size[0] - width (lon component) of block
	//size[1] - height (lat component) of block
	//size[2] - x0 (lon component) of block
	//size[3] - y_0 (lat component) of block
	//size[4] - x1 (lon component) of block
	//size[5] - y1 (lat component) of block
	//size[6] - width (lon component) of whole file
	//size[7] - height (lat component) of whole file
	//size[8] - data read methos (see GDP::DataReadMethod, GDP::CRCGeoDataProvider::GetAltitudeMap)
	void size_set_max();
	void size_set(int x0, int y_0, int x1, int y1);
	float *blind_zone_height {nullptr};
	float *ray_elevation_angle{ nullptr };
	float CalculateBlindZone_get_elevation(int x, int y) const;
	void CalculateBlindZone_helper(int x, int y, float xc, float yc, float dx, float dy, float hcr0);
	float _x {0}; 
	float _y{ 0 };
	float _e {0};
	static const std::string requestID;
public:
	CRCAltitudeDataFile(const std::string& dt2FileName);
	CRCAltitudeDataFile(double lon0, double lat0, double lon1, double lat1, int width, int height);
	~CRCAltitudeDataFile();
	
	void CalculateBlindZone(double h0, double e);
	

	short ValueAt(int x, int y);
	float BlindZoneHeightAt(int x, int y);
	float * BlindZoneHeight();
	short ValueAtLL(double lon, double lat);
	void SetValue(int x, int y, short val/*, float resX, float resY*/) const;

	void ApplyIntersection(CRCDataFile *src) override;

	bool Open() override;
	bool Close() override;
};

