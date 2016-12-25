#include "stdafx.h"
#include "CRCTextureDataFile.h"
#include "Util.h"
#include "CRCLogger.h"

CRCTextureDataFile::CRCTextureDataFile(const std::string& imgFileName) :
	CRCDataFile(Texture)
{
	std::string context = "CRCTextureDataFile::CRCTextureDataFile";
	if (imgFileName.empty())
	{
		LOG_ERROR(requestID, context, "imgFileName is empty. Will throw exception().");
		throw std::exception("imgFileName is empty");
	}
	LOG_INFO(requestID, context, (boost::format("Start... imgFileName=%1%")
		% imgFileName).str().c_str());

	fileName = imgFileName;

	std::ifstream infile;
	char line[DATFILE_MAXLINELENGTH];
	double v;
	char *pch;

	std::string datFileName = imgFileName.substr(0, imgFileName.length() - 3).append("dat");
	std::string dimFileName = imgFileName.substr(0, imgFileName.length() - 3).append("dim");

	LOG_INFO(requestID, context, (boost::format("%1% => datFileName=%2%, dimFileName=%3%") % fileName % datFileName % dimFileName).str().c_str());

	//try to read dimension data from dimension file:
	try 
	{
		infile.open(dimFileName, std::ifstream::in | std::ifstream::binary);
	}
	catch (...) 
	{		
		std::string error_string = (boost::format("infile.open() throwed exeption, dimFileName=%1%") % dimFileName).str();
		LOG_ERROR(requestID, context, error_string.c_str());
		throw std::exception(error_string.c_str());
	}
	if (!infile) {
		LOG_WARN(requestID, context, "No *.dim file, will create new");
		data = FreeImage_Load(FreeImage_GetFileType(imgFileName.c_str(), 0), imgFileName.c_str());
		if (!data) 
		{
			std::string error_string = (boost::format("FreeImage_Load failed. imgFileName=%1%") % imgFileName).str();
			LOG_ERROR(requestID, context, (error_string + ". Will throw exception().").c_str());
			throw std::exception(error_string.c_str());
		}
		width = FreeImage_GetWidth((FIBITMAP*)data);
		height = FreeImage_GetHeight((FIBITMAP*)data);
			
		//write dimension data to dimension file:
		std::ofstream outfile;

		outfile.open(dimFileName, std::ofstream::out | std::ofstream::binary);

		std::string str = std::to_string(width);
			
		outfile.write(str.c_str(), str.size());
		outfile.write(",", 1);
		str= std::to_string(height);
		outfile.write(str.c_str(), str.size());

		outfile.close();

		FreeImage_Unload((FIBITMAP*)data);
		data = nullptr;
		
		LOG_INFO(requestID, context, (boost::format("*.dim file %1% created, width=%2%, height=%3%") % dimFileName % width % height).str().c_str());
	} 
	else
	{
		//read dimension data from file:
		infile.getline(line, DATFILE_MAXLINELENGTH);
		if (strlen(line) > 0) {
			pch = strtok(line, ",");
			width = atoi(pch);			
			pch = strtok(nullptr, ",");
			height = atof(pch);			
		}
		infile.close();
		LOG_INFO(requestID, context, (boost::format("*.dim file %1% processed, width=%2%, height=%3%") % dimFileName % width % height).str().c_str());
	}


	//read gps-coordinates data from dat file:
	try 
	{
		infile.open(datFileName, std::ifstream::in | std::ifstream::binary);
	}
	catch (...) 
	{
		std::string error_string = (boost::format("infile.open() throwed exeption, datFileName=%1%") % datFileName).str();
		LOG_ERROR(requestID, context, error_string.c_str());
		throw std::exception(error_string.c_str());
	}
	if (!infile) 
	{
		std::string error_string = (boost::format("infile is nullptr, datFileName=%1%") % datFileName).str();
		LOG_ERROR(requestID, context, error_string.c_str());
		throw std::exception(error_string.c_str());
	}

	infile.getline(line, DATFILE_MAXLINELENGTH); //useless 1st line, generated by SASPlaneta

	lon0 = 180, lat0 = 90, lon1 = -180, lat1 = -90; //initial values
	
	while (infile.good()) {
		infile.getline(line, DATFILE_MAXLINELENGTH);
		if (strlen(line) > 0 && line[0] != '(') {
			pch = strtok(line, ",");
			v = atof(pch);
			if (v < lon0)
				lon0 = v;
			if (v > lon1)
				lon1 = v;
			pch = strtok(nullptr, ",");
			v = atof(pch);
			if (v < lat0)
				lat0 = v;
			if (v > lat1)
				lat1 = v;
		}
	}
	LOG_INFO(requestID, context, (boost::format("Object created from file %1%, (lon0=%2%, lat0=%3%), (lon1=%4%, lat1=%5%), (width=%6%, height=%7%)") 
		% fileName % lon0 % lat0 % lon1 % lat1 % width % height).str().c_str());
	infile.close();
}

