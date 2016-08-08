// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <windows.h>

using namespace boost::filesystem;
using namespace std;

#define DATFILE_MAXLINELENGTH 256

typedef int(_cdecl * GDPALTITUDEMAP_SIZES)(const char *, double *, int *);

enum DataFileType
{
	Texture,
	Altitude
};
class RCDataFile
{	
	std::string _imgFileName, _datFileName;
	double lon0, lat0, lon1, lat1;
	int width, height;
	DataFileType type;
public:
	RCDataFile(DataFileType type, const std::string& imgFileName, const std::string& datFileName, double lon0, double lat0, double lon1, double lat1, int width, int height)
		: type(type),
		_imgFileName(imgFileName),
		_datFileName(datFileName),
		lon0(lon0),
		lat0(lat0),
		lon1(lon1),
		lat1(lat1),
		width(width),
		height(height)
	{
	}
	DataFileType Type();
};
class RCDataFileSet
{
	std::vector<RCDataFile *> _files;
	void AddTextureFile(std::string imgFile, std::string datFile);
	void AddAltitudeFile(std::string altFile);
public:
	void AddFile(RCDataFile *file);
	void AddFile(DataFileType type, std::string imgFile, std::string datFile);
	int CountFilesOfGivenType(DataFileType type);
	RCDataFile *GetFile(int index);
	~RCDataFileSet();
};

DataFileType RCDataFile::Type()
{
	return type;
}

void RCDataFileSet::AddTextureFile(std::string imgFile, std::string datFile)
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
	auto file = new RCDataFile(Texture, imgFile, datFile, imgLon0, imgLat0, imgLon1, imgLat1, width, height);
	_files.push_back(file);
}

void RCDataFileSet::AddAltitudeFile(std::string altFile)
{
	/*HINSTANCE hDLL;               // Handle to DLL
	GDPALTITUDEMAP_SIZES gdpAltitudeMap_Sizes;    // Function pointer
	double LL[10];
	int size[8], result;

	hDLL = LoadLibrary("GeoDataProvider.dll");*/
}

void RCDataFileSet::AddFile(RCDataFile* file)
{
	_files.push_back(file);
}

void RCDataFileSet::AddFile(DataFileType type, std::string imgFile, std::string datFile)
{
	switch(type)
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

int RCDataFileSet::CountFilesOfGivenType(DataFileType type)
{
	int size = 0;
	for (auto it = begin(_files); it != end(_files); ++it)
	{
		if (*it != NULL && (*it)->Type() == type)
			size++;
	}
	return size;
}

RCDataFile* RCDataFileSet::GetFile(int index)
{
	return _files.at(index);
}

RCDataFileSet::~RCDataFileSet()
{
	for (auto it = begin(_files); it != end(_files); ++it)
	{
		if (*it != NULL)
			delete *it;
	}
	_files.empty();
}

int main(int argc, char *argv[]) {

	RCDataFileSet DataFileSet;

	path p(argc>1 ? argv[1] : ".");

	if (is_directory(p)) {
		std::cout << p << " is a directory containing:\n";

		for (auto& entry : boost::make_iterator_range(directory_iterator(p), {})) {			
			std::string filename = entry.path().filename().generic_string();
			if (filename.substr(filename.length()-4) == ".jpg")
			{
				string imgFile = entry.path().generic_string();
				string datFile = imgFile.substr(0, imgFile.length() - 3) + "dat";
				DataFileSet.AddFile(Texture, imgFile, datFile);
			}
			if (filename.substr(filename.length() - 4) == ".dt2")
			{
				string altFile = entry.path().generic_string();
				DataFileSet.AddFile(Altitude, altFile, string(NULL));
			}
			//entry.replace_filename()
		}
	}
	cout << DataFileSet.CountFilesOfGivenType(Texture);
}
