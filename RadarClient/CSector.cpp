//#include "stdafx.h"
#include "CSector.h"
#include "CSettings.h"


CSector::CSector() : C3DObjectModel(Main, new C3DObjectVBO(false), nullptr, new C3DObjectProgram("CSector.v.glsl", "CSector.f.glsl", "vertex", nullptr, nullptr, "color"))
{
	vbo.insert_or_assign(MiniMap, new C3DObjectVBO(false));
	tex.insert_or_assign(MiniMap, nullptr);
	prog.insert_or_assign(MiniMap, new C3DObjectProgram("CSector.v.glsl", "CSector.f.glsl", "vertex", nullptr, nullptr, "color"));

	translateMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
	scaleMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
	rotateMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
}


CSector::~CSector()
{
	delete (vector<VBOData>*)vbo.at(Main)->GetBuffer();

	vbo.clear();
	tex.clear();
	prog.clear();
}

float CSector::maxAmp = 0;
void CSector::Refresh(glm::vec4 origin, float mpph, float mppv, RPOINTS* info_p, RPOINT* pts, RDR_INITCL* init)
{
	if (!info_p || !pts || !init)
		return; //do nothing if no RPOINTS structure provided
	if (init->Nazm <= 0 || info_p->N <= 0) //TODO: full defence against bad data need to be there
		return;
	//mpph, mppv, pts[i].R * init->dR, init->begAzm + pts[i].B * init->dAzm, ZERO_ELEVATION + init->begElv + pts[i].E * init->dElv
	float r, a, e;
	vector<VBOData> *vbuffer = (vector<VBOData> *)vbo.at(Main)->GetBuffer();
	if (vbuffer) {
		vbuffer->clear();
		vbuffer->resize(info_p->N);
	}
	else {
		vbuffer = new vector<VBOData>(info_p->N);
	}
	glm::vec4 mincolor = CSettings::GetColor(ColorPointLowLevel);
	glm::vec4 maxcolor = CSettings::GetColor(ColorPointHighLevel);
	float zeroElevation = glm::radians(CSettings::GetFloat(FloatZeroElevation));
	for (int i = 0; i < info_p->N; i++) 
	{
		if (pts[i].Amp > maxAmp)
			maxAmp = pts[i].Amp;
		//here we calculate point spherical coordinates
		r = pts[i].R * init->dR;
		a = init->begAzm + pts[i].B * init->dAzm;
		e = zeroElevation + init->begElv + pts[i].E * init->dElv;
		(*vbuffer)[i].vert = origin + glm::vec4(-r * sin(a) * cos(e) / mpph, r * sin(e) / mppv, r * cos(a) * cos(e) / mpph, 0);
	}
	for (int i = 0; i < info_p->N; i++)
	{
		(*vbuffer)[i].color = mincolor + pts[i].Amp / maxAmp * (maxcolor - mincolor);
	}
	vbo.at(Main)->SetBuffer(vbuffer, &(*vbuffer)[0], vbuffer->size());
	vbo.at(Main)->NeedsReload = true;
	vbo.at(MiniMap)->SetBuffer(vbuffer, &(*vbuffer)[0], vbuffer->size());
	vbo.at(MiniMap)->NeedsReload = true;
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

int CSector::GetPoint(CViewPortControl* vpControl, glm::vec2 screenPoint)
{
	C3DObjectVBO *vbo_ = vbo.at(vpControl->Id);

	vector<VBOData> *buffer = (vector<VBOData> *)vbo_->GetBuffer();

	if (!buffer || buffer->size() == 0)
		return -1;
	glm::mat4 mv = vpControl->GetViewMatrix() * GetModelMatrix(vpControl);
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

void CSector::SelectPoint(int vpId, int pointIndex)
{
	C3DObjectVBO *vbo_ = vbo.at(vpId);

	vector<VBOData> *buffer = (vector<VBOData> *)vbo_->GetBuffer();

	if (!buffer || buffer->size() == 0)
		return;
	
	(*buffer)[pointIndex].color = CSettings::GetColor(ColorPointSelected);
	vbo_->NeedsReload = true;
}
