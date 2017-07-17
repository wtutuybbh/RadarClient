#include "stdafx.h"

#include "CSector.h"
#include "CRCSocket.h"
#include "CSettings.h"
#include "CViewPortControl.h"
#include "C3DObjectVBO.h"
#include "C3DObjectProgram.h"
#include "CRCLogger.h"

const std::string CSector::requestID = "CSector";

float CSector::maxAmp = 70;
float CSector::minAmp = 0;

float CSector::x0 = 0;
float CSector::f0 = 0;
float CSector::x1 = 0;
float CSector::f1 = 0;
float CSector::x2 = 0;
float CSector::f2 = 0;

CFlightPoint::CFlightPoint(double lon, double lat, int dir, int time) : lon(lon), lat(lat), dir(dir), time(time)
{
}

void CSector::get_flight_point(DWORD t, double& lon, double& lat)
{
	auto t_ = (t - flight_start_) / 1000.0f + flight_time0;
	int i0=0, i1=0;


	for (auto i=0; i<flight_points_.size(); i++)
	{
		if (flight_points_[i].time > t_)
		{
			i0 = i - 1;
			i1 = i;
			break;
		}
	}
	auto p0_x = flight_points_[i0].lon;
	auto p0_y = flight_points_[i0].lat;
	auto p1_x = flight_points_[i1].lon;
	auto p1_y = flight_points_[i1].lat;

	auto m0_x = sin(2 * M_PI * flight_points_[i0].dir / 360.0);
	auto m0_y = cos(2 * M_PI * flight_points_[i0].dir / 360.0);
	auto m1_x = sin(2 * M_PI * flight_points_[i1].dir / 360.0);
	auto m1_y = cos(2 * M_PI * flight_points_[i1].dir / 360.0);

	auto sqrt01 = sqrt((p1_x - p0_x) * (p1_x - p0_x) + (p1_y - p0_y) * (p1_y - p0_y));

	m0_x *= sqrt01;
	m0_y *= sqrt01;
	m1_x *= sqrt01;
	m1_y *= sqrt01;

	auto t__ = (t_ - flight_points_[i0].time) / (flight_points_[i1].time - flight_points_[i0].time);
	auto t__2 = t__ * t__;
	auto t__3 = t__2 * t__;

	lon = (2 * t__3 - 3 * t__2 + 1)*p0_x + (t__3 - 2 * t__2 + t__)*m0_x + (-2 * t__3 + 3 * t__2)*p1_x + (t__3 - t__2)*m1_x;
	lat = (2 * t__3 - 3 * t__2 + 1)*p0_y + (t__3 - 2 * t__2 + t__)*m0_y + (-2 * t__3 + 3 * t__2)*p1_y + (t__3 - t__2)*m1_y;
}

//DWORD CSector::flight_start_ = 0;
void CSector::flight_start_stop(bool start)
{
	if (start)
	{
		flight_start_ = GetTickCount();
	}
	else
	{
		flight_start_ = 0;
	}
}

void CSector::RefreshColorSettings()
{
	x0 = CSettings::GetFloat(FloatAmp_00);
	f0 = CSettings::GetFloat(FloatAmpPalettePosition_00);
	x1 = CSettings::GetFloat(FloatAmp_01);
	f1 = CSettings::GetFloat(FloatAmpPalettePosition_01);
	x2 = CSettings::GetFloat(FloatAmp_02);
	f2 = CSettings::GetFloat(FloatAmpPalettePosition_02);
}

CSector::CSector(int index) : C3DObjectModel()
{
	this->index = index;

	vbo.insert_or_assign(Main, new C3DObjectVBO(false));
	tex.insert_or_assign(Main, nullptr);
	prog.insert_or_assign(Main, new C3DObjectProgram("CSector.v.glsl", "CSector.f.glsl", "vertex", nullptr, nullptr, "color"));
	translateMatrix.insert_or_assign(Main, glm::mat4(1.0f));
	scaleMatrix.insert_or_assign(Main, glm::mat4(1.0f));
	rotateMatrix.insert_or_assign(Main, glm::mat4(1.0f));

	vbo.insert_or_assign(MiniMap, new C3DObjectVBO(false));
	tex.insert_or_assign(MiniMap, nullptr);
	prog.insert_or_assign(MiniMap, new C3DObjectProgram("CSector.v.glsl", "CSector.f.glsl", "vertex", nullptr, nullptr, "color"));
	translateMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
	scaleMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
	rotateMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));

	c3DObjectModel_TypeName = "CSector";

	RefreshColorSettings();
}

