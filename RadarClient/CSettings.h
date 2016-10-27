#pragma once
#include "stdafx.h"


enum Settings
{
	ColorBackgroud,
	ColorAxis,
	ColorNumbers,
	ColorPointLowLevel,
	ColorPointHighLevel,
	ColorPointSelected,
	ColorTrack,
	ColorTrackSelected,
	ColorMeasureLine,
	ColorAltitudeLowest,
	ColorAltitudeHighest,
	ColorBlankZones,
	FloatMinAltitude,
	FloatMaxAltitude,
	IntMinimapTextureSize,
	IntMarkupMarksPerCircle,
	IntMarkupSegmentsPerCircle,
	IntMarkupNumCircles,
	IntMarkupMarkDistance,
	IntMarkupMarkSize,
	IntMarkupHorizontalAxisCount,
	IntRayDensity,
	FloatZeroElevation,
	FloatMaxDistance,
	FloatMPPh,
	FloatMPPv,
	FloatBlankR1,
	FloatBlankR2,
	FloatCTrackRefresh_Kr,
	FloatCTrackRefresh_a0,
	FloatCTrackRefresh_scale1,
	FloatCTrackRefresh_scale2,
	FloatCTrackRefresh_e0,
	FloatCTrackRefresh_scale3,
	IntPointColorThreshold_00,
	IntPointColorThreshold_01,
	IntPointColorThreshold_02,
	ColorPointColor_00,
	ColorPointColor_01,
	ColorPointColor_02,
	ColorPointColor_03,
	StringHostName,
	IntPort,
	FloatMinBegAzm,
	FloatMaxBegAzm,
	FloatMinZeroElevation,
	FloatMaxZeroElevation
};


class CSettings
{
	static std::unordered_map<int, glm::vec4> colors;
	static std::unordered_map<int, float> floats;
	static std::unordered_map<int, int> ints;
	static std::unordered_map<int, std::string> strings;
	static std::map<std::string, Settings> stringmap;
public:
	CSettings();
	~CSettings();
	static void Init();
	static glm::vec4 GetColor(int key);
	static float GetFloat(int key);
	static int GetInt(int key);
	static std::string GetString(int key);
	static void SetFloat(Settings key, float value);
	static void SetInt(Settings key, int value);
	static void SetColor(Settings key, glm::vec4 value);
	static void SetString(Settings key, std::string value);
	static Settings GetIndex(std::string name);
};

