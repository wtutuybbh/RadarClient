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
const std::string CRImage::requestID = "C3DObjectModel";
CRImage::CRImage(float azemuth, glm::vec4 origin, float mpph, float mppv, RDR_INITCL * rdrinit, RIMAGE* info, void* pixels): CSector(azemuth)
{
	c3DObjectModel_TypeName = "CRImage";

	PointSize = 5;

	Refresh(azemuth, origin, mpph, mppv, rdrinit, info, pixels);
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

void CRImage::Refresh(float azemuth, glm::vec4 origin, float mpph, float mppv, RDR_INITCL* rdrinit, RIMAGE* info, void* pixels)
{
	start_tick_ = GetTickCount();
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
		if (!vertices)
			vertices = std::make_shared<C3DObjectVertices>(info->N * info->NR);

		int v = 0;

		float e = glm::radians(CSettings::GetFloat(FloatZeroElevation));
		int paletteIndex;
		RGBQUAD pixelcolor;
		glm::vec4 color;
		for (int i = 0; i < info->N; i++) //i - номер массива сканов по дальности
		{
			float a = rdrinit->begAzm + rdrinit->dAzm *(info->d1 + i * (info->d2 - info->d1) / info->N);
			for (int j = 1; j < info->NR; j++) //j - номер отсчёта по дальности
			{
				paletteIndex = min((int)(paletteWidth * ((px[i * info->NR + j] - minAmp) / (maxAmp - minAmp))), paletteWidth - 1);
				
					
				if (px[i * info->NR + j] > maxAmp)
				{
					maxAmp = px[i * info->NR + j];
				}
				if (px[i * info->NR + j] < minAmp)
				{
					minAmp = px[i * info->NR + j];
				}
				FreeImage_GetPixelColor(palette, paletteIndex, 0, &pixelcolor);
				color.x = pixelcolor.rgbRed / 255.0;
				color.y = pixelcolor.rgbGreen / 255.0;
				color.z = pixelcolor.rgbBlue / 255.0;
				color.w = 1;
				float r = (rdrinit->minR + j*(rdrinit->maxR - rdrinit->minR) / info->NR) * rdrinit->dR;
				/*if (px[i * info->NR + j] == 0)
					r = 0;*/
#if defined(CRCPOINT_CONSTRUCTOR_USES_RADIANS)

				cartesianCoords = glm::vec3(origin) + glm::vec3(-r * sin(a) * cos(e) / mpph, r * sin(e) / mppv, r * cos(a) * cos(e) / mpph); //we always add y_0 (height of the radar relative to sea level) to all cartesian coordinates 

				vertices.get()->SetValues(v, glm::vec4(cartesianCoords, 1), glm::vec3(0, 0, 0), color, glm::vec2(0, 0));

				v++;
#else
				float re = glm::radians(e);
				float ra = glm::radians(a);
				CartesianCoords = glm::vec3(-r * sin(ra) * cos(re) / mpph, y_0 + r * sin(re) / mppv, r * cos(ra) * cos(re) / mpph);
#endif
			}
		}
		if (!vbo.at(Main)->vertices)
			vbo.at(Main)->vertices = vertices;
		if (!vbo.at(MiniMap)->vertices)
			vbo.at(MiniMap)->vertices = vertices;
		vertices.get()->usesCount = 2;
		/*vbo.at(Main)->NeedsReload = true;
		vbo.at(MiniMap)->NeedsReload = true;*/
	}
}

void CRImage::BindUniforms(CViewPortControl* vpControl)
{

	CSector::BindUniforms(vpControl);

}
