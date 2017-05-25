#pragma once
#include "stdafx.h"

using boost::property_tree::ptree;
using boost::property_tree::read_json;
using boost::property_tree::write_json;

enum Settings
{
	ColorBackground,
	ColorMarkup,
	ColorNumbers,
	ColorPointLowLevel,
	ColorPointHighLevel,
	ColorPointSelected,
	ColorTrack,
	ColorTrackSelected,
	ColorMeasureLine,
	ColorAltitudeLowest,
	ColorAltitudeHighest,
	ColorBlindzoneLowest,
	ColorBlindzoneHighest,
	ColorBlankZones,
	ColorPointColor_00,
	ColorPointColor_01,
	ColorPointColor_02,
	ColorPointColor_03,
	ColorMarkupInvisible,
	ColorMeasureLineInvisible,

	IntMinimapTextureSize,
	IntMarkupMarksPerCircle,
	IntMarkupSegmentsPerCircle,
	IntMarkupNumCircles,
	IntMarkupMarkDistance,
	IntMarkupMarkSize,
	IntMarkupHorizontalAxisCount,
	IntRayDensity,
	IntPointColorThreshold_00,
	IntPointColorThreshold_01,
	IntPointColorThreshold_02,
	IntPort,
	IntResolution,
	IntTexSize,
	IntNazm,
	IntConnectionTimeout,
	IntLoadMesh,
	IntPointLifetime,

	
	FloatMaxDistance,
	FloatBlankR1,
	FloatBlankR2,
	FloatPositionRadarHeight,
	FloatMarkDistance,
	FloatMaxDist,

	FloatZeroElevation,
	FloatMinZeroElevation,
	FloatMaxZeroElevation,
	FloatMinBegAzm,
	FloatMaxBegAzm,

	FloatMPPh,
	FloatMPPv,
	
	FloatCTrackRefresh_Kr,
	FloatCTrackRefresh_a0,
	FloatCTrackRefresh_scale1,
	FloatCTrackRefresh_scale2,
	FloatCTrackRefresh_e0,
	FloatCTrackRefresh_scale3,	
	

	FloatPositionLon,
	FloatPositionLat,
	
	FloatCRImageMinAmp,
	FloatCRImageMaxAmp,
	FloatCSectorMinAmp,
	FloatCSectorMaxAmp,
	
	FloatDT0,
	FloatDTMin,
	FloatFovy, 
	FloatAspect, 
	FloatZNear, 
	FloatZFar,

	FloatAmp_00,
	FloatAmp_01,
	FloatAmp_02,

	FloatAmpPalettePosition_00,
	FloatAmpPalettePosition_01,
	FloatAmpPalettePosition_02,


	StringHostName,
	StringCSectorPaletteFileName,
	StringCRImagePaletteFileName
};


class CSettings
{
	static std::unordered_map<int, glm::vec4> colors;
	static std::unordered_map<int, float> floats;
	static std::unordered_map<int, int> ints;
	static std::unordered_map<int, std::string> strings;
	static std::map<tstring, Settings> stringmap;
	static std::map<Settings, tstring> settingsmap;

	static FIBITMAP *RPointPalette;
	static FIBITMAP *RImagePalette;
	static int RPointPaletteWidth;
	static int RImagePaletteWidth;

	
public:
	/*static ptree timing;
	static std::map<tstring, Settings> stringmap;*/

	static ptree pt;
	
	CSettings();
	~CSettings();
	static bool Init();
	static bool InitPalette();
	static glm::vec4 GetColorFromHexRGBA(std::string rgba);
	static glm::vec4 GetColor(int key);
	static tstring GetColorString(int key);
	static tstring GetColorString(glm::vec4 color);
	static COLORREF GetColorRGB(int key);
	static float GetFloat(int key);
	static int GetInt(int key);
	static std::string GetString(int key);

	static void SetFloat(Settings key, float value);
	static void InitFloat(Settings key, tstring name, float value);

	static void SetInt(Settings key, int value);
	static void InitInt(Settings key, tstring name, int value);

	static void SetColor(Settings key, glm::vec4 value);
	static void InitColor(Settings key, tstring name, glm::vec4 value);

	static void SetString(Settings key, std::string value);
	static void InitString(Settings key, tstring name, std::string value);

	static int GetIndex(tstring name);
	static tstring GetName(Settings index);
	static void Save();
	static void Load();
};

