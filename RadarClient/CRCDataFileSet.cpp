#include "stdafx.h"
#include <FreeImage.h>
#include "CRCDataFile.h"
#include "CRCAltitudeDataFile.h"
#include "CRCTextureDataFile.h"
#include "CRCDataFileSet.h"
#include "Util.h"

using namespace boost::filesystem;
using namespace std;

#define DATFILE_MAXLINELENGTH 256



void CRCDataFileSet::AddTextureFile(std::string imgFile)
{
	
	auto file = new CRCTextureDataFile(imgFile);
	_files.push_back(file);
}

void CRCDataFileSet::AddAltitudeFile(std::string altFile)
{
	auto file = new CRCAltitudeDataFile(altFile);
	_files.push_back(file);

}

void CRCDataFileSet::AddFile(CRCDataFile* file)
{
	_files.push_back(file);
}

void CRCDataFileSet::AddFile(DataFileType type, std::string file)
{
	switch (type)
	{
	case Texture:
	{
		AddTextureFile(file);
	}
	break;
	case Altitude:
	{
		AddAltitudeFile(file);
	}
	break;
	}
}

void CRCDataFileSet::AddFiles(std::string dir, DataFileType typeFilter, std::string extFilter)
{
	boost::filesystem::path p(dir);

	if (is_directory(p)) {
		//std::cout << p << " is a directory containing:\n";

		for (auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(p), {})) {
			std::string filename = entry.path().filename().generic_string();
			std::string ext = filename.substr(filename.length() - 3, 3);
			DataFileType type = CRCDataFile::GetTypeByExt(ext);
			if ((extFilter == "" || ext == extFilter) && (type!=Undefined && typeFilter == type))
			{
				string file = entry.path().generic_string();
				AddFile(type, file);
			}
		}
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
