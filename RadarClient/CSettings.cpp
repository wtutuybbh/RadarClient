#include "stdafx.h"

#include "CSettings.h"
#include "CRImage.h"
#include "CSector.h"
#include "CRCLogger.h"

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
std::map<tstring, Settings> CSettings::stringmap;
std::map<Settings, tstring> CSettings::settingsmap;

ptree CSettings::pt;

bool CSettings::Init()
{
	InitColor(ColorBackground,			TEXT("ColorBackground"),	glm::vec4(0.0, 0.0, 0.0, 1));
	InitColor(ColorAxis,				TEXT("ColorAxis"),			glm::vec4(0.5, 0.5, 0.5, 0.5));
	InitColor(ColorNumbers,				TEXT("ColorNumbers"),		glm::vec4(0.7, 0.7, 0.7, 0.5));
	InitColor(ColorPointLowLevel,		TEXT("ColorPointLowLevel"), glm::vec4(0.4, 0.4, 0.4, 1));


	InitColor(ColorPointHighLevel, TEXT("ColorPointHighLevel"),glm::vec4(1, 1, 1, 1));

	InitColor(ColorPointSelected, TEXT("ColorPointSelected"),glm::vec4(0.5, 1, 0.5, 1));

	InitColor(ColorTrack, TEXT("ColorTrack"),glm::vec4(1.0, 0.5, 0.5, 1));
	
	InitColor(ColorTrackSelected, TEXT("ColorTrackSelected"),glm::vec4(0.5, 1, 0.5, 1));

	InitColor(ColorMeasureLine, TEXT("ColorMeasureLine"),glm::vec4(1, 0.0, 0.0, 1));

	InitColor(ColorAltitudeLowest, TEXT("ColorAltitudeLowest"),glm::vec4(0, 0, 0, 1));

	InitColor(ColorAltitudeHighest, TEXT("ColorAltitudeHighest"), glm::vec4(0.3, 0.3, 0.3, 1));

	InitColor(ColorBlindzoneLowest, TEXT("ColorBlindzoneLowest"), glm::vec4(1, 1, 1, 1));

	InitColor(ColorBlindzoneHighest, TEXT("ColorBlindzoneHighest"), glm::vec4(0, 0, 0, 1));

	InitColor(ColorBlankZones, TEXT("ColorBlankZones"), glm::vec4(1.0, 0.3, 0.3, 1));

	InitFloat(FloatMinAltitude, TEXT("FloatMinAltitude"), 50);

	InitFloat(FloatMaxAltitude, TEXT("FloatMaxAltitude"), 250);

	InitFloat(FloatMaxDistance, TEXT("FloatMaxDistance"), 7000);

	InitFloat(FloatBlankR1, TEXT("FloatBlankR1"), 120);

	InitFloat(FloatBlankR2, TEXT("FloatBlankR2"), 6500);

	InitInt(IntMinimapTextureSize, TEXT("IntMinimapTextureSize"), 256);

	InitInt(IntMarkupMarksPerCircle, TEXT("IntMarkupMarksPerCircle"), 10);

	InitInt(IntMarkupSegmentsPerCircle, TEXT("IntMarkupSegmentsPerCircle"), 1000);

	InitInt(IntMarkupNumCircles, TEXT("IntMarkupNumCircles"), 7);

	InitInt(IntMarkupMarkDistance, TEXT("IntMarkupMarkDistance"), 100);

	InitInt(IntMarkupMarkSize, TEXT("IntMarkupMarkSize"), 10);

	InitInt(IntMarkupHorizontalAxisCount, TEXT("IntMarkupHorizontalAxisCount"), 18);
	
	InitInt(IntRayDensity, TEXT("IntRayDensity"), 1);
	
	InitFloat(FloatZeroElevation, TEXT("FloatZeroElevation"), 3.0f);

	InitFloat(FloatCTrackRefresh_Kr, TEXT("FloatCTrackRefresh_Kr"), 0.75f);
	
	InitFloat(FloatCTrackRefresh_a0, TEXT("FloatCTrackRefresh_a0"), -120.0f);
	
	InitFloat(FloatCTrackRefresh_scale1, TEXT("FloatCTrackRefresh_scale1"), 0.02f);

	InitFloat(FloatCTrackRefresh_scale2, TEXT("FloatCTrackRefresh_scale2"), 1000.0f);

	InitFloat(FloatCTrackRefresh_e0, TEXT("FloatCTrackRefresh_e0"), 0.0f);
	
	InitFloat(FloatCTrackRefresh_scale3, TEXT("FloatCTrackRefresh_scale3"), 1.0f);
	
	InitInt(IntPointColorThreshold_00, TEXT("IntPointColorThreshold_00"), 400);

	InitInt(IntPointColorThreshold_01, TEXT("IntPointColorThreshold_01"), 1000);

	InitInt(IntPointColorThreshold_02, TEXT("IntPointColorThreshold_02"), 2500);

	InitColor(ColorPointColor_00, TEXT("ColorPointColor_00"), glm::vec4(0.3, 0.3, 0.3, 1));
	
	InitColor(ColorPointColor_01, TEXT("ColorPointColor_01"), glm::vec4(0.5, 0.5, 0.5, 1));

	InitColor(ColorPointColor_02, TEXT("ColorPointColor_02"), glm::vec4(0.6, 0.6, 0.6, 1));

	InitColor(ColorPointColor_03, TEXT("ColorPointColor_03"), glm::vec4(0.8, 0.8, 0.8, 1));
	
	InitString(StringHostName, TEXT("StringHostName"), "localhost");
	
	InitInt(IntPort, TEXT("IntPort"), 10001);

	InitFloat(FloatMinBegAzm, TEXT("FloatMinBegAzm"), 0);

	InitFloat(FloatMaxBegAzm, TEXT("FloatMaxBegAzm"), 360);

	InitFloat(FloatMinZeroElevation, TEXT("FloatMinZeroElevation"), 0);

	InitFloat(FloatMaxZeroElevation, TEXT("FloatMaxZeroElevation"), 90);

	InitFloat(FloatPositionLon, TEXT("FloatPositionLon"), 37.712919);

	InitFloat(FloatPositionLat, TEXT("FloatPositionLat"), 55.994606);
	
	InitInt(IntResolution, TEXT("IntResolution"), 100);
	
	InitInt(IntTexSize, TEXT("IntTexSize"), 800);

	InitFloat(FloatMPPh, TEXT("FloatMPPh"), 5.0);

	InitFloat(FloatMPPv, TEXT("FloatMPPv"), 5.0);

	InitInt(IntNazm, TEXT("IntNazm"), 8192);

	InitString(StringCSectorPaletteFileName, TEXT("StringCSectorPaletteFileName"), "rimagecolor.png");

	InitString(StringCRImagePaletteFileName, TEXT("StringCRImagePaletteFileName"), "rpointcolor.png");

		
	InitFloat(FloatCRImageMinAmp, TEXT("FloatCRImageMinAmp"), 0.0);
		
	InitFloat(FloatCRImageMaxAmp, TEXT("FloatCRImageMaxAmp"), 255.0);

	InitFloat(FloatCSectorMinAmp, TEXT("FloatCSectorMinAmp"), 0.0);

	InitFloat(FloatCSectorMaxAmp, TEXT("FloatCSectorMaxAmp"), 255.0);

	InitInt(IntConnectionTimeout, TEXT("IntConnectionTimeout"), 10);
	
	return true;
	
}

