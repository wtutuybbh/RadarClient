#include "stdafx.h"
#include "CRCTextureDataFile.h"

CRCTextureDataFile::CRCTextureDataFile(const std::string& imgFileName) :
	CRCDataFile(Texture)
{
	ydir = -1;

	fileName = imgFileName;

	std::ifstream infile;
	char line[DATFILE_MAXLINELENGTH];
	double v;
	char *pch;

	std::string datFileName = imgFileName.substr(0, imgFileName.length() - 3).append("dat");
	std::string dimFileName = imgFileName.substr(0, imgFileName.length() - 3).append("dim");

	//try to read dimension data from dimension file:
	try {
		infile.open(dimFileName, std::ifstream::in | std::ifstream::binary);
	}
	catch (...) {
		return;
	}
	if (!infile) {
		data = FreeImage_Load(FreeImage_GetFileType(imgFileName.c_str(), 0), imgFileName.c_str());
		if (data) {
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
			data = NULL;
		}
	} 
	else
	{
		//read dimension data from file:
		infile.getline(line, DATFILE_MAXLINELENGTH);
		if (strlen(line) > 0) {
			pch = strtok(line, ",");
			width = atoi(pch);			
			pch = strtok(NULL, ",");
			height = atof(pch);			
		}
		infile.close();
	}


	//read gps-coordinates data from dat file:
	try {
		infile.open(datFileName, std::ifstream::in | std::ifstream::binary);
	}
	catch (...) {
		return;
	}
	if (!infile) {
		return;
	}

	infile.getline(line, DATFILE_MAXLINELENGTH); //useless 1st line

	lon0 = 180, lat0 = 90, lon1 = -180, lat1 = -90;
	
	while (infile.good()) {
		infile.getline(line, DATFILE_MAXLINELENGTH);
		if (strlen(line) > 0 && line[0] != '(') {
			pch = strtok(line, ",");
			v = atof(pch);
			if (v < lon0)
				lon0 = v;
			if (v > lon1)
				lon1 = v;
			pch = strtok(NULL, ",");
			v = atof(pch);
			if (v < lat0)
				lat0 = v;
			if (v > lat1)
				lat1 = v;
		}

	}
	infile.close();
}

CRCTextureDataFile::CRCTextureDataFile(double lon0, double lat0, double lon1, double lat1, int width, int height) :
	CRCDataFile(Texture, lon0, lat0, lon1, lat1, width, height)
{
	ydir = -1;
	bytespp = 24;
	data = FreeImage_Allocate(width, height, bytespp);
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

void CRCTextureDataFile::ApplyIntersection(CRCDataFile& src)
{
	//assume *this as a target
	int this_x0, this_x1, this_y0, this_y1, src_x0, src_x1, src_y0, src_y1;

	Open();
	src.Open();

	GetIntersection(src, this_x0, this_y0, this_x1, this_y1);
	src.GetIntersection(*this, src_x0, src_y0, src_x1, src_y1);

	FIBITMAP *dib = (FIBITMAP *)data, *src_dib = (FIBITMAP *)src.GetData();

	if (dib && src_dib) {
		FIBITMAP *copied = FreeImage_Copy(src_dib, src_x0, src_y0, src_x1, src_y1);
		FIBITMAP *resized = FreeImage_Rescale(copied, this_x1 - this_x0 + 1, this_y1 - this_y0 + 1);

		FreeImage_Paste(dib, resized, this_x0, this_y0, 255);

		//FreeImage_Save(FIF_JPEG, dib, "newmap.jpg");

		FreeImage_Unload(copied);
		FreeImage_Unload(resized);
	}
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
