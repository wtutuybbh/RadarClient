#include "stdafx.h"
#include "CRCAltitudeDataFile.h"
#include "Util.h"
#include "CRCLogger.h"
#include "CRCGeoDataProvider.h"

//(experimental method of log management)
//Errors are always logged!
#define LOG_ENABLED true
#define CRCAltitudeDataFile_v1_LOG true // constructor CRCAltitudeDataFile(const std::string& dt2FileName);
#define CRCAltitudeDataFile_v2_LOG true // constructor CRCAltitudeDataFile(double lon0, double lat0, double lon1, double lat1, int width, int height);
#define CRCAltitudeDataFile_DESTRUCTOR_LOG true // destructor info log
#define CRCAltitudeDataFile_ApplyIntersection_LOG true
#define CRCAltitudeDataFile_Open_LOG true
#define CRCAltitudeDataFile_Close_LOG true
#define CRCAltitudeDataFile_ValueAt_v1_LOG false
#define CRCAltitudeDataFile_ValueAt_v2_LOG false
#define CRCAltitudeDataFile_size_set_max_LOG true
#define CRCAltitudeDataFile_size_set_LOG true

void CRCAltitudeDataFile::size_set_max()
{
	size[0] = size[6] = width;
	size[1] = size[7] = height;
	size[2] = size[3] = 0;
	size[4] = width - 1;
	size[5] = height - 1;

	if (LOG_ENABLED && CRCAltitudeDataFile_size_set_max_LOG) 
	{
		LOG_INFO(requestID, "CRCAltitudeDataFile::size_set_max", (boost::format("fileName=%1%, size=((w, h)=(%2%, %3%), from=(%4%, %5%), to=(%6%, %7%), (w, h)=(%8%, %9%))")
			% fileName % size[0] % size[1] % size[2] % size[3] % size[4] % size[5] % size[6] % size[7]).str().c_str());
	}
}

void CRCAltitudeDataFile::size_set(int x0, int y_0, int x1, int y1)
{
	size[0] = x1 - x0 + 1; //width (lon component) of block
	size[1] = y1 - y_0 + 1; //height (lat component) of block
	size[2] = x0; //(lon component) of block
	size[3] = y_0; //(lat component) of block
	size[4] = x1; //(lon component) of block
	size[5] = y1; //(lat component) of block

	if (LOG_ENABLED && CRCAltitudeDataFile_size_set_LOG)
	{
		LOG_INFO(requestID, "CRCAltitudeDataFile::size_set", (boost::format("fileName=%1%, size=((w, h)=(%2%, %3%), from=(%4%, %5%), to=(%6%, %7%))")
			% fileName % size[0] % size[1] % size[2] % size[3] % size[4] % size[5]).str().c_str());
	}
}

