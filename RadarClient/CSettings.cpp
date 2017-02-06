#include "stdafx.h"

#include "CSettings.h"
#include "CRCLogger.h"
#include "CRImage.h"
#include "CSector.h"
#include "Util.h"


CSettings::CSettings()
{
}


CSettings::~CSettings()
{
}

std::unordered_map<int, glm::vec4> CSettings::colors;
std::unordered_map<int, float> CSettings::floats;
std::unordered_map<int, int> CSettings::ints;
std::unordered_map<int, std::string> CSettings::strings;
std::map<std::string, Settings> CSettings::stringmap;

bool CSettings::Init()
{
	colors.insert_or_assign(ColorBackgroud, glm::vec4(0, 0, 0, 1));
	stringmap.insert_or_assign("ColorBackgroud", ColorBackgroud);	

	colors.insert_or_assign(ColorAxis, glm::vec4(0.5, 0.5, 0.5, 0.5));
	stringmap.insert_or_assign("ColorAxis", ColorAxis);

	colors.insert_or_assign(ColorNumbers, glm::vec4(0.7, 0.7, 0.7, 1));
	stringmap.insert_or_assign("ColorNumbers", ColorNumbers);

	colors.insert_or_assign(ColorPointLowLevel, glm::vec4(0.4, 0.4, 0.4, 1));
	stringmap.insert_or_assign("ColorPointLowLevel", ColorPointLowLevel);

	colors.insert_or_assign(ColorPointHighLevel, glm::vec4(1, 1, 1, 1));
	stringmap.insert_or_assign("ColorPointHighLevel", ColorPointHighLevel);

	colors.insert_or_assign(ColorPointSelected, glm::vec4(0.5, 1, 0.5, 1));
	stringmap.insert_or_assign("ColorPointSelected", ColorPointSelected);

	colors.insert_or_assign(ColorTrack, glm::vec4(1.0, 0.5, 0.5, 1));
	stringmap.insert_or_assign("ColorTrack", ColorTrack);

	colors.insert_or_assign(ColorTrackSelected, glm::vec4(0.5, 1, 0.5, 1));
	stringmap.insert_or_assign("ColorTrackSelected", ColorTrackSelected);

	colors.insert_or_assign(ColorMeasureLine, glm::vec4(1, 0.0, 0.0, 1));
	stringmap.insert_or_assign("ColorMeasureLine", ColorMeasureLine);

	colors.insert_or_assign(ColorAltitudeLowest, glm::vec4(0, 0, 0, 1));
	stringmap.insert_or_assign("ColorAltitudeLowest", ColorAltitudeLowest);

	colors.insert_or_assign(ColorAltitudeHighest, glm::vec4(0.3, 0.3, 0.3, 1));
	stringmap.insert_or_assign("ColorAltitudeHighest", ColorAltitudeHighest);

	colors.insert_or_assign(ColorBlankZones, glm::vec4(1.0, 0.3, 0.3, 1));
	stringmap.insert_or_assign("ColorBlankZones", ColorBlankZones);

	floats.insert_or_assign(FloatMinAltitude, 50);
	stringmap.insert_or_assign("FloatMinAltitude", FloatMinAltitude);

	floats.insert_or_assign(FloatMaxAltitude, 250);
	stringmap.insert_or_assign("FloatMaxAltitude", FloatMaxAltitude);

	floats.insert_or_assign(FloatMaxDistance, 7000);
	stringmap.insert_or_assign("FloatMaxDistance", FloatMaxDistance);

	floats.insert_or_assign(FloatBlankR1, 120);
	stringmap.insert_or_assign("FloatBlankR1", FloatBlankR1);

	floats.insert_or_assign(FloatBlankR2, 6500);
	stringmap.insert_or_assign("FloatBlankR2", FloatBlankR2);

	ints.insert_or_assign(IntMinimapTextureSize, 256);
	stringmap.insert_or_assign("IntMinimapTextureSize", IntMinimapTextureSize);

	ints.insert_or_assign(IntMarkupMarksPerCircle, 10);
	stringmap.insert_or_assign("IntMarkupMarksPerCircle", IntMarkupMarksPerCircle);

	ints.insert_or_assign(IntMarkupSegmentsPerCircle, 1000);
	stringmap.insert_or_assign("IntMarkupSegmentsPerCircle", IntMarkupSegmentsPerCircle);

	ints.insert_or_assign(IntMarkupNumCircles, 7);
	stringmap.insert_or_assign("IntMarkupNumCircles", IntMarkupNumCircles);

	ints.insert_or_assign(IntMarkupMarkDistance, 100);
	stringmap.insert_or_assign("IntMarkupMarkDistance", IntMarkupMarkDistance);

	ints.insert_or_assign(IntMarkupMarkSize, 10);
	stringmap.insert_or_assign("IntMarkupMarkSize", IntMarkupMarkSize);

	ints.insert_or_assign(IntMarkupHorizontalAxisCount, 18);
	stringmap.insert_or_assign("IntMarkupHorizontalAxisCount", IntMarkupHorizontalAxisCount);
	
	ints.insert_or_assign(IntRayDensity, 1);
	stringmap.insert_or_assign("IntRayDensity", IntRayDensity);
	
	floats.insert_or_assign(FloatZeroElevation, 3.0f);
	stringmap.insert_or_assign("FloatZeroElevation", FloatZeroElevation);

	floats.insert_or_assign(FloatCTrackRefresh_Kr, 0.75f);
	stringmap.insert_or_assign("FloatCTrackRefresh_Kr", FloatCTrackRefresh_Kr);
	
	floats.insert_or_assign(FloatCTrackRefresh_a0, -120.0f);
	stringmap.insert_or_assign("FloatCTrackRefresh_a0", FloatCTrackRefresh_a0);
	
	floats.insert_or_assign(FloatCTrackRefresh_scale1, 0.02f);
	stringmap.insert_or_assign("FloatCTrackRefresh_scale1", FloatCTrackRefresh_scale1);

	floats.insert_or_assign(FloatCTrackRefresh_scale2, 1000.0f);
	stringmap.insert_or_assign("FloatCTrackRefresh_scale2", FloatCTrackRefresh_scale2);

	floats.insert_or_assign(FloatCTrackRefresh_e0, 0.0f);
	stringmap.insert_or_assign("FloatCTrackRefresh_e0", FloatCTrackRefresh_e0);
	
	floats.insert_or_assign(FloatCTrackRefresh_scale3, 1.0f);
	stringmap.insert_or_assign("FloatCTrackRefresh_scale3", FloatCTrackRefresh_scale3);

	
	ints.insert_or_assign(IntPointColorThreshold_00, 400);
	stringmap.insert_or_assign("IntPointColorThreshold_00", IntPointColorThreshold_00);

	ints.insert_or_assign(IntPointColorThreshold_01, 1000);
	stringmap.insert_or_assign("IntPointColorThreshold_01", IntPointColorThreshold_01);

	ints.insert_or_assign(IntPointColorThreshold_02, 2500);
	stringmap.insert_or_assign("IntPointColorThreshold_02", IntPointColorThreshold_02);

	colors.insert_or_assign(ColorPointColor_00, glm::vec4(0.3, 0.3, 0.3, 1));
	stringmap.insert_or_assign("ColorPointColor_00", ColorPointColor_00);
	
	colors.insert_or_assign(ColorPointColor_01, glm::vec4(0.5, 0.5, 0.5, 1));
	stringmap.insert_or_assign("ColorPointColor_01", ColorPointColor_01);

	colors.insert_or_assign(ColorPointColor_02, glm::vec4(0.6, 0.6, 0.6, 1));
	stringmap.insert_or_assign("ColorPointColor_02", ColorPointColor_02);

	colors.insert_or_assign(ColorPointColor_03, glm::vec4(0.8, 0.8, 0.8, 1));
	stringmap.insert_or_assign("ColorPointColor_03", ColorPointColor_03);
	
	strings.insert_or_assign(StringHostName, "localhost");
	stringmap.insert_or_assign("StringHostName", StringHostName);
	
	ints.insert_or_assign(IntPort, 10001);
	stringmap.insert_or_assign("IntPort", IntPort);

	floats.insert_or_assign(FloatMinBegAzm, 0);
	stringmap.insert_or_assign("FloatMinBegAzm", FloatMinBegAzm);

	floats.insert_or_assign(FloatMaxBegAzm, 360);
	stringmap.insert_or_assign("FloatMaxBegAzm", FloatMaxBegAzm);

	floats.insert_or_assign(FloatMinZeroElevation, 0);
	stringmap.insert_or_assign("FloatMinZeroElevation", FloatMinZeroElevation);

	floats.insert_or_assign(FloatMaxZeroElevation, 90);
	stringmap.insert_or_assign("FloatMaxZeroElevation", FloatMaxZeroElevation);

	floats.insert_or_assign(FloatPositionLon, 37.712919);
	stringmap.insert_or_assign("FloatPositionLon", FloatPositionLon);

	floats.insert_or_assign(FloatPositionLat, 55.994606);
	stringmap.insert_or_assign("FloatPositionLat", FloatPositionLat);
	
	ints.insert_or_assign(IntResolution, 100);
	stringmap.insert_or_assign("IntResolution", IntResolution);
	
	ints.insert_or_assign(IntTexSize, 800);
	stringmap.insert_or_assign("IntResolution", IntTexSize);

	floats.insert_or_assign(FloatMPPh, 5.0);
	stringmap.insert_or_assign("FloatMPPh", FloatMPPh);

	floats.insert_or_assign(FloatMPPv, 5.0);
	stringmap.insert_or_assign("FloatMPPv", FloatMPPv);

	stringmap.insert_or_assign("IntNazm", IntNazm);

	strings.insert_or_assign(StringCSectorPaletteFileName, "rimagecolor.png");
	stringmap.insert_or_assign("StringCSectorPaletteFileName", StringCSectorPaletteFileName);

	strings.insert_or_assign(StringCRImagePaletteFileName, "rpointcolor.png");
	stringmap.insert_or_assign("StringCRImagePaletteFileName", StringCRImagePaletteFileName);

		
	floats.insert_or_assign(FloatCRImageMinAmp, 0.0);
	stringmap.insert_or_assign("FloatCRImageMinAmp", FloatCRImageMinAmp);
		
	floats.insert_or_assign(FloatCRImageMaxAmp, 255.0);
	stringmap.insert_or_assign("FloatCRImageMaxAmp", FloatCRImageMaxAmp);

	floats.insert_or_assign(FloatCSectorMinAmp, 0.0);
	stringmap.insert_or_assign("FloatCSectorMinAmp", FloatCSectorMinAmp);

	floats.insert_or_assign(FloatCSectorMaxAmp, 255.0);
	stringmap.insert_or_assign("FloatCSectorMaxAmp", FloatCSectorMaxAmp);

	ints.insert_or_assign(IntConnectionTimeout, 10);
	stringmap.insert_or_assign("IntConnectionTimeout", IntConnectionTimeout);


	return true;
	
}

bool CSettings::InitPalette()
{
	return CSector::InitPalette(GetString(StringCSectorPaletteFileName)) && CRImage::InitPalette(GetString(StringCRImagePaletteFileName));
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

std::string CSettings::GetString(int key)
{
	return strings.at(key);
}

void CSettings::SetFloat(Settings key, float value)
{
	floats.insert_or_assign(key, value);
}

void CSettings::SetInt(Settings key, int value)
{
	ints.insert_or_assign(key, value);
}

void CSettings::SetColor(Settings key, glm::vec4 value)
{
	colors.insert_or_assign(key, value);
}

void CSettings::SetString(Settings key, std::string value)
{
	strings.insert_or_assign(key, value);
}

Settings CSettings::GetIndex(std::string name)
{
	return stringmap.at(name);
}
