#pragma once
#include <string>
#include "glm/glm.hpp"

enum DataFileType
{
	Texture,
	Altitude
};
class CRCDataFile
{
	std::string _imgFileName, _datFileName;
	double lon0, lat0, lon1, lat1;
	int width, height;
	DataFileType type;
	void *data{ NULL };
	float *resolutionX{ NULL }, *resolutionY{ NULL };
public:
	CRCDataFile(DataFileType type, const std::string& imgFileName, const std::string& datFileName, double lon0, double lat0, double lon1, double lat1, int width, int height)
		: _imgFileName(imgFileName),
		_datFileName(datFileName),
		lon0(lon0),
		lat0(lat0),
		lon1(lon1),
		lat1(lat1),
		width(width),
		height(height),
		type(type)
	{
	}
	DataFileType Type();
	void ApplyIntersection(CRCDataFile &src);
	float ValueAt(int x, int y);
	glm::vec4 ColorAt(int x, int y);
	void SetValue(int x, int y, float val, float resX, float resY);
	void FitSize(float resX, float resY); // set height, width and new 
};

