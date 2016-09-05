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
