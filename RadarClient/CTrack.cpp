#include "CTrack.h"
#include "CRCPoint.h"
#include "CSettings.h"


CTrack::CTrack(int id)
{
	ID = id;
	Found = false;
	Color = CSettings::GetColor(ColorTrack);
}


CTrack::~CTrack()
{
}


void CTrack::Refresh(glm::vec4 origin, float mpph, float mppv, vector<RDRTRACK>* trackPoints)
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
		r = 0.75 * sqrt((*trackPoints)[i].X * (*trackPoints)[i].X + (*trackPoints)[i].Y * (*trackPoints)[i].Y) / 1;
		a = glm::radians(-120 + 0.02 * 1000 * (atan((*trackPoints)[i].X / (*trackPoints)[i].Y)) / (M_PI / 180));
		e = glm::radians(30.0f);

		(*vbuffer)[i].vert = origin + glm::vec4(-r * sin(a) * cos(e) / mpph, r * sin(e) / mppv, r * cos(a) * cos(e) / mpph, 0);
		(*vbuffer)[i].color = Color;
	}

	vbo.at(Main)->SetBuffer(vbuffer, &(*vbuffer)[0], vbuffer->size());
	vbo.at(Main)->NeedsReload = true;
	vbo.at(MiniMap)->SetBuffer(vbuffer, &(*vbuffer)[0], vbuffer->size());
	vbo.at(MiniMap)->NeedsReload = true;
}