void CSector::Refresh(glm::vec4 origin, float mpph, float mppv, RPOINTS* info_p, RPOINT* pts, RDR_INITCL* init)
{

	start_tick_ = GetTickCount();
	std::lock_guard<std::mutex> lock(m);
	std::string context = "CSector::Refresh";

	if (!info_p)
	{
		LOG_ERROR__("info_p is nullptr");
		return;
	}
	if (!pts)
	{
		LOG_ERROR__("pts is nullptr");
		return;
	}
	if (!init)
	{
		LOG_ERROR__("init is nullptr");
		return;
	}
	if (init->MaxNAzm <= 0)
	{
		LOG_ERROR__("init->MaxNAzm = %d", init->MaxNAzm);
		return;
	}
	if (info_p->N <= 0)
	{
		LOG_ERROR__("info_p->N = %d", info_p->N);
		return;
	}

	//TODO: full defence against bad data need to be there


	//mpph, mppv, pts[i].R * init->dR, init->begAzm + pts[i].B * init->dAzm, ZERO_ELEVATION + init->begElv + pts[i].E * init->dElv
	float r, a, e;
	if (!vertices && info_p->N > 0)
		vertices = std::make_shared<C3DObjectVertices>(info_p->N);
	if (!vertices)
		return;
	float zeroElevation = glm::radians(CSettings::GetFloat(FloatZeroElevation));
	float maxDistance = CSettings::GetFloat(FloatMaxDistance);

	if (CSectorRefreshLogEnabled) LOG_INFO__(".index= %f, N= %d", index, info_p->N);

	for (int i = 0; i < info_p->N; i++) 
	{
		//here we calculate point spherical coordinates
		r = pts[i].R * init->dR;
		if (r>=0 && r <= maxDistance) {
			a = init->begAzm + pts[i].B * init->dAzm;
			e = zeroElevation + init->begElv + pts[i].E * init->dElv;
			auto c = GetColor(pts[i].Amp);
			//auto c = glm::vec4(1,0,0,1);
			vertices.get()->SetValues(i, origin + glm::vec4(-r * sin(a) * cos(e) / mpph, r * sin(e) / mppv, r * cos(a) * cos(e) / mpph, 0), glm::vec3(pts[i].Amp, 0, 0), c, glm::vec2(0, 0));

			//if (CSectorRefreshLogEnabled) LOG_INFO__(".index= %f, i= %d, R= %d, b= %d, E= %d, Amp= %f", index, i, pts[i].R, pts[i].B, pts[i].E, pts[i].Amp);
		}
		else
		{
			if (CSectorRefreshLogEnabled) LOG_INFO__(".index= %f, i= %d, R= %d, b= %d, E= %d, Amp= %f  -- WTF???", index, i, pts[i].R, pts[i].B, pts[i].E, pts[i].Amp);
		}
	}
	
	vertices.get()->needsReload = 2;

	if (!vbo.at(Main)->vertices)
		vbo.at(Main)->vertices = vertices;
	if (!vbo.at(MiniMap)->vertices)
		vbo.at(MiniMap)->vertices = vertices;

	vertices.get()->usesCount = 2;
}

void CSector::Dump(CViewPortControl* vpControl, std::ofstream *outfile)
{

}

void CSector::BindUniforms(CViewPortControl* vpControl)
{
	C3DObjectModel::BindUniforms(vpControl);
	
	if (vpControl->Id == Main)
	{
		PointSize = 3;
	}
	if (vpControl->Id == MiniMap)
	{
		PointSize = 1;
	}

	int ps_loc = prog.at(vpControl->Id)->GetUniformLocation("pointSize");
	glUniform1fv(ps_loc, 1, &PointSize);	

	int alpha_loc = prog.at(vpControl->Id)->GetUniformLocation("alpha");
	
	float a;
	
	switch (alpha_behaviour)
	{
	case Constant:
		a = alpha;
		break;
	case FadeOut:
		a = min(max(1 - (1 - residual_alpha_) * (GetTickCount() - start_tick_) / 1000 / lifetime, residual_alpha_), 1);
		break;
	default:
		break;
	}
	
	
	glUniform1fv(alpha_loc, 1, &a);

	/*int useUniColor_loc = prog.at(vpControl->Id)->GetUniformLocation("useUniColor");
	float useUniColor = 0.0f;
	glUniform1fv(ps_loc, 1, &useUniColor);

	int useUniColor_loc = prog.at(vpControl->Id)->GetUniformLocation("useUniColor");
	float useUniColor = 0.0f;
	glUniform1fv(ps_loc, 1, &useUniColor);*/
}


