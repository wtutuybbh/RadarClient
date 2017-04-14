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
	static const std::string requestID;
	std::string fileName{ std::string("") };
	double lon0{ 0.0 }, lat0{ 0.0 }, lon1{ 180.0 }, lat1{ 90.0 };
	int width{ 0 }, height{ 0 };
	
	DataFileType type { Undefined };
	void *data{ nullptr };
public:
	explicit CRCDataFile(DataFileType type);
	CRCDataFile(DataFileType type, double lon0, double lat0, double lon1, double lat1, int width, int height);
	CRCDataFile(DataFileType type, const std::string& fileName, double lon0, double lat0, double lon1, double lat1, int width, int height);

	static DataFileType GetTypeByExt(std::string ext);
	virtual ~CRCDataFile();
	DataFileType Type() const;
	std::string  TypeName() const;
	virtual void ApplyIntersection(CRCDataFile *src) = 0;
	bool GetIntersection(CRCDataFile *src, int &x0, int &y_0, int &x1, int &y1) const;
	void FitSize(float resX, float resY); // set height, width and new boundaries
	virtual bool Open() = 0;
	virtual bool Close() = 0;
	void *Data() const;
	int Width() const;
	int Height() const;
	double DLon() const;
	double DLat() const;
	double Lon0() const;
	double Lat0() const;
	double Lon1() const;
	double Lat1() const;
	void SetName(std::string fileName);
	std::string GetName() const;
	static std::string GetDataFileTypeName(DataFileType t);
	void SetLonLat(double lon0, double lat0, double lon1, double lat1);
};

