#include "stdafx.h"

#include "CRImageSet.h"
#include "CRImage.h"
#include "Util.h"
#include "CRCSocket.h"
#include "CRCLogger.h"

CRImageSet::~CRImageSet()
{	
	if (Images)
	{
		for (auto it = begin(*Images); it != end(*Images); ++it)
			delete (*it);
		Images->clear();
		delete Images;
		Images = nullptr;
	}
}


CRImageSet::CRImageSet()
{
	Images = new std::vector<CRImage*>;
}

void CRImageSet::Draw(CViewPortControl* vpControl, GLenum mode)
{
	std::lock_guard<std::mutex> lock(m);
	if (Images) {
		for (auto it = Images->begin(); it != Images->end(); ++it)
		{
			(*it)->Draw(vpControl, mode);
		}
	}
}

void CRImageSet::SetAlphaBehaviour(AlphaBehaviour ab)
{
	std::lock_guard<std::mutex> lock(m);
	if (Images) {
		for (auto it = Images->begin(); it != Images->end(); ++it)
		{
			(*it)->SetAlphaBehaviour(ab);
		}
	}
}

void CRImageSet::SetRotateMatrix(glm::mat4 m)
{
	if (Images) {
		for (auto it = Images->begin(); it != Images->end(); ++it)
		{
			(*it)->SetRotateMatrix(m);
		}
	}
}

void CRImageSet::Refresh(glm::vec4 origin, float mpph, float mppv, RDR_INITCL * rdrinit, RIMAGE* info, void* pixels)
{
	if (!Images)
	{
		return;
	}
	std::lock_guard<std::mutex> lock(m);
	std::string context = "CRImageSet::Refresh";
	if (rdrinit->ScanMode == 3) // 3D
	{

	}
	else // 2D
	{
		float tickAzimuth = rdrinit->begAzm + rdrinit->dAzm * (info->d1 + info->d2) / 2 ;
		short currentDirection = sgn(info->d2 - info->d1);
		if (CRImageSetRefreshLogEnabled) LOG_INFO__("d1= %d, d2= %d, tickAzimuth= %f", info->d1, info->d2, tickAzimuth);

		for (auto it = Images->begin(); it != Images->end(); )
		{
			if (rcutils::between_on_circle((*it)->Azemuth, prevAzimuth, tickAzimuth, currentDirection, false, true)) {
				if (CRImageSetRefreshLogEnabled) LOG_INFO__("will delete image with azimuth=%f", (*it)->Azemuth);
				delete *it;
				it = Images->erase(it);
			}
			else
				++it;
		}
		CRImage *img = new CRImage(tickAzimuth, origin, mpph, mppv, rdrinit, info, pixels);
		img->SetName(format("CRImage, tickAzimuth=%f, currentDirection=%d", tickAzimuth, currentDirection));
		Images->push_back(img);
		if (CRImageSetRefreshLogEnabled) LOG_INFO__("added new image with azimuth=%f", tickAzimuth);

		prevAzimuth = tickAzimuth;

		return;		
	}
}
