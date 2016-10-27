//#define WIN32_LEAN_AND_MEAN

//#include "stdafx.h"
#include "CRImage.h"
#include "CRCSocket.h"
#include "CSettings.h"
#include "C3DObjectVBO.h"
#include "C3DObjectProgram.h"
#include "CViewPortControl.h"

float CRImage::maxAmp = 1000;
CRImage::CRImage(float azemuth, glm::vec4 origin, float mpph, float mppv, RDR_INITCL * rdrinit, RIMAGE* info, void* pixels) : 
	C3DObjectModel(
		new C3DObjectVBO(true), 
		NULL, 
		new C3DObjectProgram("CRImage.v.glsl", "CRImage.f.glsl", "vertex", NULL, NULL, "color"))
{
	Azemuth = azemuth;
	float *px = (float *)pixels;

	short currentDirection = sgn(info->d2 - info->d1);

	glm::vec4 mincolor = CSettings::GetColor(ColorPointLowLevel);
	glm::vec4 maxcolor = CSettings::GetColor(ColorPointHighLevel);

	if (rdrinit->ScanMode == 3) // 3D
	{

	}
	else // 2D
	{
		vector<VBOData> *vbuffer =  new vector<VBOData>;

		float e = glm::radians(CSettings::GetFloat(FloatZeroElevation));
		for (int i = 0; i < info->N; i++) //i - номер массива сканов по дальности
		{
			float a = rdrinit->begAzm + rdrinit->dAzm *(info->d1 + i * (info->d2 - info->d1) / info->N);
			for (int j = 1; j < info->NR; j++) //j - номер отсчёта по дальности
			{
				/*if (px[i * info->NR + j] > maxAmp)
					maxAmp = px[i * info->NR + j];	*/				
				
				float r = (rdrinit->minR + j*(rdrinit->maxR - rdrinit->minR)/info->NR) * rdrinit->dR;
#if defined(CRCPOINT_CONSTRUCTOR_USES_RADIANS)

				cartesianCoords = glm::vec3(origin) + glm::vec3(-r * sin(a) * cos(e) / mpph, r * sin(e) / mppv, r * cos(a) * cos(e) / mpph); //we always add y0 (height of the radar relative to sea level) to all cartesian coordinates 
				vbuffer->push_back({ glm::vec4(cartesianCoords, 1), glm::vec3(0, 0, 0), mincolor + px[i * info->NR + j] / maxAmp * (maxcolor - mincolor), glm::vec2(0, 0) });
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
		delete buffer;
}


