#include "stdafx.h"
#include "CRCAltitudeDataFile.h"
#include "Util.h"
#include "CRCLogger.h"


void CRCAltitudeDataFile::size_set_max()
{
	size[0] = size[6] = width;
	size[1] = size[7] = height;
	size[2] = size[3] = 0;
	size[4] = width - 1;
	size[5] = height - 1;
	CRCLogger::Info(requestID, "CRCAltitudeDataFile::size_set_max", (boost::format("CRCAltitudeDataFile::size=((w, h)=(%1%, %2%), from=(%3%, %4%), to=(%5%, %6%), (w, h)=(%7%, %8%))")
		% size[0] % size[1] % size[2] % size[3] % size[4] % size[5] % size[6] % size[7]).str());
}

void CRCAltitudeDataFile::size_set(int x0, int y0, int x1, int y1)
{
	size[0] = x1 - x0 + 1; //width (lon component) of block
	size[1] = y1 - y0 + 1; //height (lat component) of block
	size[2] = x0; //(lon component) of block
	size[3] = y0; //(lat component) of block
	size[4] = x1; //(lon component) of block
	size[5] = y1; //(lat component) of block

	CRCLogger::Info(requestID, "CRCAltitudeDataFile::size_set", (boost::format("CRCAltitudeDataFile::size=((w, h)=(%1%, %2%), from=(%3%, %4%), to=(%5%, %6%))")
		% size[0] % size[1] % size[2] % size[3] % size[4] % size[5]).str());
}

CRCAltitudeDataFile::CRCAltitudeDataFile(const std::string& dt2FileName):
	CRCDataFile(Altitude)
{
	std::string context = "CRCAltitudeDataFile::CRCAltitudeDataFile";
	if (dt2FileName.empty())
	{
		CRCLogger::Error(requestID, context, "dt2FileName is empty. Will throw exception().");
		throw std::exception("dt2FileName is empty");
	}
	CRCLogger::Info(requestID, context, (boost::format("Start... dt2FileName=%1%") % dt2FileName).str());

	fileName = dt2FileName;

	HINSTANCE hDLL;               // Handle to DLL
	GDPALTITUDEMAP_SIZES gdpAltitudeMap_Sizes;    // Function pointer
	double LL[10];
	int size[8], result;

	hDLL = LoadLibrary(_T("GeoDataProvider.dll"));

	if (hDLL != nullptr)
	{
		gdpAltitudeMap_Sizes = (GDPALTITUDEMAP_SIZES)GetProcAddress(hDLL, "gdpAltitudeMap_Sizes");
		if (!gdpAltitudeMap_Sizes)
		{
			CRCLogger::Error(requestID, context, "Function gdpAltitudeMap_Sizes not loaded. Will throw exception().");
			FreeLibrary(hDLL);
			throw std::exception("Function gdpAltitudeMap_Sizes not loaded");
		}
		LL[0] = 0;
		LL[1] = 0;
		LL[2] = 180;
		LL[3] = 180;

		CRCLogger::Info(requestID, context, (boost::format("gdpAltitudeMap_Sizes... fileName=%1%, LL={%2%, %3%, %4%, %5%}, size={%6%, %7%, %8%, %9%, %10%, %11%, %12%, %13%}")
			% fileName
			% LL[0] % LL[1] % LL[2] % LL[3]
			% size[0] % size[1] % size[2] % size[3] % size[4] % size[5] % size[6] % size[7]).str());

		result = gdpAltitudeMap_Sizes(fileName.c_str(), LL, size);

		auto result_string = (boost::format("gdpAltitudeMap_Sizes returned: result=%1%, LL={%2%, %3%, %4%, %5%, %6%, %7%, %8%, %9%, %10%, %11%}, size={%12%, %13%, %14%, %15%, %16%, %17%, %18%, %19%}")
			% result
			% LL[0] % LL[1] % LL[2] % LL[3] % LL[4] % LL[5] % LL[6] % LL[7] % LL[8] % LL[9]
			% size[0] % size[1] % size[2] % size[3] % size[4] % size[5] % size[6] % size[7]).str();

		if (result != 0)
		{
			std::string error_string = (boost::format("%1%. Will throw exception().") % result_string).str();
			CRCLogger::Error(requestID, context, error_string);
			FreeLibrary(hDLL);
			throw std::exception(error_string.c_str());
		}

		CRCLogger::Info(requestID, context, result_string);

		lon0 = LL[8];
		lat0 = LL[9];
		lon1 = LL[8] + LL[6] * (size[6] - 1);
		lat1 = LL[9] + LL[7] * (size[7] - 1);
		width = size[6];
		height = size[7];
		size_set_max();		
		FreeLibrary(hDLL);
		CRCLogger::Info(requestID, context, (boost::format("Object created from file %1%, (lon0=%2%, lat0=%3%), (lon1=%4%, lat1=%5%), (width=%6%, height=%7%)")
			% fileName % lon0 % lat0 % lon1 % lat1 % width % height).str());
	}
	else
	{
		CRCLogger::Error(requestID, context, "GeoDataProvider.dll not loaded. Will throw exception().");
		throw std::exception("GeoDataProvider.dll not loaded");
	}
}

CRCAltitudeDataFile::CRCAltitudeDataFile(double lon0, double lat0, double lon1, double lat1, int width, int height) :
	CRCDataFile(Altitude, lon0, lat0, lon1, lat1, width, height)
{
	std::string context = "CRCAltitudeDataFile::CRCAltitudeDataFile";
	CRCLogger::Info(requestID, context, (boost::format("Start... lon0=%1%, lat0=%2%, lon1=%3%, lat1=%4%, width=%5%, height=%6%") 
		% lon0 % lat0 % lon1 % lat1 % width % height).str());
	size_set_max();
	try
	{
		data = new short[width * height];
		if (!data)
		{
			throw std::bad_alloc();
		}
	}
	catch (std::bad_alloc e)
	{
		std::string error_string = (boost::format("Memory for data not allocated. Size was (short) * ( (w*h) = (%1% * %2%) = %3% )") % width % height % (width * height)).str();
		CRCLogger::Error(requestID, context, error_string + ". RETURN FALSE.");
		throw std::exception(error_string.c_str());
	}
}

