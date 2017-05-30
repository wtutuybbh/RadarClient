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
	std::lock_guard<std::mutex> lock(m);

	if (!vertices && trackPoints->size() > 0)
		vertices = std::make_shared<C3DObjectVertices>(trackPoints->size());
	if (!vertices)
		return;
	if (vertices.get()->vertexCount < trackPoints->size())
		vertices.get()->ReCreate(trackPoints->size());

	float r, a, e;
	auto vbuffer = vertices.get()->GetBuffer();
	auto vertexSize = vertices.get()->vertexSize;
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

		auto v = origin + glm::vec4(-r * sin(a) * cos(e) / mpph, r * sin(e) / mppv, r * cos(a) * cos(e) / mpph, 0);
		vbuffer[i * vertexSize] = v.x;
		vbuffer[i * vertexSize + 1] = v.y;
		vbuffer[i * vertexSize + 2] = v.z;
		vbuffer[i * vertexSize + 3] = 1.0f;

		vbuffer[i * vertexSize + 7] = Color.r;
		vbuffer[i * vertexSize + 8] = Color.g;
		vbuffer[i * vertexSize + 9] = Color.b;
		vbuffer[i * vertexSize + 10] = Color.a;
	}

	vertices.get()->needsReload = 2;

	if (!vbo.at(Main)->vertices)
		vbo.at(Main)->vertices = vertices;
	if (!vbo.at(MiniMap)->vertices)
		vbo.at(MiniMap)->vertices = vertices;

	vertices.get()->usesCount = 2;
}

void CTrack::SelectTrack(int vpId, bool selectState)
{
	Selected = selectState;

	if (!vertices)
		return;
	float r, a, e;
	auto vbuffer = vertices.get()->GetBuffer();
	auto vertexSize = vertices.get()->vertexSize;
	auto vertexCount = vertices.get()->vertexCount;


	for (auto i = 0; i < vertexCount; i++)
	{
		auto color = Selected ? CSettings::GetColor(ColorTrackSelected) : CSettings::GetColor(ColorTrack);
		vbuffer[i * vertexSize + 7] = color.r;
		vbuffer[i * vertexSize + 8] = color.g;
		vbuffer[i * vertexSize + 9] = color.b;
		vbuffer[i * vertexSize + 10] = color.a;
	}
	vertices.get()->needsReload = 2;
}
