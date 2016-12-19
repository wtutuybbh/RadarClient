#include "stdafx.h"
#include "CRImage.h"
#include "CRCSocket.h"
#include "CSettings.h"
#include "C3DObjectVBO.h"
#include "C3DObjectProgram.h"
#include "CViewPortControl.h"
#include "CRCLogger.h"

float CRImage::maxAmp = 70;
float CRImage::minAmp = 0;
CRImage::CRImage(float azemuth, glm::vec4 origin, float mpph, float mppv, RDR_INITCL * rdrinit, RIMAGE* info, void* pixels) : 
	C3DObjectModel(
		new C3DObjectVBO(true), 
		nullptr,
		new C3DObjectProgram("CRImage.v.glsl", "CRImage.f.glsl", "vertex", nullptr, nullptr, "color"))
{
	Azemuth = azemuth;
	float *px = (float *)pixels;

	short currentDirection = sgn(info->d2 - info->d1);

	//glm::vec4 mincolor = CSettings::GetColor(ColorPointLowLevel);
	//glm::vec4 maxcolor = CSettings::GetColor(ColorPointHighLevel);

	if (rdrinit->ScanMode == 3) // 3D
	{

	}
	else // 2D
	{
		vector<VBOData> *vbuffer =  new vector<VBOData>;

		float e = glm::radians(CSettings::GetFloat(FloatZeroElevation));
		int paletteIndex;
		RGBQUAD pixelcolor;
		glm::vec4 color;
		for (int i = 0; i < info->N; i++) //i - номер массива сканов по дальности
		{
			float a = rdrinit->begAzm + rdrinit->dAzm *(info->d1 + i * (info->d2 - info->d1) / info->N);
			for (int j = 1; j < info->NR; j++) //j - номер отсчёта по дальности
			{				
				paletteIndex = min( (int) (paletteWidth * ((px[i * info->NR + j] - minAmp) / (maxAmp - minAmp))), paletteWidth-1);

				FreeImage_GetPixelColor(palette, paletteIndex, 0, &pixelcolor);
				color.x = pixelcolor.rgbRed / 255.0;
				color.y = pixelcolor.rgbGreen / 255.0;
				color.z = pixelcolor.rgbBlue / 255.0;
				color.w = 1;
				float r = (rdrinit->minR + j*(rdrinit->maxR - rdrinit->minR)/info->NR) * rdrinit->dR;
#if defined(CRCPOINT_CONSTRUCTOR_USES_RADIANS)

				cartesianCoords = glm::vec3(origin) + glm::vec3(-r * sin(a) * cos(e) / mpph, r * sin(e) / mppv, r * cos(a) * cos(e) / mpph); //we always add y0 (height of the radar relative to sea level) to all cartesian coordinates 
				vbuffer->push_back({ glm::vec4(cartesianCoords, 1), glm::vec3(0, 0, 0), color, glm::vec2(0, 0) });
#else
				float re = glm::radians(e);
				float ra = glm::radians(a);
				CartesianCoords = glm::vec3(-r * sin(ra) * cos(re) / mpph, y0 + r * sin(re) / mppv, r * cos(ra) * cos(re) / mpph);
#endif
			}
		}
		vbo.at(Main)->SetBuffer(vbuffer, &(*vbuffer)[0], vbuffer->size());
		vbo.at(MiniMap)->SetBuffer(vbuffer, &(*vbuffer)[0], vbuffer->size());
	}
}


CRImage::~CRImage()
{
	auto buffer = (vector<VBOData>*)vbo.at(Main)->GetBuffer();
	if (buffer)
	{
		delete buffer;
	}
}

FIBITMAP * CRImage::palette = nullptr;
int CRImage::paletteWidth = 0;
bool CRImage::InitPalette(std::string fileName)
{
	std::string context = "CRImage::InitPalette";
	palette = FreeImage_Load(FreeImage_GetFileType(fileName.c_str(), 0), fileName.c_str());
	if (!palette)
	{
		LOG_ERROR__("Unable to open CRImage palette file %s", fileName.c_str());
		return false;
	}
	paletteWidth = FreeImage_GetWidth(palette);
	minAmp = CSettings::GetFloat(FloatCRImageMinAmp);
	maxAmp = CSettings::GetFloat(FloatCRImageMaxAmp);
	return true;
}
