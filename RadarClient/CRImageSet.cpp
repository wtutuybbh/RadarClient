#include "stdafx.h"

#include "CRImageSet.h"
#include "CRImage.h"
#include "Util.h"
#include "CRCSocket.h"

CRImageSet::~CRImageSet()
{
	if (Images)
	{
		for (auto it = begin(*Images); it != end(*Images); ++it)
			delete (*it);
		Images->clear();
		delete Images;
	}
}


CRImageSet::CRImageSet()
{
	Images = new std::vector<CRImage*>;
}

void CRImageSet::Draw(CViewPortControl* vpControl, GLenum mode)
{
	if (Images) {
		for (auto it = Images->begin(); it != Images->end(); ++it)
		{
			(*it)->Draw(vpControl, mode);
		}
	}
}
void CRImageSet::Refresh(glm::vec4 origin, float mpph, float mppv, RDR_INITCL * rdrinit, RIMAGE* info, void* pixels)
{

	if (rdrinit->ScanMode == 3) // 3D
	{

	}
	else // 2D
	{
		float tickAzimuth = rdrinit->begAzm + rdrinit->dAzm * (info->d1 + info->d2) / 2 ;
		short currentDirection = sgn(info->d2 - info->d1);
		for (auto it = Images->begin(); it != Images->end(); ++it)
		{
			if (rcutils::between_on_circle((*it)->Azemuth, prevAzimuth, tickAzimuth, currentDirection, false, true)) {
				/*delete *it;
				it = Images->erase(it);*/
				if ((*it)->d1 == info->d1 && info->d2 == info->d2)
				{
					(*it)->Refresh(tickAzimuth, origin, mpph, mppv, rdrinit, info, pixels);
				}
				else
				{
					CRImage *img = new CRImage(tickAzimuth, origin, mpph, mppv, rdrinit, info, pixels);
					img->SetName(format("CRImage, tickAzimuth=%f, currentDirection=%d", tickAzimuth, currentDirection));
					img->d1 = info->d1;
					img->d2 = info->d2;
					Images->push_back(img);
				}
			}			
		}
		

		prevAzimuth = tickAzimuth;

		return;

		
	}

	
}