CRCAltitudeDataFile::CRCAltitudeDataFile(const std::string& dt2FileName):
	CRCDataFile(Altitude)
{
	std::string context = "CRCAltitudeDataFile::CRCAltitudeDataFile";
	if (dt2FileName.empty())
	{
		LOG_INFO(requestID, context, "dt2FileName is empty. Will throw exception().");
		throw std::exception("dt2FileName is empty");
	}
	if (LOG_ENABLED && CRCAltitudeDataFile_v1_LOG)
	{
		LOG_INFO(requestID, context, (boost::format("Start... dt2FileName=%1%") % dt2FileName).str().c_str());
	}

	fileName = dt2FileName;
	double LL[10] = { 0 };
	
	int result;

	LL[2] = 180;
	LL[3] = 180;

	if (LOG_ENABLED && CRCAltitudeDataFile_v1_LOG)
	{
		LOG_INFO(requestID, context, (boost::format("before GetAltitudeMapSizes... fileName=%1%, LL={%2%, %3%, %4%, %5%}, size={%6%, %7%, %8%, %9%, %10%, %11%, %12%, %13%}")
			% fileName
			% LL[0] % LL[1] % LL[2] % LL[3]
			% size[0] % size[1] % size[2] % size[3] % size[4] % size[5] % size[6] % size[7]).str().c_str());
	}

	result = GDP::CRCGeoDataProvider::GetAltitudeMapSizes(fileName.c_str(), LL, size);

	auto result_string = (boost::format("GetAltitudeMapSizes returned: result=%1%, LL={%2%, %3%, %4%, %5%, %6%, %7%, %8%, %9%, %10%, %11%}, size={%12%, %13%, %14%, %15%, %16%, %17%, %18%, %19%}")
		% result
		% LL[0] % LL[1] % LL[2] % LL[3] % LL[4] % LL[5] % LL[6] % LL[7] % LL[8] % LL[9]
		% size[0] % size[1] % size[2] % size[3] % size[4] % size[5] % size[6] % size[7]).str();

	if (result != 0)
	{
		std::string error_string = (boost::format("%1%. Will throw exception().") % result_string).str();
		LOG_INFO(requestID, context, error_string.c_str());
		throw std::exception(error_string.c_str());
	}

	if (LOG_ENABLED && CRCAltitudeDataFile_v1_LOG)
	{
		LOG_INFO(requestID, context, result_string.c_str());
	}

	lon0 = LL[8];
	lat0 = LL[9];
	lon1 = LL[8] + LL[6] * (size[6] - 1);
	lat1 = LL[9] + LL[7] * (size[7] - 1);
	width = size[6];
	height = size[7];
	size_set_max();	
	size[8] = (int)GDP::UseFullSize;
	if (LOG_ENABLED && CRCAltitudeDataFile_v1_LOG)
	{
		LOG_INFO(requestID, context, (boost::format("Object created from file %1%, (lon0=%2%, lat0=%3%), (lon1=%4%, lat1=%5%), (width=%6%, height=%7%)")
			% fileName % lon0 % lat0 % lon1 % lat1 % width % height).str().c_str());
	}
}

CRCAltitudeDataFile::CRCAltitudeDataFile(double lon0, double lat0, double lon1, double lat1, int width, int height) :
	CRCDataFile(Altitude, lon0, lat0, lon1, lat1, width, height)
{
	std::string context = "CRCAltitudeDataFile::CRCAltitudeDataFile";
	if (LOG_ENABLED && CRCAltitudeDataFile_v2_LOG)
	{
		LOG_INFO(requestID, context, (boost::format("Start... lon0=%1%, lat0=%2%, lon1=%3%, lat1=%4%, width=%5%, height=%6%")
			% lon0 % lat0 % lon1 % lat1 % width % height).str().c_str());
	}
	size_set_max();
	try
	{
		data = new short[width * height];
		if (!data)
		{
			throw std::bad_alloc();
		}
		if (LOG_ENABLED && CRCAltitudeDataFile_v2_LOG)
		{
			LOG_INFO(requestID, context, (boost::format("allocated %1% bytes for data (sizeof(short)=%2%, width=%3%, height=%4%, total %5% elements)")
				% (sizeof(short) * width * height) % sizeof(short) % width % height % (width * height)).str().c_str());
		}
	}
	catch (std::bad_alloc e)
	{
		std::string error_string = (boost::format("Memory for data not allocated. Size was (short) * ( (w*h) = (%1% * %2%) = %3% )") % width % height % (width * height)).str();
		LOG_ERROR(requestID, context, (error_string + ". RETURN FALSE.").c_str());
		throw std::exception(error_string.c_str());
	}
}

