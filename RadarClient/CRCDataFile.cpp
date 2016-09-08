//#include "stdafx.h"
#include "CRCDataFile.h"
#include "Util.h"
#include <math.h>


DataFileType CRCDataFile::GetTypeByExt(std::string ext)
{
	if (ext == "dt2" || ext == "bil")
		return Altitude;
	if (ext == "jpg" || ext == "png" || ext == "gif" || ext == "bmp")
		return Texture;
	return Undefined;
}

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

bool CRCDataFile::GetIntersection(CRCDataFile& src, int& x0, int& y0, int& x1, int& y1) const
{
	//x0, y0, x1, y1 - coordinates of *this.
	//if you need coordinates of src, just revert the call

	if (this->lon1 < src.lon0 || this->lon0 > src.lon1 || this->lat1 < src.lat0 || this->lat0 > src.lat1)
		return false;

	double lon_res = (this->lon1 - this->lon0) / (width - 1), lat_res = (this->lat1 - this->lat0) / (height - 1);

	x0 = ceil(zero_if_negative(src.lon0 - this->lon0) / lon_res);	
	x1 = floor((this->lon1 - this->lon0 - zero_if_negative(this->lon1 - src.lon1)) / lon_res);

	if (xdir == -1) {
		x0 = width - x0 - 1;
		x1 = width - x1 - 1;
	}
	y0 = ceil(zero_if_negative(src.lat0 - this->lat0) / lat_res);
	y1 = floor((this->lat1 - this->lat0 - zero_if_negative(this->lat1 - src.lat1)) / lat_res);
	if (ydir == -1)
	{
		y0 = height - y0 - 1;
		y1 = height - y1 - 1;
	}

	return true;
}

void CRCDataFile::FitSize(float resX, float resY) {
	//TODO:
	if (lon0 != lon1) {
		width = floor((lon1 - lon0) / resX);
		lon1 = lon0 + resX * width;
	}
	if (lat0 != lat1) {
		height = floor((lat1 - lat0) / resY);
		lat1 = lat0 + resY * height;
	}
}

void* CRCDataFile::GetData() const
{
	return data;
}

void CRCDataFile::SetName(std::string fileName)
{
	this->fileName = fileName;
}

std::string CRCDataFile::GetName() const
{
	return fileName;
}