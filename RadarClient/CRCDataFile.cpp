#include "stdafx.h"
#include "CRCDataFile.h"
#include "Util.h"
#include "CRCLogger.h"

const std::string CRCDataFile::requestID = "CRCDataFile";

CRCDataFile::CRCDataFile(DataFileType type) : type(type)
{
	LOG_INFO(requestID, "CRCDataFile::CRCDataFile", (boost::format("Start... type=%1%... End. Ok.") % type).str().c_str());
}
CRCDataFile::CRCDataFile(DataFileType type, double lon0, double lat0, double lon1, double lat1, int width, int height) : lon0(lon0), lat0(lat0),
lon1(lon1),
lat1(lat1),
width(width),
height(height),
type(type)
{
	LOG_INFO(requestID, "CRCDataFile::CRCDataFile", (boost::format("Start... type=%1%, lon0=%2%, lat0=%3%, lon1=%4%, lat1=%5%, width=%6%, height=%7%... End. Ok.") 
		% type
		% lon0
		% lat0
		% lon1
		% lat1
		% width
		% height).str().c_str());
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
	LOG_INFO(requestID, "CRCDataFile::CRCDataFile", (boost::format("Start... type=%1%, fileName=%2%, lon0=%3%, lat0=%4%, lon1=%5%, lat1=%6%, width=%7%, height=%8%... End. Ok.")
		% type
		% fileName
		% lon0
		% lat0
		% lon1
		% lat1
		% width
		% height).str().c_str());
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
	LOG_INFO(requestID, "CRCDataFile DESTRUCTOR", (boost::format("Destroying CRCDataFile object, fileName=%1%, data=%2%, type=%3%") % fileName % data % type).str().c_str());
}

DataFileType CRCDataFile::Type() const
{
	return type;
}

std::string CRCDataFile::TypeName() const
{
	return GetDataFileTypeName(type);
}

bool CRCDataFile::GetIntersection(CRCDataFile *src, int& x0, int& y_0, int& x1, int& y1) const
{
	if (!src)
	{
		LOG_ERROR_("CRCDataFile::GetIntersection", "src is nullptr");
		return false;
	}
	//x0, y_0, x1, y1 - coordinates of *this.
	//if you need coordinates of src, just revert the call

	if (this->lon1 < src->lon0 || this->lon0 > src->lon1 || this->lat1 < src->lat0 || this->lat0 > src->lat1)
		return false;

	double lon_res = (this->lon1 - this->lon0) / (width - 1), lat_res = (this->lat1 - this->lat0) / (height - 1);

	x0 = floor(zero_if_negative(src->lon0 - this->lon0) / lon_res);
	x1 = ceil((min(this->lon1, src->lon1) - this->lon0) / lon_res);
	if (x1 >= width)
		x1--;

	y_0 = floor(zero_if_negative(src->lat0 - this->lat0) / lat_res);
	y1 = ceil((min(this->lat1, src->lat1) - this->lat0) / lat_res);
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

double CRCDataFile::DLon() const
{
	if (width > 1) 
	{
		return (lon1 - lon0) / (width-1);
	}
	LOG_INFO_("CRCDataFile::DLon", "(fileName=%s) error: width<=1", fileName);
	return 0;
}

double CRCDataFile::DLat() const
{
	if (height > 1)
	{
		return (lat1 - lat0) / (height-1);
	}
	LOG_INFO_("CRCDataFile::DLat", "(fileName=%s) error: height<=1", fileName);
	return 0;
}

double CRCDataFile::Lon0() const
{
	return lon0;
}

double CRCDataFile::Lat0() const
{
	return lat0;
}

double CRCDataFile::Lon1() const
{
	return lon1;
}

double CRCDataFile::Lat1() const
{
	return lat1;
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

void CRCDataFile::SetLonLat(double lon0, double lat0, double lon1, double lat1)
{
	this->lon0 = lon0;
	this->lat0 = lat0;
	this->lon1 = lon1;
	this->lat1 = lat1;
}
