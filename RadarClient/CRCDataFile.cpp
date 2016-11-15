#include "stdafx.h"
#include "CRCDataFile.h"
#include "Util.h"
#include "CRCLogger.h"

const std::string CRCDataFile::requestID = "CRCDataFile";

CRCDataFile::CRCDataFile(DataFileType type) : type(type)
{
	CRCLogger::Info(requestID, "CRCDataFile::CRCDataFile", (boost::format("Start... type=%1%... End. Ok.") % type).str());
}
CRCDataFile::CRCDataFile(DataFileType type, double lon0, double lat0, double lon1, double lat1, int width, int height) : lon0(lon0), lat0(lat0),
lon1(lon1),
lat1(lat1),
width(width),
height(height),
type(type)
{
	CRCLogger::Info(requestID, "CRCDataFile::CRCDataFile", (boost::format("Start... type=%1%, lon0=%2%, lat0=%3%, lon1=%4%, lat1=%5%, width=%6%, height=%7%... End. Ok.") 
		% type
		% lon0
		% lat0
		% lon1
		% lat1
		% width
		% height).str());
}
CRCDataFile::CRCDataFile(DataFileType type, const std::string& fileName, double lon0, double lat0, double lon1, double lat1, int width, int height)
	: fileName(fileName),
	lon0(lon0),
	lat0(lat0),
	lon1(lon1),
	lat1(lat1),
	width(width),
	height(height),
	type(type)
{
	CRCLogger::Info(requestID, "CRCDataFile::CRCDataFile", (boost::format("Start... type=%1%, fileName=%2%, lon0=%3%, lat0=%4%, lon1=%5%, lat1=%6%, width=%7%, height=%8%... End. Ok.")
		% type
		% fileName
		% lon0
		% lat0
		% lon1
		% lat1
		% width
		% height).str());
}

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
	CRCLogger::Info(requestID, "CRCDataFile DESTRUCTOR", (boost::format("Destroying CRCDataFile object, fileName=%1%, data=%2%, type=%3%") % fileName % data % type).str());
}

DataFileType CRCDataFile::Type() const
{
	return type;
}

std::string CRCDataFile::TypeName() const
{
	return GetDataFileTypeName(type);
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

	x0 = floor(zero_if_negative(src.lon0 - this->lon0) / lon_res);	
	x1 = ceil((min(this->lon1, src.lon1) - this->lon0) / lon_res);
	if (x1 >= width)
		x1--;

	y0 = floor(zero_if_negative(src.lat0 - this->lat0) / lat_res);
	y1 = ceil((min(this->lat1, src.lat1) - this->lat0) / lat_res);
	if (y1 >= height)
		y1--;


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

void* CRCDataFile::Data() const
{
	return data;
}

int CRCDataFile::Width() const
{
	return width;
}

int CRCDataFile::Height() const
{
	return height;
}

void CRCDataFile::SetName(std::string fileName)
{
	this->fileName = fileName;
}

std::string CRCDataFile::GetName() const
{
	return fileName;
}

std::string CRCDataFile::GetDataFileTypeName(DataFileType t)
{
	if (t == Altitude)
	{
		return "Altitude";
	}
	if (t == Texture)
	{
		return "Texture";
	}
	if (t == Undefined)
	{
		return "Undefined";
	}
	return std::string();
}
