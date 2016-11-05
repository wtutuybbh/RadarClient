#pragma once
#include "stdafx.h"

enum DataFileType
{
	Undefined,
	Texture,
	Altitude
};
class CRCDataFile
{
protected:
	std::string fileName{ "" };
	double lon0{ 0 }, lat0{ 0 }, lon1{ 180 }, lat1{90};
	int width{ 0 }, height{ 0 };
	unsigned short xdir{ 1 }, ydir{ 1 };
	DataFileType type;
	void *data{ NULL };
	//float *resolutionX{ NULL }, *resolutionY{ NULL };
public:
	CRCDataFile(DataFileType type);
	CRCDataFile(DataFileType type, double lon0, double lat0, double lon1, double lat1, int width, int height);
	CRCDataFile(DataFileType type, const std::string& fileName, double lon0, double lat0, double lon1, double lat1, int width, int height);

	static DataFileType GetTypeByExt(std::string ext);
	virtual ~CRCDataFile();
	DataFileType Type();
	virtual void ApplyIntersection(CRCDataFile &src);
	bool GetIntersection(CRCDataFile &src, int &x0, int &y0, int &x1, int &y1) const;
	void FitSize(float resX, float resY); // set height, width and new boundaries
	virtual bool Open() = 0;
	virtual bool Close() = 0;
	void *Data() const;
	int Width() const;
	int Height() const;
	void SetName(std::string fileName);
	std::string GetName() const;
};

