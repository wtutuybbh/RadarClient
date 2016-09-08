#pragma once
#include <vector>
#include <string>

class CRCDataFile;
enum DataFileType; //TODO: can lead to non-compilable state of project, beware

class CRCDataFileSet
{
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
};