CRCTextureDataFile::CRCTextureDataFile(double lon0, double lat0, double lon1, double lat1, int width, int height) :
	CRCDataFile(Texture, lon0, lat0, lon1, lat1, width, height)
{
	std::string context = "CRCTextureDataFile::CRCTextureDataFile";
	LOG_INFO(requestID, context, (boost::format("Start... lon0=%1%, lat0=%2%, lon1=%3%, lat1=%4%, width=%5%, height=%6%")
		% lon0 % lat0 % lon1 % lat1 % width % height).str().c_str());

	data = FreeImage_Allocate(width, height, bytespp);
	if (!data)
	{
		std::string error_string = (boost::format("FreeImage_Allocate failed. Size was ( (w*h) = (%1% * %2%) = %3% ). %4% bytes per pixel.") % width % height % (width * height) % bytespp).str();
		LOG_ERROR(requestID, context, (error_string + ". RETURN FALSE.").c_str());
		throw std::exception(error_string.c_str());
	}
}

CRCTextureDataFile::~CRCTextureDataFile()
{
	if (data)
		FreeImage_Unload((FIBITMAP *)data);
}


glm::vec4 CRCTextureDataFile::ColorAt(int x, int y)
{
	FIBITMAP *dib = (FIBITMAP *)data;
	if (dib && x<FreeImage_GetWidth(dib) && y<FreeImage_GetHeight(dib)) {
		
		// Calculate the number of bytes per pixel (3 for 24-bit or 4 for 32-bit)
		
		BYTE *bits = FreeImage_GetScanLine(dib, y);
		return glm::vec4(bits[x*bytespp + FI_RGBA_RED] / 255.0, bits[x*bytespp + FI_RGBA_GREEN] / 255.0, bits[x*bytespp + FI_RGBA_BLUE] / 255.0, bits[x*bytespp + FI_RGBA_ALPHA] / 255.0);
	}
	//TODO:
	return glm::vec4(0);
}

glm::vec4 CRCTextureDataFile::ColorAt(float lon, float lat)
{
	if (data && lon >= lon0 && lon <= lon1 && lat >= lat0 && lat <= lat1)
	{
		float xf = width * (lon - lon0) / (lon1 - lon0);
		float yf = height * (lat - lat0) / (lat1 - lat0);

		int x0 = floor(xf), x1 = ceil(xf);
		int y0 = floor(yf), y1 = ceil(yf);
	}
	return glm::vec4(0);
}

