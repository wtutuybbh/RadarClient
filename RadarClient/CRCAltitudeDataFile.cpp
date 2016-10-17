#include "stdafx.h"
#include "CRCAltitudeDataFile.h"
#include "Util.h"


void CRCAltitudeDataFile::size_set_max()
{
	size[0] = size[6] = width;
	size[1] = size[7] = height;
	size[2] = size[3] = 0;
	size[4] = width - 1;
	size[5] = height - 1;
}

void CRCAltitudeDataFile::size_set(int x0, int y0, int x1, int y1)
{
	size[0] = x1 - x0 + 1; //width (lon component) of block
	size[1] = y1 - y0 + 1; //height (lat component) of block
	size[2] = x0; //(lon component) of block
	size[3] = y0; //(lat component) of block
	size[4] = x1; //(lon component) of block
	size[5] = y1; //(lat component) of block
}

CRCAltitudeDataFile::CRCAltitudeDataFile(const std::string& dt2FileName):
	CRCDataFile(Altitude)
{
	fileName = dt2FileName;

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
		result = gdpAltitudeMap_Sizes(dt2FileName.c_str(), LL, size);
		if (result == 0) {
			lon0 = LL[8];
			lat0 = LL[9];
			lon1 = LL[8] + LL[6] * (size[6] - 1);
			lat1 = LL[9] + LL[7] * (size[7] - 1);
			width = size[6];
			height = size[7];
			size_set_max();
		}
		FreeLibrary(hDLL);
	}
}

CRCAltitudeDataFile::CRCAltitudeDataFile(double lon0, double lat0, double lon1, double lat1, int width, int height) :
	CRCDataFile(Altitude, lon0, lat0, lon1, lat1, width, height)
{
	size_set_max();
	data = new short[width * height];
}

CRCAltitudeDataFile::~CRCAltitudeDataFile()
{
	if (data)
		delete data;
}

short CRCAltitudeDataFile::ValueAt(int x, int y)
{
	if (data)
		return ((short *)data)[y*width + x];
	return 0;
}

short CRCAltitudeDataFile::ValueAt(double lon, double lat)
{	
	if (data && lon >= lon0 && lon <= lon1 && lat >= lat0 && lat <= lat1)
	{
		float xf = width * (lon - lon0) / (lon1 - lon0);
		float yf = height * (lat - lat0) / (lat1 - lat0);

		int x0 = floor(xf), x1 = ceil(xf);
		int y0 = floor(yf), y1 = ceil(yf);

		if (x0 < 0) x0 = 0;
		if (y0 < 0) y0 = 0;
		if (x1 >= width) x1 = width - 1;
		if (y1 >= height) y1 = height - 1;

		//if(x0==x1)

		short *adata = (short *)data;

		float ret = BilinearInterpolation(adata[y0*width + x0], adata[y1*width + x0], adata[y0*width + x1], adata[y1*width + x1], (lon1 - lon0) * x0 / width, (lon1 - lon0) * x1 / width, (lat1 - lat0) * y0 / height, (lat1 - lat0) * y1 / height, lon, lat);
		return ret;
	}
	return 0;
}

void CRCAltitudeDataFile::SetValue(int x, int y, short val/*, float resX, float resY*/)
{
	if (type == Altitude && data /*&& resolutionX && resolutionY*/)
	{
		//if (resX < resolutionX[y*width + x] && resY < resolutionY[y*width + x])
		//{
			//resolutionX[y*width + x] = resX;
			//resolutionY[y*width + x] = resY;
			((short *)data)[y*width + x] = val;
		//}
	}
}

void CRCAltitudeDataFile::ApplyIntersection(CRCDataFile& src)
{
	if (!src.Open())
		return;

	int this_x0, this_x1, this_y0, this_y1, src_x0, src_x1, src_y0, src_y1;
	CRCAltitudeDataFile *asrc = reinterpret_cast <CRCAltitudeDataFile *> (&src);
	
	if (!GetIntersection(src, this_x0, this_y0, this_x1, this_y1) || !src.GetIntersection(*this, src_x0, src_y0, src_x1, src_y1))
		return;

	asrc->size_set(src_x0, src_y0, src_x1, src_y1);

	float dlon = (lon1 - lon0) / width;
	float dlat = (lat1 - lat0) / height;

	for (int x = this_x0; x <= this_x1; x++)
	{
		for (int y = this_y0; y < this_y1; y++)
		{
			SetValue(x, y, asrc->ValueAt(lon0 + x * dlon, lat0 + y * dlat)/*, dlon, dlat*/);
		}
	}
}

bool CRCAltitudeDataFile::Open()
{
	if (fileName != "" && !data)
	{
		HINSTANCE hDLL;               // Handle to DLL
		GDPALTITUDEMAP gdpAltitudeMap;    // Function pointer
		//GDPALTITUDEMAP_SIZES gdpAltitudeMap_Sizes;    // Function pointer
		double LL[10];
		int result;

		hDLL = LoadLibrary("GeoDataProvider.dll");
		if (hDLL != NULL)
		{
			gdpAltitudeMap = (GDPALTITUDEMAP)GetProcAddress(hDLL, "gdpAltitudeMap");
			if (!gdpAltitudeMap)
			{
				FreeLibrary(hDLL);
				return NULL;
			}
			LL[0] = lon0;
			LL[1] = lat0;
			LL[2] = lon1;
			LL[3] = lat1;

			data = new short[width * height];
			/*
			aMapH->sizeX = size[0];
			aMapH->sizeY = size[1];
			aMapH->iLonStart = size[2];
			aMapH->iLatStart = size[3];
			aMapH->iLonEnd = size[4];
			aMapH->iLatEnd = size[5];
			aMapH->Nlon = size[6];
			aMapH->Nlat = size[7];
			*/
			

			result = gdpAltitudeMap(fileName.c_str(), LL, size, (short *)data);

			if (result != 0 && data) {
				delete[] data;
				data = NULL;
				FreeLibrary(hDLL);
				return false;
			}				
			
			FreeLibrary(hDLL);
			return true;
		}
	}
	return false;
}

bool CRCAltitudeDataFile::Close()
{
	if (data) {
		delete data;
		return true;
	}
	return false;
}
