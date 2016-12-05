#include "stdafx.h"

#include "CSector.h"
#include "CRCSocket.h"
#include "CSettings.h"
#include "CViewPortControl.h"
#include "C3DObjectVBO.h"
#include "C3DObjectProgram.h"
#include "CRCLogger.h"

const std::string CSector::requestID = "CSector";

CSector::CSector(int index) : C3DObjectModel(Main, new C3DObjectVBO(false), nullptr, new C3DObjectProgram("CSector.v.glsl", "CSector.f.glsl", "vertex", nullptr, nullptr, "color"))
{
	this->index = index;

	vbo.insert_or_assign(MiniMap, new C3DObjectVBO(false));
	tex.insert_or_assign(MiniMap, nullptr);
	prog.insert_or_assign(MiniMap, new C3DObjectProgram("CSector.v.glsl", "CSector.f.glsl", "vertex", nullptr, nullptr, "color"));

	translateMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
	scaleMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
	rotateMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
}


CSector::~CSector()
{
	C3DObjectVBO* _vbo;
	try
	{
		_vbo = (C3DObjectVBO*)vbo.at(Main);
		if (_vbo)
		{
			auto _b = _vbo->GetBuffer();
			if (_b)
			{
				delete _b;
			}
			else
			{
				CRCLogger::Warn(requestID, "CSector DESTRUCTOR", "buffer is nullptr");
			}
		}
		else
		{
			CRCLogger::Warn(requestID, "CSector DESTRUCTOR", "vbo at Main is nullptr");
		}
	}
	catch (std::out_of_range ex)
	{
		CRCLogger::Warn(requestID, "CSector DESTRUCTOR", "no vbo at Main");
	}	
}

float CSector::maxAmp = 0;
void CSector::Refresh(glm::vec4 origin, float mpph, float mppv, RPOINTS* info_p, RPOINT* pts, RDR_INITCL* init)
{
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
	if (init->Nazm <= 0)
	{
		LOG_ERROR__("init->Nazm = %d", init->Nazm);
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
	vector<VBOData> *vbuffer = (vector<VBOData> *)GetBufferAt(Main);
	if (vbuffer) {
		vbuffer->clear();
		vbuffer->resize(info_p->N);
	}
	else {
		vbuffer = new vector<VBOData>(info_p->N);
	}
	float zeroElevation = glm::radians(CSettings::GetFloat(FloatZeroElevation));
	for (int i = 0; i < info_p->N; i++) 
	{
		//here we calculate point spherical coordinates
		r = pts[i].R * init->dR;
		a = init->begAzm + pts[i].B * init->dAzm;
		e = zeroElevation + init->begElv + pts[i].E * init->dElv;
		(*vbuffer)[i].vert = origin + glm::vec4(-r * sin(a) * cos(e) / mpph, r * sin(e) / mppv, r * cos(a) * cos(e) / mpph, 0);
		(*vbuffer)[i].norm.x = pts[i].Amp;		
		(*vbuffer)[i].color = GetColor(pts[i].Amp);		
		LOG_INFO__("index= %d, i= %d, R= %d, b= %d, E= %d, Amp= %f", index, i, pts[i].R, pts[i].B, pts[i].E, pts[i].Amp);
	}
	
	vbo.at(Main)->SetBuffer(vbuffer, &(*vbuffer)[0], vbuffer->size());
	vbo.at(Main)->NeedsReload = true;
	if (vbo.find(MiniMap) != vbo.end())
	{
		vbo.at(MiniMap)->SetBuffer(vbuffer, &(*vbuffer)[0], vbuffer->size());
		vbo.at(MiniMap)->NeedsReload = true;
	}
}

void CSector::Dump(CViewPortControl* vpControl, std::ofstream *outfile)
{
	C3DObjectVBO *vbo_ = vbo.at(vpControl->Id);
	vector<VBOData> *buffer = (vector<VBOData> *)vbo_->GetBuffer();
	for (auto it = buffer->begin(); it != buffer->end(); ++it)
	{
		*outfile << (*it).norm.x << char(13) << std::endl;
	}
}

void CSector::BindUniforms(CViewPortControl* vpControl)
{
	glm::mat4 m = GetModelMatrix(vpControl);
	glm::mat4 v = vpControl->GetViewMatrix();
	glm::mat4 p = vpControl->GetProjMatrix();
	glm::mat4 mvp = p*v*m;
	int mvp_loc = prog.at(vpControl->Id)->GetUniformLocation("mvp");
	glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, glm::value_ptr(mvp));

	int ps_loc = prog.at(vpControl->Id)->GetUniformLocation("pointSize");
	glUniform1fv(ps_loc, 1, &PointSize);
}

