#include <fstream>
#include <iostream>
#include <string>
#include <math.h>
#include <algorithm>
#include <cctype>
#include <functional>


#define UHL_SIZE 80
#define DSI_SIZE 648
#define ACC_SIZE 2700
#define DATFILE_MAXLINELENGTH 256

extern "C"
{

	static inline void ltrim(std::string &s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(),
			std::not1(std::ptr_fun<int, int>(std::isspace))));
	}

	// trim from end (in place)
	static inline void rtrim(std::string &s) {
		s.erase(std::find_if(s.rbegin(), s.rend(),
			std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	}

	// trim from both ends (in place)
	static inline void trim(std::string &s) {
		ltrim(s);
		rtrim(s);
	}

	// trim from start (copying)
	static inline std::string ltrimmed(std::string s) {
		ltrim(s);
		return s;
	}

	// trim from end (copying)
	static inline std::string rtrimmed(std::string s) {
		rtrim(s);
		return s;
	}

	// trim from both ends (copying)
	static inline std::string trimmed(std::string s) {
		trim(s);
		return s;
	}

	__declspec(dllexport) int gdpAltitudeMap_Sizes(const char *fileName, double *LL, int *size) {
		char chr1[1], chr2[2], chr3[3], chr4[5];

		int Nlat, Nlon, q;

		double latSW, lonSW, dlat, dlon;
		//all pointers must me initialized
		if (!fileName || !LL || !size) {
			return -1;
		}
		//filename must be valid character string
		if (strlen(fileName) == 0) {
			return -2;
		}
		std::string strFileName(fileName);
		std::ifstream infile;

		int iLonStart, iLatStart, iLonEnd, iLatEnd;

		if (strFileName.substr(strFileName.length() - 3, 3) == "dt2") {
			
			try {
				infile.open(fileName, std::ifstream::in | std::ifstream::binary);
			}
			catch (...) {
				return -3;
			}

			if (!infile) {
				return -4;
			}

			char UHL[UHL_SIZE];
			infile.read(UHL, UHL_SIZE);
			if (!infile.good()) {
				infile.close(); return -5;
			}
			char DSI[DSI_SIZE];
			infile.read(DSI, DSI_SIZE);
			if (!infile.good()) {
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
			if (!infile.good()) {
				infile.close(); return -7;
			}

			

			

			try {
				infile.close();
			}
			catch (...) {
				return -8;
			}
			
		}
		if (strFileName.substr(strFileName.length() - 3, 3) == "bil")
		{
			//there should be header file, *.hdr:
			std::string hdrFileName = strFileName.substr(0, strFileName.length() - 3).append("hdr");
			try {
				infile.open(hdrFileName, std::ifstream::in | std::ifstream::binary);
			}
			catch (...) {
				return -13;
			}

			if (!infile) {
				return -14;
			}
			char line[DATFILE_MAXLINELENGTH];
			double ulxmap = 0, ulymap = 90, xdim = 0, ydim = 0;
			char *pch;
			std::string strLine;

			while (infile.good()) {
				infile.getline(line, DATFILE_MAXLINELENGTH);
				strLine = std::string(line);
				if (strLine.length() > 0) {
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

		iLonStart = ceil((LL[0] - lonSW) / dlon);
		iLatStart = ceil((LL[1] - latSW) / dlat);
		iLonEnd = floor((LL[2] - lonSW) / dlon);
		iLatEnd = floor((LL[3] - latSW) / dlat);

		if (iLonStart < 0) iLonStart = 0;
		if (iLonEnd >= Nlon) iLonEnd = Nlon - 1;

		if (iLatStart < 0) iLatStart = 0;
		if (iLatEnd >= Nlat) iLatEnd = Nlat - 1;

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
		return 0;
	}
	// gdpAltitudeMap, version 0.1
	// LL: [lat1, lon1, lat2, lon2, lat0, lon0, dlat, dlon] 
	// size: [Nlat, Nlon]
	__declspec(dllexport) int gdpAltitudeMap(const char *fileName, double *LL, int *size, short *data) {
		//TODO: optimize speed

		//all pointers must me initialized
		if (!fileName || !LL || !size || !data) {
			return -1;
		}
		//filename must be valid character string
		if (strlen(fileName) == 0) {
			return -2;
		}

		std::string strFileName(fileName);


		std::ifstream infile;
		try {
			infile.open(fileName, std::ifstream::in | std::ifstream::binary);
		}
		catch (...) {
			return -3;
		}

		if (!infile) {
			return -4;
		}		
		volatile char chr2[2];

		if (strFileName.substr(strFileName.length() - 3, 3) == "dt2") {
			int blocksize = 12 + size[7] * 2;
			for (int m = 0; m < size[0]; m++) {
				infile.seekg(UHL_SIZE + DSI_SIZE + ACC_SIZE + (size[2] + m) * blocksize + 8 + size[3] * 2);
				for (int p = 0; p < size[1]; p++) {
					infile.read((char *)chr2, 2);
					//((nX % m_pTextureImage->sizeX) + ((nY % m_pTextureImage->sizeY) * m_pTextureImage->sizeX))
					data[p + m * size[1]] = ((short)(unsigned char)chr2[0] << 8) + (short)(unsigned char)chr2[1];
				}
			}
			infile.close();
			return 0;
		}
		if (strFileName.substr(strFileName.length() - 3, 3) == "bil") {
			//Band interleaved by line, nbands=1, nbits=16 supported only
			int blocksize = size[6] * 2;
			for (int p = 0; p < size[1]; p++) {
				infile.seekg((size[3] + p) * blocksize + size[2] * 2);
				for (int m = 0; m < size[0]; m++) {
					infile.read((char *)chr2, 2);
					//((nX % m_pTextureImage->sizeX) + ((nY % m_pTextureImage->sizeY) * m_pTextureImage->sizeX))
					data[p + m * size[1]] = (unsigned char)chr2[1] << 8  | (unsigned char)chr2[0];
				}
			}
			infile.close();
			return 0;
		}
		return -5;
	}


}