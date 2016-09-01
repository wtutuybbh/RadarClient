#include "stdafx.h"
#include <FreeImage.h>
#include "CRCDataFile.h"
#include "CRCDataFileSet.h"

using namespace boost::filesystem;
using namespace std;

#define DATFILE_MAXLINELENGTH 256

typedef int(_cdecl * GDPALTITUDEMAP_SIZES)(const char *, double *, int *);

CRCDataFileSet::CRCDataFileSet()
{
}


CRCDataFileSet::~CRCDataFileSet()
{
}

void CRCDataFileSet::AddTextureFile(std::string imgFile, std::string datFile)
{
	void *bitmap;
	try {
		bitmap = FreeImage_Load(FreeImage_GetFileType(imgFile.c_str(), 0), imgFile.c_str());
	}
	catch (...) {
		return;
	}
	//access to data: ((FIBITMAP *)bitmap)->data

	std::ifstream infile;

	try {
		infile.open(datFile, std::ifstream::in | std::ifstream::binary);
	}
	catch (...) {
		return;
	}

	if (!infile) {
		return;
	}
	char line[DATFILE_MAXLINELENGTH];
	infile.getline(line, DATFILE_MAXLINELENGTH); //useless 1st line

	double imgLon0 = 180, imgLat0 = 90, imgLon1 = -180, imgLat1 = -90, v;
	char *pch;
	while (infile.good()) {
		infile.getline(line, DATFILE_MAXLINELENGTH);
		if (strlen(line) > 0 && line[0] != '(') {
			pch = strtok(line, ",");
			v = atof(pch);
			if (v < imgLon0)
				imgLon0 = v;
			if (v > imgLon1)
				imgLon1 = v;
			pch = strtok(NULL, ",");
			v = atof(pch);
			if (v < imgLat0)
				imgLat0 = v;
			if (v > imgLat1)
				imgLat1 = v;
		}

	}
	infile.close();
	int width = FreeImage_GetWidth((FIBITMAP*)bitmap), height = FreeImage_GetHeight((FIBITMAP*)bitmap);
	FreeImage_Unload((FIBITMAP*)bitmap);
	auto file = new CRCDataFile(Texture, imgFile, datFile, imgLon0, imgLat0, imgLon1, imgLat1, width, height);
	_files.push_back(file);
}

void CRCDataFileSet::AddAltitudeFile(std::string altFile)
{
	HINSTANCE hDLL;               // Handle to DLL
	GDPALTITUDEMAP_SIZES gdpAltitudeMap_Sizes;    // Function pointer
	double LL[10];
	int size[8], result;

	hDLL = LoadLibrary(_T("GeoDataProvider.dll"));

	if (hDLL != NULL)
	{
		gdpAltitudeMap_Sizes = (GDPALTITUDEMAP_SIZES)GetProcAddress(hDLL, "gdpAltitudeMap_Sizes");
		if (!gdpAltitudeMap_Sizes)
		{
			FreeLibrary(hDLL);
			return;
		}
		LL[0] = 0;
		LL[1] = 0;
		LL[2] = 180;
		LL[3] = 180;
		result = gdpAltitudeMap_Sizes(altFile.c_str(), LL, size);
		if (result == 0) {
			auto file = new CRCDataFile(Altitude, altFile, altFile, LL[8], LL[9], LL[8] + LL[6] * (size[6] - 1), LL[9] + LL[7] * (size[7] - 1), size[6], size[7]);
			_files.push_back(file);
		}
		FreeLibrary(hDLL);
	}
	return;
}

void CRCDataFileSet::AddFile(CRCDataFile* file)
{
	_files.push_back(file);
}

void CRCDataFileSet::AddFile(DataFileType type, std::string imgFile, std::string datFile)
{
	switch (type)
	{
	case Texture:
	{
		AddTextureFile(imgFile, datFile);
	}
	break;
	case Altitude:
	{
		AddAltitudeFile(imgFile);
	}
	break;
	}
}

int CRCDataFileSet::CountFilesOfGivenType(DataFileType type)
{
	int size = 0;
	for (auto it = begin(_files); it != end(_files); ++it)
	{
		if (*it != NULL && (*it)->Type() == type)
			size++;
	}
	return size;
}

CRCDataFile* CRCDataFileSet::GetFile(int index)
{
	return _files.at(index);
}

CRCDataFileSet::~CRCDataFileSet()
{
	for (auto it = begin(_files); it != end(_files); ++it)
	{
		if (*it != NULL)
			delete *it;
	}
	_files.empty();
}