CRCAltitudeDataFile::~CRCAltitudeDataFile()
{
	if (data)
	{
		delete data;
		if (LOG_ENABLED && CRCAltitudeDataFile_DESTRUCTOR_LOG)
		{
			LOG_INFO(requestID, "DESTRUCTOR", "data deleted");
		}
	}
	if (LOG_ENABLED && CRCAltitudeDataFile_DESTRUCTOR_LOG)
	{
		LOG_INFO(requestID, "DESTRUCTOR", "data is nullptr");
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
		if (LOG_ENABLED && CRCAltitudeDataFile_ValueAt_v2_LOG)
		{
			LOG_INFO("ValueAt", "CRCAltitudeDataFile::ValueAt(lon, lat)", "Start... lon=%.6f, lat=%.6f", lon, lat);
		}
		float xf = (width - 1) * (lon - lon0) / (lon1 - lon0);
		float yf = (height - 1) * (lat - lat0) / (lat1 - lat0);

		int x0 = floor(xf), x1 = ceil(xf);
		int y_0 = floor(yf), y1 = ceil(yf);

		if (x0 < 0) x0 = 0;
		if (y_0 < 0) y_0 = 0;
		if (x1 >= width) x1 = width - 1;
		if (y1 >= height) y1 = height - 1;

		if (LOG_ENABLED && CRCAltitudeDataFile_ValueAt_v2_LOG)
		{
			LOG_INFO("ValueAt", "CRCAltitudeDataFile::ValueAt(lon, lat)", "xf=%.6f, yf=%.6f, x0=%d, y_0=%d, x1=%d, y1=%d", xf, yf, x0, y_0, x1, y1);
		}

		short *adata = (short *)data;		
		float ret = BilinearInterpolation(adata[y_0*width + x0], adata[y1*width + x0], adata[y_0*width + x1], adata[y1*width + x1], x0, x1, y_0, y1, xf, yf);
		if (LOG_ENABLED && CRCAltitudeDataFile_ValueAt_v2_LOG)
		{
			LOG_INFO("ValueAt", "CRCAltitudeDataFile::ValueAt(double lon, double lat)", "BilinearInterpolation(q11=%d, q12=%d, q21=%d, q22=%d, x1=%d, x2=%d, y1=%d, y2=%d, x=%.6f, y=%.6f) = %.6f",
				adata[y_0*width + x0], adata[y1*width + x0], adata[y_0*width + x1], adata[y1*width + x1], x0, x1, y_0, y1, xf, yf, ret);
		}
		return ret;
	}
	if (LOG_ENABLED && CRCAltitudeDataFile_ValueAt_v2_LOG)
	{
		LOG_WARN("ValueAt", "CRCAltitudeDataFile::ValueAt(double lon, double lat)", "Out of bounds! lon=%.6f, lat=%.6f", lon, lat);
	}
	return 0;
}

void CRCAltitudeDataFile::SetValue(int x, int y, short val) const
{
	if (type == Altitude && data)
	{
		((short *)data)[y*width + x] = val;
		return;
	}
	if (type != Altitude)
	{
		LOG_ERROR_("CRCAltitudeDataFile::SetValue", "type != Altitude (type is %s)", TypeName());
	}
}

void CRCAltitudeDataFile::ApplyIntersection(CRCDataFile *src)
{
	std::string context = "CRCAltitudeDataFile::ApplyIntersection";
	if (!src)
	{
		LOG_ERROR__("src is nullptr");
		return;
	}

	if(height==0)
	{
		LOG_ERROR__("height==0");
		return;
	}
	if (height == 0)
	{
		LOG_ERROR__("width==0");
		return;
	}
	if (LOG_ENABLED && CRCAltitudeDataFile_ApplyIntersection_LOG)
	{
		LOG_INFO(requestID, context, (boost::format("Start... this->filename=%1%, src.Filename=%2%") % fileName % src->GetName()).str().c_str());
	}	
	

	int this_x0, this_x1, this_y0, this_y1, src_x0, src_x1, src_y0, src_y1;
	CRCAltitudeDataFile *asrc = static_cast<CRCAltitudeDataFile *>(src);
	
	if (!GetIntersection(src, this_x0, this_y0, this_x1, this_y1) || !src->GetIntersection(this, src_x0, src_y0, src_x1, src_y1))
	{
		LOG_WARN(requestID, context, (boost::format("Intersection not found. src.Filename=%1%, this_x0=%2%, this_y0=%3%, this_x1=%4%, this_y1=%5%.") 
			% src->GetName() % this_x0 % this_y0 % this_x1 % this_y1).str().c_str());
		return;
	}
	if (LOG_ENABLED && CRCAltitudeDataFile_ApplyIntersection_LOG)
	{
		LOG_INFO(requestID, context, (boost::format("Intersection found. src.Filename=%1%, this_x0=%2%, this_y0=%3%, this_x1=%4%, this_y1=%5%,    src_x0=%6%, src_y0=%7%, src_x1=%8%, src_y1=%9%.")
			% src->GetName() % this_x0 % this_y0 % this_x1 % this_y1 % src_x0 % src_y0 % src_x1 % src_y1).str().c_str());
	}

	asrc->size_set(src_x0, src_y0, src_x1, src_y1);

	float dlon = DLon();
	float dlat = DLat();

	if (!src->Open())
	{
		LOG_ERROR(requestID, context, "src.Open() failed. RETURN.");
		return;
	}
	for (int x = this_x0; x <= this_x1; x++)
	{
		for (int y = this_y0; y <= this_y1; y++)
		{
			if (y==0 && x == 1)
			{
				int bp = 0;
			}
			SetValue(x, y, asrc->ValueAt(lon0 + x * dlon, lat0 + y * dlat));
		}
	}
}

