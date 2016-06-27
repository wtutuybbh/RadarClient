//#include "stdafx.h"
#include "CRImageSet.h"
#include "CRImage.h"
#include "Util.h"

CRImageSet::~CRImageSet()
{
}


CRImageSet::CRImageSet()
{
	Images = new std::vector<CRImage*>;
}

void CRImageSet::Draw(CViewPortControl* vpControl, GLenum mode)
{
	for (auto it = Images->begin(); it != Images->end(); ++it)
	{
		(*it)->Draw(vpControl, mode);
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
		for (auto it = Images->begin(); it != Images->end(); )
		{
			if (rcutils::between_on_circle((*it)->Azemuth, prevAzimuth, tickAzimuth, currentDirection, false, true)) {
				delete *it;
				it = Images->erase(it);
			}
			else
				++it;
		}
		CRImage *img = new CRImage(tickAzimuth, origin, mpph, mppv, rdrinit, info, pixels);
		Images->push_back(img);

		prevAzimuth = tickAzimuth;

		return;

		
	}

	
}