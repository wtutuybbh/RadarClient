#pragma once
#include "CRCDataFile.h"
class CRCAltitudeDataFile :
	public CRCDataFile
{
public:
	CRCAltitudeDataFile(const std::string& dt2FileName);
	CRCAltitudeDataFile(double lon0, double lat0, double lon1, double lat1, int width, int height);
	~CRCAltitudeDataFile();

	short ValueAt(int x, int y);
	short ValueAt(double lon, double lat);
	void SetValue(int x, int y, short val, float resX, float resY);

	void ApplyIntersection(CRCDataFile &src) override;

	bool Open() override;
};