bool CSettings::InitPalette()
{
	return CSector::InitPalette(GetString(StringCSectorPaletteFileName)) && CRImage::InitPalette(GetString(StringCRImagePaletteFileName));
}

glm::vec4 CSettings::GetColorFromHexRGBA(std::string rgba)
{
	unsigned short shift = 0;
	if (rgba.substr(0, 1) == "#")
	{
		shift = 1;
	}
	glm::vec4 settingValue;
	unsigned short v;
	std::stringstream ss, ss1, ss2, ss3;

	std::string str_r = rgba.substr(shift, 2);
	ss << std::hex << str_r;
	ss >> v;
	settingValue.r = (float)v / 255.0;

	str_r = rgba.substr(shift + 2, 2);
	ss1 << std::hex << str_r;
	ss1 >> v;
	settingValue.g = (float)v / 255.0;

	str_r = rgba.substr(shift + 4, 2);
	ss2 << std::hex << str_r;
	ss2 >> v;
	settingValue.b = (float)v / 255.0;

	str_r = rgba.substr(shift + 6, 2);
	ss3 << std::hex << str_r;
	ss3 >> v;
	settingValue.a = (float)v / 255.0;

	return settingValue;
}

glm::vec4 CSettings::GetColor(int key)
{
	return colors.at(key);
}

tstring CSettings::GetColorString(int key)
{
	auto color = GetColor(key);
	return GetColorString(color);
}

tstring CSettings::GetColorString(glm::vec4 color)
{
	std::wstringstream str;
	str << "#" << std::hex << std::setw(2) << std::setfill(L'0') << int(255 * color.r);
	str << std::hex << std::setw(2) << std::setfill(L'0') << int(255 * color.g);
	str << std::hex << std::setw(2) << std::setfill(L'0') << int(255 * color.b);
	str << std::hex << std::setw(2) << std::setfill(L'0') << int(255 * color.a);
	return str.str();
}