int CSector::GetPoint(CViewPortControl* vpControl, glm::vec2 screenPoint)
{
	std::string context = "CSector::GetPoint";

	if (!vpControl)
	{
		LOG_ERROR(requestID, context, (boost::format("vpControl is nullptr, screenPoint=(%1%, %2%)") % screenPoint.x % screenPoint.y).str().c_str());
		return -1;
	}

	if (CSector_GetPoint_LogInfo) 
		LOG_INFO(requestID, context, (boost::format("Start... vpControl.Id=%1%, screenPoint=(%2%, %3%)") % vpControl->Id % screenPoint.x % screenPoint.y).str().c_str());

	C3DObjectVBO *vbo_ = vbo.at(vpControl->Id);

	//so, it works only if buffer elements have datatype of VBOData (or identical) and organized using std::vector

	glm::mat4 mv = vpControl->GetViewMatrix() * GetModelMatrix(vpControl->Id);	

	if (CSector_GetPoint_LogInfo)
	{
		LOG_INFO(requestID, context, (boost::format("mv[0]=(%1%)") % mat4row2str(mv, 0, 3)).str().c_str());
		LOG_INFO(requestID, context, (boost::format("mv[1]=(%1%)") % mat4row2str(mv, 1, 3)).str().c_str());
		LOG_INFO(requestID, context, (boost::format("mv[2]=(%1%)") % mat4row2str(mv, 2, 3)).str().c_str());
		LOG_INFO(requestID, context, (boost::format("mv[3]=(%1%)") % mat4row2str(mv, 3, 3)).str().c_str());
	}

	glm::mat4 proj = vpControl->GetProjMatrix();
	if (!vertices)
		return -1;
	for (int i = 0; i < vertices.get()->vertexCount; i++)
	{
		glm::vec4 p = vpControl->GetProjMatrix() * vpControl->GetViewMatrix() * GetModelMatrix(vpControl->Id) * *vertices.get()->getv(i);
		p = p / p.w;
		glm::vec3 screenP = glm::project(
			glm::vec3(*vertices.get()->getv(i)),
			mv,
			proj,
			glm::vec4(0, 0, vpControl->GetWidth(), vpControl->GetHeight())
			);
		if (glm::distance(glm::vec2(screenP), glm::vec2(screenPoint.x, vpControl->GetHeight() - screenPoint.y - 1)) < PointSize)
		{
			return i;
		}
	}
	return -1;
}

glm::vec3 CSector::GetPointCoords(CViewPortControl* vpControl, int index)
{
	return glm::vec3(*vertices.get()->getv(index));
}

void CSector::SelectPoint(int vpId, int pointIndex)
{
	if (!vertices)
		return;

	auto vbuffer = vertices.get()->GetBuffer();
	auto vertexSize = vertices.get()->vertexSize;
	auto vertexCount = vertices.get()->vertexCount;

	if (!vbuffer || vertexCount == 0)
		return;
	
	auto color = CSettings::GetColor(ColorPointSelected);
	vbuffer[pointIndex * vertexSize + 7] = color.r;
	vbuffer[pointIndex * vertexSize + 8] = color.g;
	vbuffer[pointIndex * vertexSize + 9] = color.b;
	vbuffer[pointIndex * vertexSize + 10] = color.a;
	
	vertices.get()->needsReload = 2;
}

void CSector::UnselectAll(int vpId)
{
	if (!vertices)
		return;

	auto vbuffer = vertices.get()->GetBuffer();
	auto vertexSize = vertices.get()->vertexSize;
	auto vertexCount = vertices.get()->vertexCount;

	if (!vbuffer || vertexCount == 0)
		return;

	for (int i = 0; i < vertexCount; i++)
	{
		auto color = GetColor(vbuffer[i * vertexSize + 4]);
		vbuffer[i * vertexSize + 7] = color.r;
		vbuffer[i * vertexSize + 8] = color.g;
		vbuffer[i * vertexSize + 9] = color.b;
		vbuffer[i * vertexSize + 10] = color.a;
	}

	vertices.get()->needsReload = 2;
}

glm::vec4 CSector::GetColor(float level)
{
	RGBQUAD pixelcolor;
	glm::vec4 color;

	float b0, b1, b2;
	
	

	b0 = f0;

	b1 = (f1 - f0) / (x1 - x0);

	b2 = (((f2 - f1) / (x2 - x1)) - ((f1 - f0) / (x1 - x0))) / (x2 - x0);

	auto f = b0 + b1*(level - x0) + b2*(level - x0)*(level - x1);


	int paletteIndex = min((int)(paletteWidth * ((f - 0) / (1 - 0))), paletteWidth-1);

	FreeImage_GetPixelColor(palette, paletteIndex, 0, &pixelcolor);
	color.r = pixelcolor.rgbRed / 255.0;
	color.g = pixelcolor.rgbGreen / 255.0;
	color.b = pixelcolor.rgbBlue / 255.0;
	color.a = 5 * float(paletteIndex) / paletteWidth;
	if (color.a > 1) color.a = 1;
	if (color.a < 0) color.a = 0;
	return color;
}

FIBITMAP * CSector::palette = nullptr;
int CSector::paletteWidth = 0;

bool CSector::InitPalette(std::string fileName)
{
	std::string context = "CSector::InitPalette";
	if (palette)
		FreeImage_Unload(palette);
	palette = FreeImage_Load(FreeImage_GetFileType(fileName.c_str(), 0), fileName.c_str());
	if (!palette)
	{
		LOG_ERROR__("Unable to open CSector palette file %s", fileName.c_str());
		return false;
	}
	paletteWidth = FreeImage_GetWidth(palette);
	minAmp = CSettings::GetFloat(FloatCSectorMinAmp);
	maxAmp = CSettings::GetFloat(FloatCSectorMaxAmp);
	return true;
}
