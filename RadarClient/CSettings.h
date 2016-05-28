#pragma once
#include <unordered_map>
#include "glm/glm.hpp"
#include "glm/detail/type_mat.hpp"


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
	FloatBlankR2	
};


class CSettings
{
	static std::unordered_map<int, glm::vec4> colors;
	static std::unordered_map<int, float> floats;
	static std::unordered_map<int, int> ints;
public:
	CSettings();
	~CSettings();
	static void Init();
	static glm::vec4 GetColor(int key);
	static float GetFloat(int key);
	static int GetInt(int key);
	static void SetFloat(int key, float value);
};