COLORREF CSettings::GetColorRGB(int key)
{
	auto color = CSettings::GetColor(key);
	return RGB(255 * color.r, 255 * color.g, 255 * color.b);
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
	if (CSettings_Change_Log)
	{
		if (floats.find(key) != floats.end())
		{
			if (floats.at(key) != value)
			{
				LOG_INFO("CSettings", "SetFloat", to__string(GetName(key) + tstring(TEXT(": ")) + to_tstring(std::to_string(floats.at(key))) + tstring(TEXT(" -> ")) + to_tstring(std::to_string(value))).c_str());
			}
		}
	}
	floats.insert_or_assign(key, value);
	pt.put(to__string(GetName(key)), std::to_string(value));
}

void CSettings::InitFloat(Settings key, tstring name, float value)
{
	stringmap.insert_or_assign(name, key);
	settingsmap.insert_or_assign(key, name);
	SetFloat(key, value);
}

void CSettings::SetInt(Settings key, int value)
{
	if (CSettings_Change_Log)
	{
		if (ints.find(key) != ints.end())
		{
			if (ints.at(key) != value)
			{
				LOG_INFO("CSettings", "SetInt", to__string(GetName(key) + tstring(TEXT(": ")) + to_tstring(std::to_string(ints.at(key))) + tstring(TEXT(" -> ")) + to_tstring(std::to_string(value))).c_str());
			}
		}
	}
	ints.insert_or_assign(key, value);
	pt.put(to__string(GetName(key)), std::to_string(value));
}

void CSettings::InitInt(Settings key, tstring name, int value)
{	
	stringmap.insert_or_assign(name, key);
	settingsmap.insert_or_assign(key, name);
	SetInt(key, value);
}

void CSettings::SetColor(Settings key, glm::vec4 value)
{
	if (CSettings_Change_Log)
	{
		if (colors.find(key) != colors.end())
		{
			if (colors.at(key) != value)
			{				
				LOG_INFO("CSettings", "SetColor", to__string(GetName(key) + tstring(TEXT(": ")) + GetColorString(colors.at(key)) + tstring(TEXT(" -> ")) + GetColorString(value)).c_str());
			}
		}
	}
	colors.insert_or_assign(key, value);
	pt.put(to__string(GetName(key)), to__string(GetColorString(value)));

}

void CSettings::InitColor(Settings key, tstring name, glm::vec4 value)
{
	stringmap.insert_or_assign(name, key);
	settingsmap.insert_or_assign(key, name);
	SetColor(key, value);
}

void CSettings::SetString(Settings key, std::string value)
{
	if (CSettings_Change_Log)
	{
		if (strings.find(key) != strings.end())
		{
			if (strings.at(key) != value)
			{
				LOG_INFO("CSettings", "SetString", to__string(GetName(key) + tstring(TEXT(": ")) + to_tstring(strings.at(key)) + tstring(TEXT(" -> ")) + to_tstring(value)).c_str());

			}
		}
	}
	strings.insert_or_assign(key, value);
	pt.put(to__string(GetName(key)), value);
}

void CSettings::InitString(Settings key, tstring name, std::string value)
{
	stringmap.insert_or_assign(name, key);
	settingsmap.insert_or_assign(key, name);
	SetString(key, value);
}

Settings CSettings::GetIndex(tstring name)
{
	return stringmap.at(name);
}

tstring CSettings::GetName(Settings index)
{
	return settingsmap.at(index);
}

void CSettings::Save()
{
	write_json("settings.json", pt);
}

void CSettings::Load()
{
	read_json("settings.json", pt);
	for(auto it = floats.begin(); it != floats.end(); ++it)
	{
		SetFloat(Settings(it->first), pt.get<float>(to__string(GetName(Settings(it->first)))));
	}
	for (auto it = ints.begin(); it != ints.end(); ++it)
	{
		SetInt(Settings(it->first), pt.get<int>(to__string(GetName(Settings(it->first)))));
	}
	for (auto it = colors.begin(); it != colors.end(); ++it)
	{
		SetColor(Settings(it->first), GetColorFromHexRGBA(pt.get<std::string>(to__string(GetName(Settings(it->first))))));
	}
	for (auto it = strings.begin(); it != strings.end(); ++it)
	{
		SetString(Settings(it->first), pt.get<std::string>(to__string(GetName(Settings(it->first)))));
	}
}
