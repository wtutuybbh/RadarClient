#include "stdafx.h"

#include "CTrack.h"
#include "CRCSocket.h"
#include "CSettings.h"
#include "CViewPortControl.h"
#include "C3DObjectVBO.h"
#include "CRCLogger.h"

const std::string CTrack::requestID = "CTrack";

CTrack::CTrack(int id, bool selected): CSector(id)
{
	Selected = selected;
	ID = id;
	Found = false;
	Color = Selected ? CSettings::GetColor(ColorTrackSelected) : CSettings::GetColor(ColorTrack);
	c3DObjectModel_TypeName = "CTrack";
}


CTrack::~CTrack()
{
#define CTrack_DESTRUCTOR_LogInfo false
	if (CTrack_DESTRUCTOR_LogInfo)
	{
		LOG_INFO(requestID, "CTrack DESTRUCTOR", (boost::format("ID = %1%") % ID).str().c_str());
	}	
}


void CTrack::Refresh(glm::vec4 origin, float mpph, float mppv, vector<RDRTRACK*>* trackPoints)
{
	C3DObjectVBO *_vbo = nullptr;
	try 
	{
		_vbo = vbo.at(Main);
		if (!_vbo)
		{
			LOG_ERROR(requestID, "CTrack::Refresh", "vbo.at(Main) is null. RETURN.");
			return;
		}
	}
	catch (std::out_of_range ex)
	{
		LOG_ERROR(requestID, "CTrack::Refresh", "no vbo at index Main. RETURN.");
		return;
	}
	catch (const std::exception &ex) {
		LOG_WARN("exception", "CTrack::Refresh", ex.what());
	}
	void *_vbuffer = _vbo->GetBuffer();
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
		r = sqrt((*trackPoints)[i]->X * (*trackPoints)[i]->X + (*trackPoints)[i]->Y * (*trackPoints)[i]->Y + (*trackPoints)[i]->Z * (*trackPoints)[i]->Z);
		if ((*trackPoints)[i]->X > 0)
		{
			a = M_PI/2 - atan((*trackPoints)[i]->Y / (*trackPoints)[i]->X);
		}
		if ((*trackPoints)[i]->X < 0)
		{
			a = -M_PI+M_PI/2 - atan((*trackPoints)[i]->Y / (*trackPoints)[i]->X);
		}
		if ((*trackPoints)[i]->X == 0)
		{
			if ((*trackPoints)[i]->Y > 0)
			{
				a = 0;
			}
			if ((*trackPoints)[i]->Y < 0)
			{
				a = M_PI;
			}
		}
		
		e = atan((*trackPoints)[i]->Z / sqrt((*trackPoints)[i]->X * (*trackPoints)[i]->X + (*trackPoints)[i]->Y * (*trackPoints)[i]->Y));

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
