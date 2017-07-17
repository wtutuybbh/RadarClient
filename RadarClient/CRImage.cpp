#include "stdafx.h"
#include "CRImage.h"
#include "CRCSocket.h"
#include "CSettings.h"
#include "C3DObjectVBO.h"
#include "C3DObjectProgram.h"
#include "C3DObjectTexture.h"
#include "CViewPortControl.h"
#include "CRCLogger.h"

float CRImage::maxAmp = 70;
float CRImage::minAmp = 0;
const std::string CRImage::requestID = "C3DObjectModel";

//DWORD CSector::prev_image_tick_ = 0;

void CRImage::_refresh_multipoints_2d(float azemuth, glm::vec4 origin, float mpph, float mppv, RDR_INITCL* rdrinit, RIMAGE* info, void* pixels)
{
	if (!vertices)
		vertices = std::make_shared<C3DObjectVertices>(info->N * info->NR);
	else
		vertices->ReCreate(info->N * info->NR);

	int v = 0;
	float *px = (float *)pixels;
	float e = glm::radians(CSettings::GetFloat(FloatZeroElevation));
	int paletteIndex;
	RGBQUAD pixelcolor;
	glm::vec4 color;

	DWORD curr_image_tick_ = GetTickCount();
	
	DWORD ticks_per_scan_ = (curr_image_tick_ - prev_image_tick_) / info->N;
	LOG_INFO("flight_test", "CRImage::_refresh_multipoints_2d", "ticks_per_scan=%d", ticks_per_scan_);

	double lon = 0, lat = 0;
	glm::vec3 flight_point, p0, p1;
	double threshold = 0.1;
	for (int i = 0; i < info->N; i++) //i - номер массива сканов по дальности
	{
		float a = rdrinit->begAzm + rdrinit->dAzm *(info->d1 + i * (info->d2 - info->d1) / info->N);

		// test flight code. 
		if (flight_start_)
		{
				auto t = prev_image_tick_+ i * ticks_per_scan_;
				CSector::get_flight_point(t, lon, lat);
				//LOG_INFO("flight_test", "CRImage::_refresh_multipoints_2d", "t=%d, lon=%f, lat=%f", t, lon, lat);
				if (mesh) 
				{
					flight_point = glm::vec3(mesh->LLH2XYZT(glm::vec3(lon, lat, 0)));
					//LOG_INFO("flight_test", "CRImage::_refresh_multipoints_2d", "flight_point={%f, %f, %f}", flight_point.x, flight_point.y, flight_point.z);
				}
		}
		p0 = glm::vec3(origin) + glm::vec3(-rdrinit->minR * sin(a) * cos(e) / mpph, rdrinit->minR * sin(e) / mppv, rdrinit->minR * cos(a) * cos(e) / mpph);
		p1 = glm::vec3(origin) + glm::vec3(-rdrinit->maxR * sin(a) * cos(e) / mpph, rdrinit->maxR * sin(e) / mppv, rdrinit->maxR * cos(a) * cos(e) / mpph);

		auto d01 = glm::distance(p0, p1);
		auto d0 = glm::distance(p0, flight_point);
		auto d1 = glm::distance(p1, flight_point);

		int jx = -1;

		if (d0 + d1 - d01 < 1 )
		{
			jx = int(info->NR * d0 / (d0 + d1));
		}

		LOG_INFO("flight_test", "CRImage::_refresh_multipoints_2d", "distance test result=%f", d0 + d1 - d01);

		

		for (int j = 1; j < info->NR; j++) //j - номер отсчёта по дальности
		{
			//paletteIndex = min((int)(paletteWidth * ((px[i * info->NR + j] - minAmp) / (maxAmp - minAmp))), paletteWidth - 1);
			
			if (px[i * info->NR + j] > maxAmp)
			{
				maxAmp = px[i * info->NR + j];
				//if (maxAmp > 4095) maxAmp = 4095;
			}
			if (px[i * info->NR + j] < minAmp)
			{
				minAmp = px[i * info->NR + j];
			}
			/*FreeImage_GetPixelColor(palette, paletteIndex, 0, &pixelcolor);
			color.r = pixelcolor.rgbRed / 255.0;
			color.g = pixelcolor.rgbGreen / 255.0;
			color.b = pixelcolor.rgbBlue / 255.0;
			color.a =  5 * float(paletteIndex) / paletteWidth;*/

			color = GetColor(px[i * info->NR + j]);

			if (jx > 0 && abs(j - jx) <= 5)
			{
				//color = GetColor(maxAmp);
				color = glm::vec4(1, 1, 1, 1);
			}

			if (color.a > 1) color.a = 1;
			if (CRImage_Refresh_Log) LOG_INFO_("CRImage_Refresh_Log", "color.a=%f", color.a);
			float r = (rdrinit->minR + j*(rdrinit->maxR - rdrinit->minR) / info->NR) * rdrinit->dR;
			/*if (px[i * info->NR + j] == 0)
			r = 0;*/
#if defined(CRCPOINT_CONSTRUCTOR_USES_RADIANS)

			cartesianCoords = glm::vec3(origin) + glm::vec3(-r * sin(a) * cos(e) / mpph, r * sin(e) / mppv, r * cos(a) * cos(e) / mpph); //we always add y_0 (height of the radar relative to sea level) to all cartesian coordinates 

			
			/*if (flight_start_)
			{
				LOG_INFO("flight_test", "CRImage::_refresh_multipoints_2d", "t=%f", glm::distance(flight_point, cartesianCoords));
			}*/
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
	vertices->needsReload = 2;
	/*vbo.at(MiniMap)->NeedsReload = true;*/

	prev_image_tick_ = curr_image_tick_;
}

void CRImage::_refresh_textured_2d(float azemuth, glm::vec4 origin, float mpph, float mppv, RDR_INITCL* rdrinit, RIMAGE* info, void* pixels)
{
	int npoints = 4;

	if (!vertices)
		vertices = std::make_shared<C3DObjectVertices>(npoints);
	else
		vertices->ReCreate(npoints);

	float *px = (float *)pixels;
	float e = glm::radians(CSettings::GetFloat(FloatZeroElevation));
	int paletteIndex;
	RGBQUAD pixelcolor;
	glm::vec4 color;

	auto maxtc = 0.999;

	auto r = rdrinit->minR;
	auto a = rdrinit->begAzm + rdrinit->dAzm *info->d1;

	auto coords = glm::vec3(origin) + glm::vec3(-r * sin(a) * cos(e) / mpph, r * sin(e) / mppv, r * cos(a) * cos(e) / mpph);
	vertices->SetValues(0, glm::vec4(coords, 1), glm::vec3(0, 0, 0), glm::vec4(1, 0, 0, 1), glm::vec2(0.0, 0.0));

	r = rdrinit->maxR;

	coords = glm::vec3(origin) + glm::vec3(-r * sin(a) * cos(e) / mpph, r * sin(e) / mppv, r * cos(a) * cos(e) / mpph);
	vertices->SetValues(1, glm::vec4(coords, 1), glm::vec3(0, 0, 0), glm::vec4(0, 0, 1, 1), glm::vec2(maxtc, 0.0));
	//vertices->SetValues(4, glm::vec4(coords, 1), glm::vec3(0, 0, 0), glm::vec4(0, 0, 1, 1), glm::vec2(1.0, 0.0));

	r = rdrinit->minR;
	a = rdrinit->begAzm + rdrinit->dAzm *info->d2;
	coords = glm::vec3(origin) + glm::vec3(-r * sin(a) * cos(e) / mpph, r * sin(e) / mppv, r * cos(a) * cos(e) / mpph);
	//vertices->SetValues(2, glm::vec4(coords, 1), glm::vec3(0, 0, 0), glm::vec4(1, 0, 0, 1), glm::vec2(0.0, 1.0));
	//vertices->SetValues(3, glm::vec4(coords, 1), glm::vec3(0, 0, 0), glm::vec4(1, 0, 0, 1), glm::vec2(0.0, 1.0));
	vertices->SetValues(3, glm::vec4(coords, 1), glm::vec3(0, 0, 0), glm::vec4(1, 0, 0, 1), glm::vec2(maxtc, maxtc));

	r = rdrinit->maxR;
	coords = glm::vec3(origin) + glm::vec3(-r * sin(a) * cos(e) / mpph, r * sin(e) / mppv, r * cos(a) * cos(e) / mpph);
	//vertices->SetValues(5, glm::vec4(coords, 1), glm::vec3(0, 0, 0), glm::vec4(0, 0, 1, 1), glm::vec2(1.0, 1.0));
	vertices->SetValues(2, glm::vec4(coords, 1), glm::vec3(0, 0, 0), glm::vec4(1, 0, 0, 1), glm::vec2(0.0, maxtc));

	auto texbuffer = tex.at(Main)->GetBytes();
	if (!texbuffer)
	{
		texbuffer = new unsigned char[info->N * (info->NR - 1) * 4];
		tex.at(Main)->SetBytes(texbuffer, info->NR - 1, info->N);
		tex.at(MiniMap)->SetBytes(texbuffer, info->NR - 1, info->N);
	}

	for (auto r = 0; r<info->N; r++)
	{
		for (auto c = 0; c < info->NR - 1; c++)
		{
			color = GetColor(px[r * (info->NR - 1) + c]);
			texbuffer[r * (info->NR - 1) + c] = color.r * 255;
			texbuffer[r * (info->NR - 1) + c + 1] = color.g * 255;
			texbuffer[r * (info->NR - 1) + c + 2] = color.b * 255;
			texbuffer[r * (info->NR - 1) + c + 3] = 255;// color.a * 255;
		}
	}

	if (!vbo.at(Main)->vertices)
		vbo.at(Main)->vertices = vertices;
	if (!vbo.at(MiniMap)->vertices)
		vbo.at(MiniMap)->vertices = vertices;
	vertices.get()->usesCount = 2;
	vertices->needsReload = 2;
}

CRImage::CRImage(float azemuth, glm::vec4 origin, float mpph, float mppv, RDR_INITCL * rdrinit, RIMAGE* info, void* pixels): CSector(azemuth)
{
	c3DObjectModel_TypeName = "CRImage";

	PointSize = 5;

	//DrawMode = GL_TRIANGLES;
	//DrawMode = GL_QUADS;
	DrawMode = GL_POINTS;

	//prog.at(Main)->SetNames("CRImage.v.glsl", "CRImage.f.glsl", "vertex", "texcoor", nullptr, nullptr, nullptr);
	//prog.at(MiniMap)->SetNames("CRImage.v.glsl", "CRImage.f.glsl", "vertex", "texcoor", nullptr, nullptr, nullptr);

	/*tex.insert_or_assign(Main, new C3DObjectTexture(nullptr, "tex", false, true));
	tex.insert_or_assign(MiniMap, new C3DObjectTexture(nullptr, "tex", false, true));
*/

	//tex.insert_or_assign(Main, new C3DObjectTexture("testtex.png", "tex"));
	//tex.insert_or_assign(MiniMap, new C3DObjectTexture("testtex.png", "tex"));

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
	

	short currentDirection = sgn(info->d2 - info->d1);

	//glm::vec4 mincolor = CSettings::GetColor(ColorPointLowLevel);
	//glm::vec4 maxcolor = CSettings::GetColor(ColorPointHighLevel);

	if (rdrinit->ScanMode == 3) // 3D
	{

	}
	else // 2D
	{
		_refresh_multipoints_2d(azemuth, origin, mpph, mppv, rdrinit, info, pixels);
		//_refresh_textured_2d(azemuth, origin, mpph, mppv, rdrinit, info, pixels);
		
	}
}

void CRImage::BindUniforms(CViewPortControl* vpControl)
{

	CSector::BindUniforms(vpControl);

}
