#pragma once
#include <vector>
#include <string>

class CRCDataFile;
enum DataFileType; //TODO: can lead to non-compilable state of project, beware

class CRCDataFileSet
{
	std::vector<CRCDataFile *> _files;
	void AddTextureFile(std::string imgFile);
	void AddAltitudeFile(std::string altFile);
public:
	void AddFile(CRCDataFile *file);
	void AddFile(DataFileType type, std::string file);
	int CountFilesOfGivenType(DataFileType type);
	CRCDataFile *GetFile(int index);
	~CRCDataFileSet();
};