void CRCTextureDataFile::ApplyIntersection(CRCDataFile *src)
{
	std::string context = "CRCTextureDataFile::ApplyIntersection";

	if (!src)
	{
		LOG_ERROR__("src is nullptr");
		return;
	}

	LOG_INFO(requestID, context, (boost::format("Start... this->filename=%1%, src.Filename=%2%") % fileName % src->GetName()).str().c_str());

	if (!src->Open())
	{
		LOG_ERROR(requestID, context, "src.Open() failed. RETURN.");
		return;
	}
	
	FIBITMAP *dib = (FIBITMAP *)data;
	if (!dib)
	{
		LOG_ERROR(requestID, context, "this->data not initialized. Call Open() first. RETURN.");
		return;
	}

	FIBITMAP *src_dib = (FIBITMAP *)src->Data();
	if (!src_dib)
	{
		LOG_ERROR(requestID, context, "src.data not initialized. RETURN.");
		return;
	}

	//assume *this as a target
	int this_x0, this_x1, this_y0, this_y1, src_x0, src_x1, src_y0, src_y1;

	if (!GetIntersection(src, this_x0, this_y0, this_x1, this_y1) || !src->GetIntersection(this, src_x0, src_y0, src_x1, src_y1))
	{
		LOG_WARN(requestID, context, (boost::format("Intersection not found. this_x0=%1%, this_y0=%2%, this_x1=%3%, this_y1=%4%.")
			% this_x0 % this_y0 % this_x1 % this_y1).str().c_str());
		return;
	}
	LOG_INFO(requestID, context, (boost::format("Intersection found. this_x0=%1%, this_y0=%2%, this_x1=%3%, this_y1=%4%,    src_x0=%5%, src_y0=%6%, src_x1=%7%, src_y1=%8%.")
		% this_x0 % this_y0 % this_x1 % this_y1 % src_x0 % src_y0 % src_x1 % src_y1).str().c_str());

	//magic swap (because image origin is top-left):
	int tmp = this_y0;
	this_y0 = height - this_y1 - 1;
	this_y1 = height - tmp - 1;
	tmp = src_y0;
	src_y0 = src->Height() - src_y1 - 1;
	src_y1 = src->Height() - tmp - 1;

	LOG_INFO(requestID, context, (boost::format("After 'magic swap'. this_x0=%1%, this_y0=%2%, this_x1=%3%, this_y1=%4%,    src_x0=%5%, src_y0=%6%, src_x1=%7%, src_y1=%8%.")
		% this_x0 % this_y0 % this_x1 % this_y1 % src_x0 % src_y0 % src_x1 % src_y1).str().c_str());

	FIBITMAP *copied = FreeImage_Copy(src_dib, src_x0, src_y0, src_x1, src_y1);
	if (!copied)
	{
		LOG_ERROR(requestID, context, "FreeImage_Copy failed. RETURN.");
		return;
	}
	FIBITMAP *resized = FreeImage_Rescale(copied, this_x1 - this_x0 + 1, this_y1 - this_y0 + 1);
	if (!resized)
	{
		LOG_ERROR(requestID, context, "FreeImage_Rescale failed. RETURN.");
		FreeImage_Unload(copied);
		return;
	}
	FreeImage_Unload(copied);
	if (!FreeImage_Paste(dib, resized, this_x0, this_y0, alpha))
	{
		LOG_ERROR(requestID, context, "FreeImage_Paste failed. RETURN.");
		FreeImage_Unload(resized);
		return;
	}	
	FreeImage_Unload(resized);

	LOG_INFO(requestID, context, "End. Success.");
}

bool CRCTextureDataFile::Open()
{
	if (!data) {
		data = FreeImage_Load(FreeImage_GetFileType(fileName.c_str(), 0), fileName.c_str());

		//correct width and height and bytespp:
		if (data) {
			width = FreeImage_GetWidth((FIBITMAP*)data);
			height = FreeImage_GetHeight((FIBITMAP*)data);
			bytespp = FreeImage_GetLine((FIBITMAP*)data) / FreeImage_GetWidth((FIBITMAP*)data);
			return true;
		}
	}
	return false;
}

bool CRCTextureDataFile::Close()
{
	if (data) {
		FreeImage_Unload((FIBITMAP *)data);
		return true;
	}
}

bool CRCTextureDataFile::Save()
{
	if (data) {
		try {
			FreeImage_Save(FIF_JPEG, (FIBITMAP *)data, fileName.c_str());
			return true;
		}
		catch (...) {
			return false;
		}
	}
	return false;
}
