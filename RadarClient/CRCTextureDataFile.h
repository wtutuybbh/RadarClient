#pragma once
#include "CRCDataFile.h"

#define DATFILE_MAXLINELENGTH 256

class CRCTextureDataFile :
	public CRCDataFile
{
	int bytespp;
public:
	CRCTextureDataFile(const std::string& imgFileName);
	CRCTextureDataFile(double lon0, double lat0, double lon1, double lat1, int width, int height);
	~CRCTextureDataFile();

	glm::vec4 ColorAt(int x, int y);
	glm::vec4 ColorAt(float lon, float lat);

	void ApplyIntersection(CRCDataFile &src) override;

	bool Open() override;

	bool Save();
};

