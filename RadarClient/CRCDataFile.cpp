//#include "stdafx.h"
#include "CRCDataFile.h"


CRCDataFile::~CRCDataFile()
{
}

DataFileType CRCDataFile::Type()
{
	return type;
}

void CRCDataFile::ApplyIntersection(CRCDataFile& src)
{
	//TODO:
}

float CRCDataFile::ValueAt(int x, int y)
{
	if (type==Altitude && data)
		return ((float *)data)[y*width + x];
	if (type == Texture && data) {
		// TODO:
	}
	return 0;
}

glm::vec4 CRCDataFile::ColorAt(int x, int y)
{
	if (type == Texture && data) {
		// TODO:
	}
	//TODO:
	return glm::vec4(0);
}

void CRCDataFile::FitSize(float resX, float resY) {
	//TODO:
}

void CRCDataFile::SetValue(int x, int y, float val, float resX, float resY)
{
	if (type == Altitude && data && resolutionX && resolutionY)
	{
		if (resX < resolutionX[y*width + x] && resY < resolutionY[y*width + x])
		{
			resolutionX[y*width + x] = resX;
			resolutionY[y*width + x] = resY;
			((float *)data)[y*width + x] = val;
		}
	}
}