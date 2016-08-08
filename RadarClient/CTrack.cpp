#define WIN32_LEAN_AND_MEAN

#include "CRCSocket.h"
#include "CTrack.h"
#include "CSettings.h"
#include "CViewPortControl.h"
#include "C3DObjectVBO.h"


CTrack::CTrack(int id, bool selected)
{
	Selected = selected;
	ID = id;
	Found = false;
	Color = Selected ? CSettings::GetColor(ColorTrackSelected) : CSettings::GetColor(ColorTrack);
}


CTrack::~CTrack()
{
}


void CTrack::Refresh(glm::vec4 origin, float mpph, float mppv, vector<RDRTRACK*>* trackPoints)
{
	vector<VBOData> *vbuffer = (vector<VBOData> *)vbo.at(Main)->GetBuffer();
	if (vbuffer) {
		vbuffer->clear();
		vbuffer->resize(trackPoints->size());
	}
	else {
		vbuffer = new vector<VBOData>(trackPoints->size());
	}
	float r, a, e;
	for (int i = 0; i < trackPoints->size(); i++)
	{
		r = CSettings::GetFloat(FloatCTrackRefresh_Kr) * sqrt((*trackPoints)[i]->X * (*trackPoints)[i]->X + (*trackPoints)[i]->Y * (*trackPoints)[i]->Y + (*trackPoints)[i]->Z * (*trackPoints)[i]->Z);
		a = glm::radians(CSettings::GetFloat(FloatCTrackRefresh_a0) + CSettings::GetFloat(FloatCTrackRefresh_scale1) * CSettings::GetFloat(FloatCTrackRefresh_scale2) * (atan((*trackPoints)[i]->X / (*trackPoints)[i]->Y)) / (M_PI / 180));
		e = glm::radians(CSettings::GetFloat(FloatCTrackRefresh_e0) + CSettings::GetFloat(FloatCTrackRefresh_scale3) * atan((*trackPoints)[i]->Z / sqrt((*trackPoints)[i]->X * (*trackPoints)[i]->X + (*trackPoints)[i]->Y * (*trackPoints)[i]->Y)) / (M_PI / 180));

		(*vbuffer)[i].vert = origin + glm::vec4(-r * sin(a) * cos(e) / mpph, r * sin(e) / mppv, r * cos(a) * cos(e) / mpph, 0);
		(*vbuffer)[i].color = Color;
	}

	vbo.at(Main)->SetBuffer(vbuffer, &(*vbuffer)[0], vbuffer->size());
	vbo.at(Main)->NeedsReload = true;
	vbo.at(MiniMap)->SetBuffer(vbuffer, &(*vbuffer)[0], vbuffer->size());
	vbo.at(MiniMap)->NeedsReload = true;
}

void CTrack::SelectTrack(int vpId, bool selectState)
{
	Selected = selectState;

	C3DObjectVBO *vbo_ = vbo.at(vpId);

	vector<VBOData> *buffer = (vector<VBOData> *)vbo_->GetBuffer();

	if (!buffer || buffer->size() == 0)
		return;

	for (auto it = buffer->begin(); it != buffer->end(); ++it)
	{
		(*it).color = Selected ? CSettings::GetColor(ColorTrackSelected) : CSettings::GetColor(ColorTrack);
	}
	vbo_->NeedsReload = true;
}
