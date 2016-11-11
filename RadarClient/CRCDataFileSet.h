#pragma once
#include "stdafx.h"

class CRCDataFile;
enum DataFileType; //TODO: can lead to non-compilable state of project, beware

class CRCDataFileSet
{
	static const std::string requestID;
	std::vector<CRCDataFile *> _files;
public:
	void AddTextureFile(std::string imgFile);
	void AddAltitudeFile(std::string altFile);
	

	void AddFile(CRCDataFile *file);
	void AddFile(DataFileType type, std::string file);
	void AddFiles(std::string dir, DataFileType typeFilter, std::string extFilter);
	int CountFilesOfGivenType(DataFileType type);
	CRCDataFile *GetFile(int index);
	~CRCDataFileSet();
	std::vector<CRCDataFile *> &Files();
};

