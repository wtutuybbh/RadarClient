// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

//

using namespace boost::filesystem;
using namespace std;

#define DATFILE_MAXLINELENGTH 256

typedef int(_cdecl * GDPALTITUDEMAP_SIZES)(const char *, double *, int *);

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
	float *data{ NULL };
	float *resolution{ NULL };
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
	void SetValue(int x, int y, float val, float res);
};
class CRCDataFileSet
{
	std::vector<CRCDataFile *> _files;
	void AddTextureFile(std::string imgFile, std::string datFile);
	void AddAltitudeFile(std::string altFile);
public:
	void AddFile(CRCDataFile *file);
	void AddFile(DataFileType type, std::string imgFile, std::string datFile);
	int CountFilesOfGivenType(DataFileType type);
	CRCDataFile *GetFile(int index);
	~CRCDataFileSet();
};

DataFileType CRCDataFile::Type()
{
	return type;
}

void CRCDataFile::ApplyIntersection(CRCDataFile& src)
{

}

float CRCDataFile::ValueAt(int x, int y)
{
	if (data)
		return data[y*width + x];
	return 0;
}

void CRCDataFile::SetValue(int x, int y, float val, float res)
{
	if (data && resolution)
	{
		if (res < resolution[y*width + x])
		{
			resolution[y*width + x] = res;
			data[y*width + x] = val;
		}
	}
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
			auto file = new CRCDataFile(Altitude, altFile, altFile, LL[8], LL[9], LL[8] + LL[6] * (size[6]-1), LL[9] + LL[7] * (size[7] - 1), size[6], size[7]);
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

int main(int argc, char *argv[]) {

	CRCDataFileSet DataFileSet;

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
				DataFileSet.AddFile(Altitude, altFile, "");
			}
			//entry.replace_filename()
		}
	}
	cout << DataFileSet.CountFilesOfGivenType(Texture);

	try {
		string img_64x64("c:\\Users\\RazumovSa\\Documents\\Visual Studio 2015\\Projects\\RadarClient\\ConsoleApplication1\\Debug\\img_64x64.jpg");
		string img_new("c:\\Users\\RazumovSa\\Documents\\Visual Studio 2015\\Projects\\RadarClient\\ConsoleApplication1\\Debug\\img_new.jpg");

		FIBITMAP *bitmap_64x64 = FreeImage_Load(FreeImage_GetFileType(img_64x64.c_str(), 0), img_64x64.c_str());

		FIBITMAP *bitmap_new = FreeImage_Allocate(128, 128, 24);

		FIBITMAP *copied = FreeImage_Copy(bitmap_64x64, 0, 0, 63, 63);

		FIBITMAP *resized = FreeImage_Rescale(copied, 128, 128);

		FreeImage_Paste(bitmap_new, resized, 0, 0, 256);

		FreeImage_Save(FIF_JPEG, bitmap_new, img_new.c_str());

		FreeImage_Unload(bitmap_64x64);
		FreeImage_Unload(copied);
		FreeImage_Unload(resized);
		FreeImage_Unload(bitmap_new);

	}
	catch (...) {
		return 0;
	}
}
