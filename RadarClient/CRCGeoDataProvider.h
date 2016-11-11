#pragma once
#include "stdafx.h"

#define UHL_SIZE 80
#define DSI_SIZE 648
#define ACC_SIZE 2700

class CRCGeoDataProvider
{
	static const std::string requestID;

	// trim from start (in place)
	static void ltrim(std::string &s);

	// trim from end (in place)
	static void rtrim(std::string &s);

	// trim from both ends (in place)
	static void trim(std::string &s); 

	// trim from start (copying)
	static std::string ltrimmed(std::string s); 

	// trim from end (copying)
	static std::string rtrimmed(std::string s); 
	
	// trim from both ends (copying)
	std::string trimmed(std::string s); 
public:
	static int GetAltitudeMapSizes(const char *fileName, double *LL, int *size);
	static int GetAltitudeMap(const char *fileName, double *LL, int *size, short *data);
};