bool CRCAltitudeDataFile::Open()
{
	std::string context = "CRCAltitudeDataFile::Open";
	if (LOG_ENABLED && CRCAltitudeDataFile_Open_LOG)
	{
		LOG_INFO__("Start... (fileName=%s)", fileName);
	}
	if (!fileName.empty() && !data)
	{		
		double LL[10];
		int result;

		LL[0] = lon0;
		LL[1] = lat0;
		LL[2] = lon1;
		LL[3] = lat1;

		try
		{
			data = new short[width * height]();
			if (!data)
			{
				throw std::bad_alloc();
			}
			if (LOG_ENABLED && CRCAltitudeDataFile_Open_LOG)
			{
				LOG_INFO(requestID, context, (boost::format("allocated %1% bytes for data (sizeof(short)=%2%, width=%3%, height=%4%, total %5% elements)")
					% (sizeof(short) * width * height) % sizeof(short) % width % height % (width * height)).str().c_str());
			}
		}
		catch (std::bad_alloc e)
		{
			LOG_ERROR(requestID, context, "memory for data not allocated. RETURN FALSE.");
			return false;
		}
		short *sdata = (short*)data;
		if (LOG_ENABLED && CRCAltitudeDataFile_Open_LOG)
		{
			LOG_INFO(requestID, context, (boost::format("CRCGeoDataProvider::GetAltitudeMap... fileName=%1%, LL={%2%, %3%, %4%, %5%}, size={%6%, %7%, %8%, %9%, %10%, %11%, %12%, %13%}")
				% fileName
				% LL[0] % LL[1] % LL[2] % LL[3]
				% size[0] % size[1] % size[2] % size[3] % size[4] % size[5] % size[6] % size[7]).str().c_str());
		}
		result = GDP::CRCGeoDataProvider::GetAltitudeMap(fileName.c_str(), LL, size, sdata);

		auto result_string = (boost::format("CRCGeoDataProvider::GetAltitudeMap returned: result=%1%, data=%2% {%3%, %4%, %5%, %6%, %7%, %8%, %9%, ..., %10%, %11%, %12%}") 
			% result
			% data % sdata[0] % sdata[1] % sdata[2] % sdata[3] % sdata[4] % sdata[5] % sdata[6] % sdata[width * height - 3] % sdata[width * height - 2] % sdata[width * height - 1]).str();

		if (result != 0) {
			LOG_ERROR(requestID, context, (result_string + ". RETURN FALSE.").c_str());
			delete[] data;
			data = nullptr;
			return false;
		}							
		if (LOG_ENABLED && CRCAltitudeDataFile_Open_LOG)
		{
			LOG_INFO(requestID, context, (result_string + ". RETURN TRUE").c_str());
		}
		return true;
	}
	if (fileName.empty())
	{
		LOG_ERROR(requestID, context, "fileName is empty. RETURN FALSE.");
	}
	if (data)
	{
		LOG_ERROR(requestID, context, "data is not nullptr (file already opened). RETURN FALSE.");
	}
	return false;
}

bool CRCAltitudeDataFile::Close()
{	
	if (data) 
	{
		delete data;
		if (LOG_ENABLED && CRCAltitudeDataFile_Close_LOG)
		{
			LOG_INFO(requestID, "CRCAltitudeDataFile::Close", (boost::format("data deleted, fileName=%1%") % fileName).str().c_str());
		}
		return true;
	}
	if (LOG_ENABLED && CRCAltitudeDataFile_Close_LOG)
	{
		LOG_INFO(requestID, "CRCAltitudeDataFile::Close", (boost::format("data was nullptr, fileName=%1%") % fileName).str().c_str());
	}
	return false;
}
