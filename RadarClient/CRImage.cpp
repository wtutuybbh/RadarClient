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

CRImage::~CRImage()
{
	if (texData)
		delete[] texData;
}

CRImage::CRImage(float azemuth, glm::vec4 origin, float mpph, float mppv, RDR_INITCL * rdrinit, RIMAGE* info, void* pixels): CSector(azemuth)
{
	this->index = index;

	tex.insert_or_assign(Main, new C3DObjectTexture("tex"));
	tex.at(Main)->SetFormatsAndType(GL_R16F, GL_RED, GL_FLOAT);
	prog.at(Main)->SetNames("CRImage.v.glsl", "CRImage.f.glsl", "vertex", "texcoor", nullptr, "color");

	tex.insert_or_assign(MiniMap, new C3DObjectTexture("tex"));
	tex.at(MiniMap)->SetFormatsAndType(GL_R16F, GL_RED, GL_FLOAT);
	prog.at(MiniMap)->SetNames("CRImage.v.glsl", "CRImage.f.glsl", "vertex", "texcoor", nullptr, "color");


	c3DObjectModel_TypeName = "CRImage";

	PointSize = 1;

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
			vertices = std::make_shared<C3DObjectVertices>(info->N * 6);

		int v = 0;

		float e = glm::radians(CSettings::GetFloat(FloatZeroElevation));
		int paletteIndex;
		RGBQUAD pixelcolor;
		glm::vec4 color;

		texSize = info->N * (info->NR - 1);
		texData = new float[texSize];
		

		auto rmin = rdrinit->minR;
		auto rmax = rdrinit->maxR;

		auto pxstep = rdrinit->dAzm;
		auto vrt = vertices.get();

		glm::vec3 tmppoint;
		float a;
		for (int i = 0; i < info->N; i++) //i - номер массива сканов по дальности
		{
			
			
			memcpy(&(texData[i * (info->NR-1)]), &(px[i * info->NR + 1]), (info->NR - 1)*sizeof(float));
			
			a = rdrinit->begAzm + rdrinit->dAzm *(info->d1 + (i-0.5) * (info->d2 - info->d1) / info->N);
			tmppoint = glm::vec3(origin) + glm::vec3(-rmin * sin(a) * cos(e) / mpph, rmin * sin(e) / mppv, rmin * cos(a) * cos(e) / mpph); 
			vrt->SetValues(i * 6, glm::vec4(tmppoint, 1), glm::vec3(0, 0, 0), glm::vec4(0, 0, 0, 0), glm::vec2(0, i / info->N));
			
			tmppoint = glm::vec3(origin) + glm::vec3(-rmax * sin(a) * cos(e) / mpph, rmax * sin(e) / mppv, rmax * cos(a) * cos(e) / mpph);
			vrt->SetValues(i * 6 + 1, glm::vec4(tmppoint, 1), glm::vec3(0, 0, 0), glm::vec4(0, 0, 0, 0), glm::vec2(1, i / info->N));
			vrt->SetValues(i * 6 + 4, glm::vec4(tmppoint, 1), glm::vec3(0, 0, 0), glm::vec4(0, 0, 0, 0), glm::vec2(1, i / info->N));
			
			a = rdrinit->begAzm + rdrinit->dAzm *(info->d1 + (i + 0.5) * (info->d2 - info->d1) / info->N);
			tmppoint = glm::vec3(origin) + glm::vec3(-rmin * sin(a) * cos(e) / mpph, rmin * sin(e) / mppv, rmin * cos(a) * cos(e) / mpph);
			vrt->SetValues(i * 6 + 2, glm::vec4(tmppoint, 1), glm::vec3(0, 0, 0), glm::vec4(0, 0, 0, 0), glm::vec2(0, (i + 1) / info->N));
			vrt->SetValues(i * 6 + 3, glm::vec4(tmppoint, 1), glm::vec3(0, 0, 0), glm::vec4(0, 0, 0, 0), glm::vec2(0, (i + 1) / info->N));

			tmppoint = glm::vec3(origin) + glm::vec3(-rmax * sin(a) * cos(e) / mpph, rmax * sin(e) / mppv, rmax * cos(a) * cos(e) / mpph);
			vrt->SetValues(i * 6 + 5, glm::vec4(tmppoint, 1), glm::vec3(0, 0, 0), glm::vec4(0, 0, 0, 0), glm::vec2(1, (i + 1) / info->N));


			
		}
		for (auto f=0; f<texSize; f++)
		{
			texData[f] = 1;
		}
		tex.at(Main)->Reload(texData, info->NR - 1, info->N);
		tex.at(MiniMap)->Reload(texData, info->NR - 1, info->N);

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

	glm::mat4 m = GetModelMatrix(vpControl->Id);
	glm::mat4 v = vpControl->GetViewMatrix();
	glm::mat4 p = vpControl->GetProjMatrix();
	glm::mat4 mvp = p*v*m;
	int mvp_loc = prog.at(vpControl->Id)->GetUniformLocation("mvp");
	glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, glm::value_ptr(mvp));

	int ps_loc = prog.at(vpControl->Id)->GetUniformLocation("pointSize");
	glUniform1fv(ps_loc, 1, &PointSize);

	float alpha = min(max(1 - (1 - residual_alpha_) * (GetTickCount() - start_tick_) / 1000 / lifetime, residual_alpha_), 1);
	int alpha_loc = prog.at(vpControl->Id)->GetUniformLocation("alpha");
	glUniform1fv(alpha_loc, 1, &alpha);

}
