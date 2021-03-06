#pragma once
#include "stdafx.h"
#include "CRCDataFile.h"


class CRCTextureDataFile :
	public CRCDataFile
{
	int bytespp { 24 };
	int alpha{ 255 }; // parameter for Paste operation
public:
	CRCTextureDataFile(const std::string& imgFileName);
	CRCTextureDataFile(double lon0, double lat0, double lon1, double lat1, int width, int height);
	~CRCTextureDataFile();

	glm::vec4 ColorAt(int x, int y);
	glm::vec4 ColorAt(float lon, float lat);

	void ApplyIntersection(CRCDataFile *src) override;

	bool Open() override;
	bool Close() override;

	bool Save();
};

