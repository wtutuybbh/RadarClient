#include <fstream>
#include <iostream>
#include <string>
#include <math.h>


#define UHL_SIZE 80
#define DSI_SIZE 648
#define ACC_SIZE 2700

extern "C"
{
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

		std::ifstream infile;
		try {
			infile.open(fileName, std::ifstream::in | std::ifstream::binary);
		}
		catch(...) {
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

		/*volatile*/ 

		//std::copy(DSI + 273, DSI + 277, chr4);
		
		std::string s(DSI);

		/*for (q = 0; q < 4; q++) chr4[q] = DSI[273 + q];		
		chr4[q] = 0;*/

		dlat = std::stoi(s.substr(273, 4)) / 36000.0;
		
		dlon = std::stoi(s.substr(277, 4)) / 36000.0;

		Nlat = std::stoi(s.substr(281, 4));

		Nlon = std::stoi(s.substr(285, 4));

		latSW = std::stoi(s.substr(185, 2));
		latSW+= std::stoi(s.substr(187, 2));
		latSW+= std::stof(s.substr(189, 4));
		if (s.at(193) == 'S') latSW *= -1;

		lonSW = std::stoi(s.substr(194, 3));
		lonSW += std::stoi(s.substr(197, 2));
		lonSW += std::stof(s.substr(199, 4));
		if (s.at(203) == 'W') lonSW *= -1;

		char ACC[ACC_SIZE];
		infile.read(ACC, ACC_SIZE);
		if (!infile.good()) {
			infile.close(); return -15;
		}

		int iLonStart, iLatStart, iLonEnd, iLatEnd;

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

		try {
			infile.close();
		}
		catch (...) {
			return -18;
		}
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
		int shift, blocksize = 12 + size[7] * 2;
		for (int m = 0; m < size[0]; m++) {
			infile.seekg(UHL_SIZE + DSI_SIZE + ACC_SIZE + (size[2] + m) * blocksize + 8 + size[3] * 2);
			for (int p = 0; p < size[1]; p++) {
				infile.read((char *)chr2, 2);
				//((nX % m_pTextureImage->sizeX) + ((nY % m_pTextureImage->sizeY) * m_pTextureImage->sizeX))
				data[p + m * size[1]] = ((short)(unsigned char)chr2[0] << 8) + (short)(unsigned char)chr2[1];
			}
		}
		/*
		for (int m = 0; m < Nlon; m++) {
			infile.read(chr1, 1);
			if (!infile.good()) {
				result[0] = -13;
				for (int i = 0; i < Nlon; i++) {
					free(data[i]);
				}
				free(data);
				infile.close();
				return NULL;
			}
			checksum = (unsigned char)chr1[0];

			infile.read(chr3, 3);
			dataBlockCount = (((int)(unsigned char)chr3[0]) << 16) + (((int)(unsigned char)chr3[1]) << 8) + ((int)(unsigned char)chr3[2]);
			checksum += (unsigned char)chr3[0] + (unsigned char)chr3[1] + (unsigned char)chr3[2];

			infile.read(chr2, 2);
			lonCount = ((int)(unsigned char)chr2[0] << 8) + (int)(unsigned char)chr2[1];
			checksum += (unsigned char)chr2[0] + (unsigned char)chr2[1];

			infile.read(chr2, 2);
			latCount = ((int)(unsigned char)chr2[0] << 8) + (int)(unsigned char)chr2[1];
			checksum += (unsigned char)chr2[0] + (unsigned char)chr2[1];
			for (int p = 0; p < Nlat; p++) {
				infile.read(chr2, 2);	
				checksum += (unsigned char)chr2[0] + (unsigned char)chr2[1];
				data[m][p] = ((short)(unsigned char)chr2[0] << 8) + (short)(unsigned char)chr2[1];
			} 
			infile.read(chr4, 4);
			if ((((int)(unsigned char)chr4[0]) << 24) + (((int)(unsigned char)chr4[1]) << 16) + (((int)(unsigned char)chr4[2]) << 8) + ((int)(unsigned char)chr4[3]) != checksum) {
				result[0] = -14;
				for (int i = 0; i < Nlon; i++) {
					free(data[i]);
				}
				free(data);
				infile.close();
				return NULL;
			}
		}*/
		infile.close();
		return 0;
	}
}