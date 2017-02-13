#include "stdafx.h"
#include "CRCDataFileSet.h"
#include "CRCTextureDataFile.h"
#include "CRCAltitudeDataFile.h"
#include "CRCLogger.h"
#include "Util.h"

using namespace boost::filesystem;
using namespace std;

#define DATFILE_MAXLINELENGTH 256

const std::string CRCDataFileSet::requestID = "CRCDataFileSet";

void CRCDataFileSet::AddTextureFile(std::string imgFile)
{
	std::string context = "CRCDataFileSet::AddTextureFile";
	LOG_INFO(requestID, context, (boost::format("Start... imgFile=%1%") % imgFile).str().c_str());

	CRCTextureDataFile * file;
	try 
	{
		file = new CRCTextureDataFile(imgFile);
	}
	catch (const std::exception &ex) {
		LOG_WARN("exception", "CRCDataFileSet::AddTextureFile", ex.what());
		return;
	}
	if (!file)
	{
		LOG_ERROR(requestID, context, "file is nullptr for some reason.");
		return;
	}
	_files.push_back(file);
	LOG_INFO(requestID, context, (boost::format("End. Ok. _files collection now contains %1% elements.") % _files.size()).str().c_str());
}

void CRCDataFileSet::AddAltitudeFile(std::string altFile)
{
	std::string context = "CRCDataFileSet::AddAltitudeFile";
	LOG_INFO(requestID, context, (boost::format("Start... altFile=%1%") % altFile).str().c_str());

	CRCAltitudeDataFile * file;
	try
	{
		file = new CRCAltitudeDataFile(altFile);
	}
	catch (const std::exception& ex)
	{
		LOG_ERROR(requestID, context, ("CRCAltitudeDataFile constructor throwed exception: " + std::string(ex.what())).c_str());
		return;
	}
	if (!file)
	{
		LOG_ERROR(requestID, context, "file is nullptr for some reason.");
		return;
	}
	_files.push_back(file);
	LOG_INFO(requestID, context, (boost::format("End. Ok. _files collection now contains %1% elements.") % _files.size()).str().c_str());
}

void CRCDataFileSet::AddFile(CRCDataFile* file)
{
	std::string context = "CRCDataFileSet::AddFile";
	
	if (!file)
	{
		LOG_ERROR(requestID, context, "file is nullptr.");
		return;
	}
	LOG_INFO(requestID, context, (boost::format("Start... file->fileName=%1%...") % file->GetName()).str().c_str());
	_files.push_back(file);
	LOG_INFO(requestID, context, (boost::format("End. Ok. _files collection now contains %1% elements.") % _files.size()).str().c_str());
}

void CRCDataFileSet::AddFile(DataFileType type, std::string file)
{
	std::string context = "CRCDataFileSet::AddFile";
	if (file.empty())
	{
		LOG_ERROR(requestID, context, "file is empty.");
		return;
	}
	if (type == Undefined)
	{
		LOG_ERROR(requestID, context, "Undefined type is not allowed.");
		return;
	}
	
	LOG_INFO(requestID, context, (boost::format("Start... type=%1%, file=%2%") % type % file).str().c_str());

	switch (type)
	{
	case Texture:	
		AddTextureFile(file);	
		break;
	case Altitude:	
		AddAltitudeFile(file);		
		break;
	default:
		LOG_ERROR(requestID, context, "Unknown type detected, need code refactoring.");
		break;
	}
	LOG_INFO(requestID, context, "End. Ok.");
}

void CRCDataFileSet::AddFiles(std::string dir, DataFileType typeFilter, std::string extFilter)
{
	std::string context = "CRCDataFileSet::AddFiles";
	LOG_INFO(requestID, context, (boost::format("Start... dir=%1%, typeFilter=%2%, extFilter=%3%") % dir % typeFilter % extFilter).str().c_str());

	boost::filesystem::path p(dir);

	if (!is_directory(p)) {
		LOG_ERROR(requestID, context, (boost::format("dir=%1% is not a directory") % dir).str().c_str());
		return;
	}
	for (auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(p), {})) {
		std::string filename = entry.path().filename().generic_string();
		std::string ext = filename.substr(filename.length() - 3, 3);
		DataFileType type = CRCDataFile::GetTypeByExt(ext);
		if ((extFilter == "" || ext == extFilter) && (type!=Undefined && typeFilter == type))
		{			
			string file = entry.path().generic_string();
			LOG_INFO(requestID, context, (boost::format("Will add file=%1%") % file).str().c_str());
			AddFile(type, file);
		}
	}
	LOG_INFO(requestID, context, "End. Ok.");	
}

void CRCDataFileSet::ApplyIntersection(CRCDataFile& target)
{
	for (auto it = begin(_files); it != end(_files); ++it)
	{
		if (*it != nullptr && (*it)->Type() == target.Type())
		{
			target.ApplyIntersection((*it));
		}			
	}
}

int CRCDataFileSet::CountFilesOfGivenType(DataFileType type)
{
	int size = 0;
	for (auto it = begin(_files); it != end(_files); ++it)
	{
		if (*it != nullptr && (*it)->Type() == type)
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
	std::string context = "CRCDataFileSet DESTRUCTOR";
	LOG_INFO(requestID, context, (boost::format("Destroying CRCDataFileSet object, containing %1% files") % _files.size()).str().c_str());
	Clear();
	LOG_INFO(requestID, context, "End. Ok.");
}

std::vector<CRCDataFile*>& CRCDataFileSet::Files()
{	
	return _files;
}

void CRCDataFileSet::Clear()
{
	std::string context = "CRCDataFileSet::Clear";
	LOG_INFO__("Deleting files...");
	for (auto it = begin(_files); it != end(_files); ++it)
	{
		if (*it != nullptr)
			delete *it;
	}
	_files.clear();
}