#define CSector_GetPoint_LogInfo false
int CSector::GetPoint(CViewPortControl* vpControl, glm::vec2 screenPoint)
{
	std::string context = "CSector::GetPoint";

	if (!vpControl)
	{
		CRCLogger::Error(requestID, context, (boost::format("vpControl is nullptr, screenPoint=(%1%, %2%)") % screenPoint.x % screenPoint.y).str());
		return -1;
	}

	if (CSector_GetPoint_LogInfo) 
		CRCLogger::Info(requestID, context, (boost::format("Start... vpControl.Id=%1%, screenPoint=(%2%, %3%)") % vpControl->Id % screenPoint.x % screenPoint.y).str());

	C3DObjectVBO *vbo_ = vbo.at(vpControl->Id);

	//so, it works only if buffer elements have datatype of VBOData (or identical) and organized using std::vector
	vector<VBOData> *buffer = (vector<VBOData> *)vbo_->GetBuffer();

	if (!buffer || buffer->size() == 0) 
	{
		if (!buffer)
		{
			CRCLogger::Error(requestID, context, "buffer is nullptr, RETURN -1");
		}
		if (buffer->size() == 0)
		{
			CRCLogger::Error(requestID, context, "buffer->size() == 0, RETURN -1");
		}
		return -1;
	}
	glm::mat4 mv = vpControl->GetViewMatrix() * GetModelMatrix(vpControl);	

	if (CSector_GetPoint_LogInfo)
	{
		CRCLogger::Info(requestID, context, (boost::format("mv[0]=(%1%)") % mat4row2str(mv, 0, 3)).str());
		CRCLogger::Info(requestID, context, (boost::format("mv[1]=(%1%)") % mat4row2str(mv, 1, 3)).str());
		CRCLogger::Info(requestID, context, (boost::format("mv[2]=(%1%)") % mat4row2str(mv, 2, 3)).str());
		CRCLogger::Info(requestID, context, (boost::format("mv[3]=(%1%)") % mat4row2str(mv, 3, 3)).str());
	}

	glm::mat4 proj = vpControl->GetProjMatrix();
	for (int i = 0; i < buffer->size(); i++)
	{
		glm::vec4 p = vpControl->GetProjMatrix() * vpControl->GetViewMatrix() * GetModelMatrix(vpControl) * (*buffer)[i].vert;
		p = p / p.w;
		glm::vec3 screenP = glm::project(
			glm::vec3((*buffer)[i].vert),
			mv,
			proj,
			glm::vec4(0, 0, vpControl->Width, vpControl->Height)
			);
		if (glm::distance(glm::vec2(screenP), glm::vec2(screenPoint.x, vpControl->Height - screenPoint.y - 1)) < PointSize)
		{
			return i;
		}
	}
	return -1;
}

glm::vec3 CSector::GetPointCoords(CViewPortControl* vpControl, int index)
{
	C3DObjectVBO *vbo_ = vbo.at(vpControl->Id);

	vector<VBOData> *buffer = (vector<VBOData> *)vbo_->GetBuffer();

	return glm::vec3(buffer->at(index).vert);
}

void CSector::SelectPoint(int vpId, int pointIndex)
{
	C3DObjectVBO *vbo_ = vbo.at(vpId);

	vector<VBOData> *buffer = (vector<VBOData> *)vbo_->GetBuffer();

	if (!buffer || buffer->size() == 0)
		return;
	
	(*buffer)[pointIndex].color = CSettings::GetColor(ColorPointSelected);
	vbo_->NeedsReload = true;
}

void CSector::UnselectAll(int vpId)
{
	C3DObjectVBO *vbo_ = vbo.at(vpId);

	vector<VBOData> *buffer = (vector<VBOData> *)vbo_->GetBuffer();

	if (!buffer || buffer->size() == 0)
		return;

	for (int i = 0; i < buffer->size(); i++)
	{
		buffer->at(i).color = GetColor(buffer->at(i).norm.x);
	}
	vbo_->NeedsReload = true;
}

glm::vec4 CSector::GetColor(float level)
{
	if (level < CSettings::GetInt(IntPointColorThreshold_00))
	{
		return CSettings::GetColor(ColorPointColor_00);
	}
	else if (level < CSettings::GetInt(IntPointColorThreshold_01))
	{
		return CSettings::GetColor(ColorPointColor_01);
	}
	else if (level < CSettings::GetInt(IntPointColorThreshold_02))
	{
		return CSettings::GetColor(ColorPointColor_02);
	}
	else
	{
		return CSettings::GetColor(ColorPointColor_03);
	}
}
