#include "CSettings.h"


CSettings::CSettings()
{
}


CSettings::~CSettings()
{
}
std::unordered_map<int, glm::vec4> CSettings::colors;
std::unordered_map<int, float> CSettings::floats;
std::unordered_map<int, int> CSettings::ints;
void CSettings::Init()
{
	colors.insert_or_assign(ColorBackgroud, glm::vec4(0, 0, 0, 1));
	colors.insert_or_assign(ColorAxis, glm::vec4(0.7, 0.7, 0.7, 1));
	colors.insert_or_assign(ColorNumbers, glm::vec4(0.7, 0.7, 0.7, 1));
	colors.insert_or_assign(ColorPointLowLevel, glm::vec4(0.4, 0.4, 0.4, 1));
	colors.insert_or_assign(ColorPointHighLevel, glm::vec4(1, 1, 1, 1));
	colors.insert_or_assign(ColorPointSelected, glm::vec4(0.5, 1, 0.5, 1));
	colors.insert_or_assign(ColorTrack, glm::vec4(1.0, 0.5, 0.5, 1));
	colors.insert_or_assign(ColorTrackSelected, glm::vec4(0.5, 1, 0.5, 1));
	colors.insert_or_assign(ColorMeasureLine, glm::vec4(1, 0.0, 0.0, 1));
	colors.insert_or_assign(ColorAltitudeLowest, glm::vec4(0, 0, 0, 1));
	colors.insert_or_assign(ColorAltitudeHighest, glm::vec4(0.3, 0.3, 0.3, 1));
	colors.insert_or_assign(ColorBlankZones, glm::vec4(1.0, 0.3, 0.3, 1));

	floats.insert_or_assign(FloatMinAltitude, 50);
	floats.insert_or_assign(FloatMaxAltitude, 250);

	floats.insert_or_assign(FloatMaxDistance, 7000);
	floats.insert_or_assign(FloatBlankR1, 120);
	floats.insert_or_assign(FloatBlankR2, 6500);

	ints.insert_or_assign(IntMinimapTextureSize, 256);

	ints.insert_or_assign(IntMarkupMarksPerCircle, 10);
	ints.insert_or_assign(IntMarkupSegmentsPerCircle, 1000);
	ints.insert_or_assign(IntMarkupNumCircles, 7);
	ints.insert_or_assign(IntMarkupMarkDistance, 100);
	ints.insert_or_assign(IntMarkupMarkSize, 10);
	ints.insert_or_assign(IntMarkupHorizontalAxisCount, 18);
	
	ints.insert_or_assign(IntRayDensity, 1);
	
	floats.insert_or_assign(FloatZeroElevation, 30.0f);
}

glm::vec4 CSettings::GetColor(int key)
{
	return colors.at(key);
}

float CSettings::GetFloat(int key)
{
	return floats.at(key);
}

int CSettings::GetInt(int key)
{
	return ints.at(key);
}

void CSettings::SetFloat(int key, float value)
{
	floats.insert_or_assign(key, value);
}