CRCAltitudeDataFile::~CRCAltitudeDataFile()
{
	if (data)
	{
		delete data;
	}
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
	std::string context = "CRCAltitudeDataFile::ApplyIntersection";
	CRCLogger::Info(requestID, context, (boost::format("Start... this->filename=%1%, src.Filename=%2%") % fileName % src.GetName()).str());
	if (!src.Open()) 
	{
		CRCLogger::Error(requestID, context, "src.Open() failed. RETURN.");
		return;
	}

	int this_x0, this_x1, this_y0, this_y1, src_x0, src_x1, src_y0, src_y1;
	CRCAltitudeDataFile *asrc = reinterpret_cast <CRCAltitudeDataFile *> (&src);
	
	if (!GetIntersection(src, this_x0, this_y0, this_x1, this_y1) || !src.GetIntersection(*this, src_x0, src_y0, src_x1, src_y1))
	{
		CRCLogger::Warn(requestID, context, (boost::format("Intersection not found. this_x0=%1%, this_y0=%2%, this_x1=%3%, this_y1=%4%.") 
			% this_x0 % this_y0 % this_x1 % this_y1).str());
		return;
	}
	CRCLogger::Info(requestID, context, (boost::format("Intersection found. this_x0=%1%, this_y0=%2%, this_x1=%3%, this_y1=%4%,    src_x0=%5%, src_y0=%6%, src_x1=%7%, src_y1=%8%.")
		% this_x0 % this_y0 % this_x1 % this_y1 % src_x0 % src_y0 % src_x1 % src_y1).str());

	asrc->size_set(src_x0, src_y0, src_x1, src_y1);

	float dlon = (lon1 - lon0) / width;
	float dlat = (lat1 - lat0) / height;

	for (int x = this_x0; x <= this_x1; x++)
	{
		for (int y = this_y0; y < this_y1; y++)
		{
			SetValue(x, y, asrc->ValueAt(lon0 + x * dlon, lat0 + y * dlat));
		}
	}
}

bool CRCAltitudeDataFile::Open()
{
	std::string context = "CRCAltitudeDataFile::Open";
	CRCLogger::Info(requestID, context, "Start...");
	if (!fileName.empty() && !data)
	{
		HINSTANCE hDLL;               // Handle to DLL
		GDPALTITUDEMAP gdpAltitudeMap;    // Function pointer
		//GDPALTITUDEMAP_SIZES gdpAltitudeMap_Sizes;    // Function pointer
		double LL[10];
		int result;

		hDLL = LoadLibrary("GeoDataProvider.dll");
		if (hDLL != nullptr)
		{
			gdpAltitudeMap = (GDPALTITUDEMAP)GetProcAddress(hDLL, "gdpAltitudeMap");
			if (!gdpAltitudeMap)
			{
				FreeLibrary(hDLL);
				CRCLogger::Error(requestID, context, "Function gdpAltitudeMap not loaded. RETURN FALSE.");
				return false;
			}
			LL[0] = lon0;
			LL[1] = lat0;
			LL[2] = lon1;
			LL[3] = lat1;

			try
			{
				data = new short[width * height];
				if (!data)
				{
					throw std::bad_alloc();
				}
			}
			catch (std::bad_alloc e)
			{
				CRCLogger::Error(requestID, context, "memory for data not allocated. RETURN FALSE.");
				return false;
			}
			short *sdata = (short*)data;
			CRCLogger::Info(requestID, context, (boost::format("gdpAltitudeMap... fileName=%1%, LL={%2%, %3%, %4%, %5%}, size={%6%, %7%, %8%, %9%, %10%, %11%, %12%, %13%}")
				% fileName 
				% LL[0] % LL[1] % LL[2] % LL[3]
				% size[0] % size[1] % size[2] % size[3] % size[4] % size[5] % size[6] % size[7]).str());
			result = gdpAltitudeMap(fileName.c_str(), LL, size, (short *)data);

			auto result_string = (boost::format("gdpAltitudeMap returned: result=%1%, data=%2% {%3%, %4%, %5%, %6%, %7%, %8%, %9%, ...}") 
				% result
				% data % sdata[0] % sdata[1] % sdata[2] % sdata[3] % sdata[4] % sdata[5] % sdata[6]).str();

			if (result != 0) {
				CRCLogger::Error(requestID, context, result_string + ". RETURN FALSE.");
				delete[] data;
				data = nullptr;
				FreeLibrary(hDLL);
				return false;
			}							
			FreeLibrary(hDLL);
			CRCLogger::Error(requestID, context, result_string + ". RETURN TRUE");
			return true;
		}
		else
		{
			CRCLogger::Error(requestID, context, "GeoDataProvider.dll not loaded. RETURN FALSE.");
		}
	}
	else 
	{
		if (fileName.empty())
		{
			CRCLogger::Error(requestID, context, "fileName is empty. RETURN FALSE.");
		}
		if (data)
		{
			CRCLogger::Error(requestID, context, "data is not nullptr (file already opened). RETURN FALSE.");
		}
	}
	return false;
}

bool CRCAltitudeDataFile::Close()
{
	CRCLogger::Info(requestID, "CRCAltitudeDataFile::Close", (boost::format("fileName=%1%") % fileName).str());
	if (data) {
		delete data;
		return true;
	}
	return false;
}
