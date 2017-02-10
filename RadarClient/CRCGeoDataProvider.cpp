#include "stdafx.h"
#include "CRCGeoDataProvider.h"
#include "CRCLogger.h"
#include "Util.h"

#define LOG_ENABLED true
#define CRCGeoDataProvider_GetAltitudeMapSizes_LOG true
#define CRCGeoDataProvider_GetAltitudeMap_LOG true

using namespace GDP;

const std::string CRCGeoDataProvider::requestID = "CRCGeoDataProvider";



void CRCGeoDataProvider::ltrim(std::string& s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(),
		std::not1(std::ptr_fun<int, int>(std::isspace))));
}

void CRCGeoDataProvider::rtrim(std::string& s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(),
		std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

void CRCGeoDataProvider::trim(std::string& s)
{
	ltrim(s);
	rtrim(s);
}

std::string CRCGeoDataProvider::ltrimmed(std::string s)
{
	ltrim(s);
	return s;
}

std::string CRCGeoDataProvider::rtrimmed(std::string s)
{
	rtrim(s);
	return s;
}

std::string CRCGeoDataProvider::trimmed(std::string s)
{
	trim(s);
	return s;
}

int CRCGeoDataProvider::GetAltitudeMapSizes(const char* fileName, double* LL, int* size)
{
	std::string context = "GetAltitudeMapSizes";
	//all pointers must me initialized:
	if (!fileName) 
	{
		LOG_ERROR__("parameter fileName is nullptr");
		return -1;
	}
	//filename must be valid character string
	if (strlen(fileName) == 0) 
	{
		LOG_ERROR__("parameter fileName is empty");
		return -2;
	}
	//all pointers must me initialized:
	if (!LL)
	{
		LOG_ERROR__("parameter LL is nullptr");
		return -1;
	}
	//all pointers must me initialized:
	if(!size)
	{
		LOG_ERROR__("parameter size is nullptr");
		return -1;
	}

	if (LOG_ENABLED && CRCGeoDataProvider_GetAltitudeMapSizes_LOG)
	{
		LOG_INFO__("Start... fileName=%s, LL[0]=%.6f, LL[1]=%.6f, LL[2]=%.6f, LL[3]=%.6f, LL[4]=%.6f, LL[5]=%.6f, LL[6]=%.6f, LL[7]=%.6f, LL[8]=%.6f, LL[9]=%.6f", 
			fileName, LL[0], LL[1], LL[2], LL[3], LL[4], LL[5], LL[6], LL[7], LL[8], LL[9]);
	}
	char chr1[1], chr2[2], chr3[3], chr4[5];

	int Nlat, Nlon, q;

	double latSW, lonSW, dlat, dlon;
	
	
	std::string strFileName(fileName);
	std::ifstream infile;

	int iLonStart, iLatStart, iLonEnd, iLatEnd;

	std::string ext = strFileName.substr(strFileName.length() - 3, 3);

	if (ext.compare("dt2") == 0) 
	{
		if (LOG_ENABLED && CRCGeoDataProvider_GetAltitudeMapSizes_LOG)
		{
			LOG_INFO__("input detected as DT2 file");
		}
		try 
		{
			infile.open(fileName, std::ifstream::in | std::ifstream::binary);
		}
		catch (...) 
		{
			LOG_ERROR__("exception when opening the file %s", fileName);
			return -3;
		}

		if (!infile) 
		{
			LOG_ERROR__("infile is nullptr (fileName=%s)", fileName);
			return -4;
		}

		char UHL[UHL_SIZE];
		infile.read(UHL, UHL_SIZE);
		if (!infile.good()) 
		{
			LOG_ERROR__("unexpected end of file after reading UHL");
			infile.close(); return -5;
		}
		char DSI[DSI_SIZE];
		infile.read(DSI, DSI_SIZE);
		if (!infile.good()) 
		{
			LOG_ERROR__("unexpected end of file after reading DSI");
			infile.close(); return -6;
		}

		std::string s(DSI);

		dlat = std::stoi(s.substr(273, 4)) / 36000.0;

		dlon = std::stoi(s.substr(277, 4)) / 36000.0;

		Nlat = std::stoi(s.substr(281, 4));

		Nlon = std::stoi(s.substr(285, 4));

		latSW = std::stoi(s.substr(185, 2));
		latSW += std::stoi(s.substr(187, 2));
		latSW += std::stof(s.substr(189, 4));
		if (s.at(193) == 'S') latSW *= -1;

		lonSW = std::stoi(s.substr(194, 3));
		lonSW += std::stoi(s.substr(197, 2));
		lonSW += std::stof(s.substr(199, 4));
		if (s.at(203) == 'W') lonSW *= -1;

		char ACC[ACC_SIZE];
		infile.read(ACC, ACC_SIZE);
		if (!infile.good()) 
		{
			LOG_ERROR__("unexpected end of file after reading ACC");
			infile.close(); return -7;
		}		
		infile.close();
	}
	else if (ext.compare("bil")==0)
	{
		if (LOG_ENABLED && CRCGeoDataProvider_GetAltitudeMapSizes_LOG)
		{
			LOG_INFO__("input detected as BIL file, will read corresponding hdr file");
		}
		//there should be header file, *.hdr:
		std::string hdrFileName = strFileName.substr(0, strFileName.length() - 3).append("hdr");
		try 
		{
			infile.open(hdrFileName, std::ifstream::in | std::ifstream::binary);
		}
		catch (...) 
		{
			LOG_ERROR__("exception when opening the file %s", hdrFileName);
			return -13;
		}

		if (!infile) 
		{
			LOG_ERROR__("infile is nullptr (hdrFileName=%s)", hdrFileName);
			return -14;
		}
		char line[DATFILE_MAXLINELENGTH];
		double ulxmap = 0, ulymap = 90, xdim = 0, ydim = 0;
		char *pch;
		std::string strLine;

		while (infile.good()) 
		{
			infile.getline(line, DATFILE_MAXLINELENGTH);
			strLine = std::string(line);
			if (strLine.length() > 0) 
			{
				std::transform(strLine.begin(), strLine.end(), strLine.begin(), ::toupper);
				if (strLine.substr(0, 9) == "BYTEORDER")
				{
					std::string value = strLine.substr(9, strLine.length() - 9);
					trim(value);
					if (value != "I")
						return -15;
				}
				if (strLine.substr(0, 6) == "LAYOUT")
				{
					std::string value = strLine.substr(6, strLine.length() - 6);
					trim(value);
					if (value != "BIL")
						return -16;
				}
				if (strLine.substr(0, 9) == "PIXELTYPE")
				{
					std::string value = strLine.substr(9, strLine.length() - 9);
					trim(value);
					if (value != "SIGNEDINT")
						return -17;
				}
				if (strLine.substr(0, 5) == "NBANDS")
				{
					if (std::stoi(strLine.substr(5, strLine.length() - 5)) != 1)
						return -18;
				}
				if (strLine.substr(0, 5) == "NBITS")
				{
					if (std::stoi(strLine.substr(5, strLine.length() - 5)) != 16)
						return -19;
				}

				if (strLine.substr(0, 5) == "NROWS")
				{
					size[7] = std::stoi(strLine.substr(5, strLine.length() - 5));
				}
				if (strLine.substr(0, 5) == "NCOLS")
				{
					size[6] = std::stoi(strLine.substr(5, strLine.length() - 5));
				}
				if (strLine.substr(0, 6) == "ULXMAP")
				{
					ulxmap = std::stof(strLine.substr(6, strLine.length() - 6));
				}
				if (strLine.substr(0, 6) == "ULYMAP")
				{
					ulymap = std::stof(strLine.substr(6, strLine.length() - 6));
				}
				if (strLine.substr(0, 4) == "XDIM")
				{
					xdim = std::stof(strLine.substr(4, strLine.length() - 4));
				}
				if (strLine.substr(0, 4) == "YDIM")
				{
					ydim = std::stof(strLine.substr(4, strLine.length() - 4));
				}
			}
		}
		latSW = ulymap - ydim * size[7];
		lonSW = ulxmap;
		dlat = ydim;
		dlon = xdim;
		Nlat = size[7];
		Nlon = size[6];

		infile.close();
	}
	else
	{
		LOG_ERROR__("unknown file type (%s)", ext);
		return -8;
	}
	

	iLonStart = ceil((LL[0] - lonSW) / dlon);
	iLatStart = ceil((LL[1] - latSW) / dlat);
	iLonEnd = floor((LL[2] - lonSW) / dlon);
	iLatEnd = floor((LL[3] - latSW) / dlat);

	if (LOG_ENABLED && CRCGeoDataProvider_GetAltitudeMapSizes_LOG)
	{
		LOG_INFO__("### fileName=%s, LL={%.6f, %.6f, %.6f, %.6f}, lonSW=%.6f, latSW=%.6f, iLonStart=%d, iLatStart=%d, iLonEnd=%d, iLatEnd=%d", 
			fileName, LL[0], LL[1], LL[2], LL[3], lonSW, latSW, iLonStart, iLatStart, iLonEnd, iLatEnd);
	}

	if (iLonStart < 0) iLonStart = 0;
	if (iLonEnd >= Nlon) iLonEnd = Nlon - 1;

	if (iLatStart < 0) iLatStart = 0;
	if (iLatEnd >= Nlat) iLatEnd = Nlat - 1;

	if (LOG_ENABLED && CRCGeoDataProvider_GetAltitudeMapSizes_LOG)
	{
		LOG_INFO__("### fileName=%s, LL={%.6f, %.6f, %.6f, %.6f}, lonSW=%.6f, latSW=%.6f, iLonStart=%d, iLatStart=%d, iLonEnd=%d, iLatEnd=%d",
			fileName, LL[0], LL[1], LL[2], LL[3], lonSW, latSW, iLonStart, iLatStart, iLonEnd, iLatEnd);
	}


	size[0] = iLonEnd - iLonStart + 1;
	size[1] = iLatEnd - iLatStart + 1;
	size[2] = iLonStart;
	size[3] = iLatStart;
	size[4] = iLonEnd;
	size[5] = iLatEnd;
	size[6] = Nlon;
	size[7] = Nlat;

	LL[4] = lonSW + iLonStart * dlon;
	LL[5] = latSW + iLatStart * dlat;
	LL[6] = dlon;
	LL[7] = dlat;
	LL[8] = lonSW;
	LL[9] = latSW;
	if (LOG_ENABLED && CRCGeoDataProvider_GetAltitudeMapSizes_LOG)
	{
		LOG_INFO__("End (part 1). fileName=%s. size={width=%d, height=%d, x0=%d, y_0=%d, x1=%d, y1=%d, filewidth=%d, fileheight=%d}",
			fileName, size[0], size[1], size[2], size[3], size[4], size[5], size[6], size[7]);
		LOG_INFO__("End (part 2). fileName=%s. LL={.,.,.,., lon0=%.6f, lat0=%.6f, dlon=%.6f, dlat=%.6f, lonSW=%.6f, latSW=%.6f}",
			fileName, LL[4], LL[5], LL[6], LL[7], LL[8], LL[9]);
	}
	return 0;
}

int CRCGeoDataProvider::GetAltitudeMap(const char* fileName, double* LL, int* size, short* data)
{
	//TODO: optimize speed
	std::string context = "GetAltitudeMap";
	//all pointers must me initialized:
	if (!fileName)
	{
		LOG_ERROR__("parameter fileName is nullptr");
		return -1;
	}
	//filename must be valid character string
	if (strlen(fileName) == 0)
	{
		LOG_ERROR__("parameter fileName is empty");
		return -2;
	}
	//all pointers must me initialized:
	if (!LL)
	{
		LOG_ERROR__("parameter LL is nullptr");
		return -1;
	}
	//all pointers must me initialized:
	if (!size)
	{
		LOG_ERROR__("parameter size is nullptr");
		return -1;
	}
	//all pointers must me initialized:
	if (!data)
	{
		LOG_ERROR__("parameter data is nullptr");
		return -1;
	}

	std::string strFileName(fileName);

	std::string ext = strFileName.substr(strFileName.length() - 3, 3);
	if (ext.compare("dt2") != 0 && ext.compare("bil") != 0)
	{
		LOG_ERROR__("unknown file type (%s)", ext);
		return -8;
	}

	std::ifstream infile;
	try 
	{
		infile.open(fileName, std::ifstream::in | std::ifstream::binary);
	}
	catch (...) 
	{
		LOG_ERROR__("exception when opening the file %s", fileName);
		return -3;
	}

	if (!infile) 
	{
		LOG_ERROR__("infile is nullptr (fileName=%s)", fileName);
		return -4;
	}
	
	if (LOG_ENABLED && CRCGeoDataProvider_GetAltitudeMap_LOG)
	{
		LOG_INFO__("Start (part 1)... fileName=%s. size={width=%d, height=%d, x0=%d, y_0=%d, x1=%d, y1=%d, filewidth=%d, fileheight=%d}",
			fileName, size[0], size[1], size[2], size[3], size[4], size[5], size[6], size[7]);
		LOG_INFO__("Start (part 2)... fileName=%s, LL[0]=%.6f, LL[1]=%.6f, LL[2]=%.6f, LL[3]=%.6f, LL[4]=%.6f, LL[5]=%.6f, LL[6]=%.6f, LL[7]=%.6f, LL[8]=%.6f, LL[9]=%.6f",
			fileName, LL[0], LL[1], LL[2], LL[3], LL[4], LL[5], LL[6], LL[7], LL[8], LL[9]);
	}
	volatile char chr2[2];

	if (ext.compare("dt2") == 0) 
	{
		if (LOG_ENABLED && CRCGeoDataProvider_GetAltitudeMap_LOG)
		{
			LOG_INFO__("input detected as DT2 file, method=%d", size[8]);
		}
		int blocksize;
		switch (size[8])
		{
		case (int)GDP::UseRegionSize:
			blocksize = 12 + size[7] * 2;
			for (int m = 0; m < size[0]; m++) {
				infile.seekg(UHL_SIZE + DSI_SIZE + ACC_SIZE + (size[2] + m) * blocksize + 8 + size[3] * 2);
				for (int p = 0; p < size[1]; p++) {
					infile.read((char *)chr2, 2);
					data[p + m * size[1]] = ((short)(unsigned char)chr2[0] << 8) + (short)(unsigned char)chr2[1];
				}
			}
			break;
		case (int)GDP::UseFullSize:
			blocksize = 12 + size[7] * 2;
			//p ~ lat
			//m ~ lon
			for (int m = 0; m < size[0]; m++) {
				infile.seekg(UHL_SIZE + DSI_SIZE + ACC_SIZE + (size[2] + m) * blocksize + 8 + size[3] * 2);
				for (int p = 0; p < size[1]; p++) {
					infile.read((char *)chr2, 2);
					data[size[3] + p + (m + size[2]) * size[7]] = (unsigned char)chr2[1] << 8 | (unsigned char)chr2[0];
				}
			}
			break;
		default:
			LOG_ERROR__("unknown method");
			return -9;
			break;
		}
		
		infile.close();
		if (LOG_ENABLED && CRCGeoDataProvider_GetAltitudeMap_LOG)
		{
			if (size[0] * size[1] >= 8)
			{
				LOG_INFO__("data={%d, %d, %d, %d, ... , %d, %d, %d, %d}",
					data[0], data[1], data[2], data[3], data[size[0] * size[1] - 4], data[size[0] * size[1] - 3], data[size[0] * size[1] - 2], data[size[0] * size[1] - 1]);
			}
			else
			{
				LOG_INFO__("data={%d, ... }", data[0]);
			}
		}
		return 0;
	}
	if (ext.compare("bil") == 0) {
		if (LOG_ENABLED && CRCGeoDataProvider_GetAltitudeMap_LOG)
		{
			LOG_INFO__("input detected as BIL file, method=%d", size[8]);
		}
		//Band interleaved by line, nbands=1, nbits=16 supported only
		int blocksize;
		switch(size[8])
		{
		case (int)GDP::UseRegionSize:
			blocksize = size[6] * 2;
			for (int p = 0; p < size[1]; p++) {
				infile.seekg((size[3] + p) * blocksize + size[2] * 2);
				for (int m = 0; m < size[0]; m++) {
					infile.read((char *)chr2, 2);
					data[p + m * size[1]] = (unsigned char)chr2[1] << 8 | (unsigned char)chr2[0];
				}
			}
			break;
		case (int)GDP::UseFullSize:
			//p ~ lat
			//m ~ lon
			
			for (int p = 0; p < size[1]; p++) {
				//size={width=463, height=240, x0=1041, y_0=3361, x1=1503, y1=3600, filewidth=1801, fileheight=3601}
				// =1801*3360+1040 :
				int rowstart_file = (size[7] - size[5] - 1 + p)*size[6] + size[2];
				int rowstart_data = (size[5] - p) * size[6] + size[2];
				infile.seekg(rowstart_file *2);
				for (int m = 0; m < size[0]; m++) {
					infile.read((char *)chr2, 2);
					data[rowstart_data + m] = (unsigned char)chr2[1] << 8 | (unsigned char)chr2[0];
				}
				/*if (LOG_ENABLED && CRCGeoDataProvider_GetAltitudeMap_LOG)
				{
					LOG_INFO__("loaded values data[%d] = %d ... data[%d] = %d", rowstart_data, data[rowstart_data], rowstart_data - size[0] + 1, data[rowstart_data - size[0] + 1]);
				}*/
			}
			break;
		default:
			LOG_ERROR__("unknown method");
			return -9;
			break;
		}

		infile.close();
		if (LOG_ENABLED && CRCGeoDataProvider_GetAltitudeMap_LOG)
		{
			if (size[0] * size[1] >= 8)
			{
				LOG_INFO__("data={%d, %d, %d, %d, ... , %d, %d, %d, %d}",
					data[0], data[1], data[2], data[3], data[size[0] * size[1] - 4], data[size[0] * size[1] - 3], data[size[0] * size[1] - 2], data[size[0] * size[1] - 1]);
			}
			else 
			{
				LOG_INFO__("data={%d, ... }", data[0]);
			}
		}
		return 0;
	}
	LOG_ERROR__("unknown file type (%s)", ext);
	return -8;
}
