#include "stdafx.h"
#include "CScene.h"

#include "CRCPoint.h"
#include "CTrack.h"

#include "CMesh.h"
#include "CMarkup.h"
#include "CSector.h"

#include "CMinimapPointer.h"

#include "Util.h"

#include "CCamera.h"
#include "CSettings.h"
#include "CViewPortControl.h"
#include "CRImageSet.h"
#include "CLine.h"

#include "CRCSocket.h"
#include "C3DObjectModel.h"
#include "C3DObjectVBO.h"
#include "C3DObjectProgram.h"

#include "CUserInterface.h"
#include "CRCLogger.h"
#include "CRay.h"
#include "CRImage.h"

const std::string CScene::requestID = "CScene";

void CScene::PushSelection(C3DObjectModel* o)
{
	if (o)
		Selection.push_back(o);
}

void CScene::ClearSelection()
{
	Selection.clear();
	for (int i = 0; i < Sectors.size(); i++)
	{
		if (Sectors.at(i)) {
			Sectors.at(i)->UnselectAll(Main);
			Sectors.at(i)->UnselectAll(MiniMap);
		}
	}
}

CScene::CScene() 
{
	std::string context = "CScene::CScene";
	LOG_INFO(requestID, context, "Start");
	rdrinit = nullptr;

	SectorsCount = 0;

	Camera = new CCamera();
	Camera->Move(glm::vec3(0, 235, -310), false);

	position.x = CSettings::GetFloat(FloatPositionLon);
	position.y = CSettings::GetFloat(FloatPositionLat);
	max_range = CSettings::GetFloat(FloatMaxDistance);
	resolution = CSettings::GetInt(IntResolution);	
	MPPv = CSettings::GetFloat(FloatMPPv);
	MPPh = CSettings::GetFloat(FloatMPPh);
	texsize = CSettings::GetInt(IntTexSize);

	CScene::LoadMesh();
	/*
	boost::thread t(boost::bind(&CScene::LoadMesh, this));
	t.detach();
	*/

	mmPointer = new CMiniMapPointer(this);	

	
	Markup = new CMarkup(glm::vec4(0, 0, 0, 1));
	

	numCircles = 7;
	marksPerCircle = 10;

	markCount = marksPerCircle * numCircles * 5;

	segmentsPerCircle = 1000;

	rayWidth = 10;
	minE = 10;
	maxE = 30;

	

	rayDensity = 1;
	vertexCount_Ray = 3;// (rayDensity + 1) * (rayDensity + 1) * 2;
	
	vertexCount_Axis = 38;
	
	vertexCount_Info = 4;
	

	vertexCount = vertexCount_Axis + markCount*2 + numCircles * segmentsPerCircle + vertexCount_Info;



	infoWidth = 370;
	infoHeight = 250;

	markSize = 10.0f;
	AxisGridShift = 250;
	viewAngle = 0;
	markDistance = 100;
	YAxisLength = (markCount / 5) * markDistance / MPPv;

	RotatingSpeed = 120;	

	RayVBOisBuilt = VBOisBuilt = MiniMapVBOisBuilt = false;

	auto r = max_range / MPPh;
	begAzmLine = new CLine(Main, glm::vec4(0, 0, 0, 1), glm::vec4(-r * sin(0), 0, r * cos(0), 1), Simple);
	begAzmLine->SetName("begAzmLine");

}
CScene::~CScene() {
	if (CScene_Destructor_Log) LOG_INFO_("CScene DESTRUCTOR", "ololo");
	ClearSelection();
	if (Markup)
	{
		delete Markup;
		Markup = nullptr;
	}

	if (mmPointer)
	{
		delete mmPointer;
		mmPointer = nullptr;
	}

	if (Camera) 
	{
		delete Camera;
		Camera = nullptr;
	}


	for (auto it = begin(Sectors); it != end(Sectors); ++it)
	{
		delete *it;
	}
	Sectors.clear();

	for (auto it = begin(Tracks); it != end(Tracks); ++it)
	{
		delete it->second; //TODO: here it fails
	}
	Tracks.clear();

	if (begAzmLine)
	{
		delete begAzmLine;
		begAzmLine = nullptr;
	}

	if (ImageSet)
	{
		delete ImageSet;
		ImageSet = nullptr;
	}

	if (RayObj)
	{
		delete RayObj;
		RayObj = nullptr;
	}
	if (Mesh) {
		delete Mesh;
		Mesh = nullptr;
	}
}

bool CScene::DrawScene(CViewPortControl * vpControl)
{
	if (Camera)
	{
		Camera->SetFovy(CSettings::GetFloat(FloatFovy));
		Camera->SetZPlanes(CSettings::GetFloat(FloatZNear), CSettings::GetFloat(FloatZFar));
	}
	//glLineWidth(3);
	glEnable(GL_DEPTH_BUFFER);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LESS);
	//

	if (MeshReady())
	{
		Mesh->UseTexture = vpControl->DisplayMap;
		Mesh->UseY0Loc = !vpControl->DisplayLandscape;
		//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		Mesh->Draw(vpControl, GL_TRIANGLES);
		//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}
	glDisable(GL_DEPTH_BUFFER);

	if (begAzmLine) {
		begAzmLine->Draw(vpControl, GL_LINES);
	}


	/*if (UI->GetCheckboxState_MarkupLabels())
	{
		DrawBitmaps();
	}*/

	if (RayObj)
	{
		RayObj->Draw(vpControl, GL_LINES);
	}
	
	//glEnable(GL_DEPTH_TEST);
	glEnable(GL_PROGRAM_POINT_SIZE);
	if (UI->GetCheckboxState_Points()) {
		std::lock_guard<std::mutex> lock(mtxSectors);
		for (int i = 0; i < Sectors.size(); i++) {
			if (Sectors[i] != nullptr)
			{
				if (UI->GetCheckboxState_PointsFadeout())
				{
					Sectors[i]->SetAlphaBehaviour(FadeOut);
				}
				else
				{
					Sectors[i]->SetAlphaBehaviour(Constant);
				}
				Sectors[i]->Draw(vpControl, GL_POINTS);
			}
				
		}
	}

	if (UI->GetCheckboxState_Tracks()) {
		std::lock_guard<std::mutex> lock(mtxTracks);
		for (auto it = Tracks.begin(); it != Tracks.end(); ++it)
		{

			it->second->Draw(vpControl, GL_POINTS);
			it->second->Draw(vpControl, GL_LINE_STRIP);
			glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
			auto buffer = it->second->vertices.get()->GetBuffer();
			if (buffer)
			{
				auto vert = it->second->vertices.get()->getv(0);
				BitmapString(vert->x, vert->y + 10, vert->z, num2str(it->second->ID, 0));
			}							
		}
	}

	
	if (UI->GetCheckboxState_Images())
	{
		if (ImageSet)
		{
			if (UI->GetCheckboxState_ImagesFadeout())
			{
				ImageSet->SetAlphaBehaviour(FadeOut);
			}
			else
			{
				ImageSet->SetAlphaBehaviour(Constant);
			}
			ImageSet->Draw(vpControl, GL_POINTS);
		}
	}

	//glDisable(GL_PROGRAM_POINT_SIZE);
	if (MeasurePath)
	{
		glDepthFunc(GL_GEQUAL);
		glDepthMask(GL_FALSE);
		glLineStipple(1, 0xAAAA);
		glEnable(GL_LINE_STIPPLE);
		MeasurePath->UseUniColor(1.0f);
		MeasurePath->Draw(vpControl, GL_POINTS);
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		glDisable(GL_LINE_STIPPLE);
		MeasurePath->UseUniColor(0.0f);
		MeasurePath->Draw(vpControl, GL_POINTS);
	}
	if (Markup && UI && UI->GetCheckboxState_MarkupLines())
	{
		//glDepthFunc(GL_GEQUAL);
		if (CUserInterface::GetCheckboxState_AltitudeMap()) 
		{
			glDepthFunc(GL_GEQUAL);
			glDepthMask(GL_FALSE);
			glLineStipple(1, 0xAAAA);
			glEnable(GL_LINE_STIPPLE);
			Markup->UseUniColor(1.0);
			Markup->Draw(vpControl, 0);
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
		}
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		glDisable(GL_LINE_STIPPLE);
		Markup->UseUniColor(0.0);
		Markup->Draw(vpControl, 0);
	}
	glDisable(GL_PROGRAM_POINT_SIZE);

	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	
	
	mainDrawCount++;
	return true;
}

bool CScene::MiniMapDraw(CViewPortControl * vpControl)
{
	//glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_BUFFER);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (Mesh && Mesh->Ready())
	{
		if (waitingForMesh && Camera) {
			Camera->MeshSize = Mesh->GetSize();
			waitingForMesh = false;
		}
		Mesh->Draw(vpControl, GL_TRIANGLES);
	}
	glDisable(GL_DEPTH_BUFFER);
	
	if (RayObj)
	{
		RayObj->Draw(vpControl, GL_LINES);
	}
	glEnable(GL_PROGRAM_POINT_SIZE);

	if (UI->GetCheckboxState_Points()) 
	{
		for (int i = 0; i < Sectors.size(); i++) 
		{
			if (Sectors[i] != nullptr)
				Sectors[i]->Draw(vpControl, GL_POINTS);
		}
	}
	if (UI->GetCheckboxState_Tracks()) {
		std::lock_guard<std::mutex> lock(mtxTracks);
		for (auto it = Tracks.begin(); it != Tracks.end(); ++it)
		{
			it->second->Draw(vpControl, GL_POINTS);
			it->second->Draw(vpControl, GL_LINE_STRIP);
		}
	}
	if (UI->GetCheckboxState_Images())
	{
		if (ImageSet)
		{
			ImageSet->Draw(vpControl, GL_POINTS);
		}
	}
	glDisable(GL_PROGRAM_POINT_SIZE);

	if (Markup)
	{
		Markup->Draw(vpControl, 0);
	}
	if (begAzmLine)
	{
		begAzmLine->Draw(vpControl, GL_LINES);
	}
	if (mmPointer)
	{
		mmPointer->Draw(vpControl, GL_TRIANGLES);
	}
	return true;
}


void CScene::RefreshSector(RPOINTS * info_p, RPOINT * pts, RDR_INITCL* init)
{

	if (!info_p || !pts || !init)
		return; //do nothing if no RPOINTS structure provided
	if (init->MaxNAzm <= 0 || info_p->N<=0) //TODO: full defence against bad data need to be there
		return;
	if (Sectors.size() == 0)
	{
		LOG_ERROR("CScene", "RefreshSector", "Sectors.size() == 0");
		return;
	}

	//Init = init;
	if (!Initialized)
		Init(init);	

	viewAngle = glm::degrees(init->begAzm + init->dAzm * (info_p->d1 + info_p->d2) / 2);
	
	if (info_p->d1 == info_p->d2)
		return;

	float min_ = std::fmin(info_p->d1, info_p->d2);

	if (min_ < 0)
		return;

	if (std::fmax(info_p->d1, info_p->d2) > init->MaxNAzm)
		return;

	if (SectorsCount <= 0)
		return;
	
	int currentSector = SectorsCount * (min_ / init->MaxNAzm);

	MPPh = CSettings::GetFloat(FloatMPPh);
	MPPv = CSettings::GetFloat(FloatMPPv);
	auto lifeTime = CSettings::GetInt(IntPointLifetime);
	for(auto i=0; i<lifeTime; i++)
	{
		if (Sectors[SectorsCount * i + currentSector] == nullptr)
		{
			Sectors[SectorsCount * i + currentSector] = new CSector(currentSector);
			if (CSceneRefreshSector_Log) LOG_INFO("CSceneRefreshSector_Log", "RefreshSector", "new CSector %d %d", i, currentSector);
			break;
		}
		Sectors[SectorsCount * i + currentSector]->life_counter++;
		if (CSceneRefreshSector_Log) LOG_INFO("CSceneRefreshSector_Log", "RefreshSector", "CSector %d %d, life_counter++ : %d -> %d", i, currentSector,
			Sectors[SectorsCount * i + currentSector]->life_counter-1,
			Sectors[SectorsCount * i + currentSector]->life_counter);

		if (Sectors[SectorsCount * i + currentSector]->life_counter == lifeTime)
		{
			Sectors[SectorsCount * i + currentSector]->Refresh(glm::vec4(0, 0, 0, 1), MPPh, MPPv, info_p, pts, init);
			if (CSceneRefreshSector_Log) LOG_INFO("CSceneRefreshSector_Log", "RefreshSector", "CSector %d %d, Refresh", i, currentSector);
			Sectors[SectorsCount * i + currentSector]->life_counter = 0;
		}
				
	}

	

	//LOG_INFO_("CScene::RefreshSector", "currentSector=%d", currentSector);
	if (RayObj) 
	{
		//RayObj->SetRotateMatrix(glm::rotate((float)(- 2.0f * M_PI * (currentSector + 0.5) / SectorsCount), glm::vec3(0, 1, 0)));
	}


		
	
	UI->FillInfoGrid(this);
}

void CScene::RefreshSectorAsync(RPOINTS* info_p, RPOINT* pts, RDR_INITCL* init, char* delete_after)
{	
	std::thread t( [this](RPOINTS* info_p_, RPOINT* pts_, RDR_INITCL* init_, char* delete_after_)
	{
		std::lock_guard<std::mutex> lock(mtxSectors);
		if (delete_after_)
		{
			RefreshSector(info_p_, pts_, init_);
			delete[] delete_after_;
		}
	}, info_p, pts, init, delete_after );
	t.detach();
}

void CScene::ClearSectors()
{
	Sectors.clear();

	SectorsCount = 0;
}

void CScene::Dump() const
{
	ptree pt;
	if (Markup)
	{
		pt.add_child("Markup", Markup->vertices.get()->GetPropertyTree(0, 100));
	}
	if (MeshReady()) 
	{
		pt.add_child("Mesh", Mesh->vertices.get()->GetPropertyTree(0, 100));
	}
	
	write_json("objects.json", pt);
}

void CScene::RefreshTracks(vector<TRK*>* tracks)
{	
	{
		std::lock_guard<std::mutex> lock(mtxTracks);
		if (tracks->size() == 0)
			return;

		//m->lock();

		for (int i = 0; i < tracks->size(); i++)
		{
			bool insertNew = true;
			for (auto it = Tracks.begin(); it != Tracks.end(); ++it)
			{
				if (it->first == tracks->at(i)->id)
				{
					it->second->Found = tracks->at(i)->Found = true;
					it->second->Refresh(glm::vec4(0, 0, 0, 1), MPPh, MPPv, &tracks->at(i)->P);
					it->second->SelectTrack(Main, std::find(SelectedTracksIds.begin(), SelectedTracksIds.end(), tracks->at(i)->id) != SelectedTracksIds.end());
					insertNew = false;
				}
			}
			if (insertNew)
			{
				LOG_INFO(requestID, "CScene::RefreshTracks", "new track, id=%d", tracks->at(i)->id);
				CTrack *t = new CTrack(tracks->at(i)->id, std::find(SelectedTracksIds.begin(), SelectedTracksIds.end(), tracks->at(i)->id) != SelectedTracksIds.end());
				t->Refresh(glm::vec4(0, 0, 0, 1), MPPh, MPPv, &tracks->at(i)->P);
				t->Found = true;
				Tracks.insert_or_assign(tracks->at(i)->id, t);
			}
		}
		for (auto it = Tracks.begin(); it != Tracks.end();)
		{
			if (!it->second->Found)
			{
				LOG_INFO(requestID, "CScene::RefreshTracks", (boost::format("going to delete track with id=%1%") % it->second->ID).str().c_str());
				delete it->second;
				it = Tracks.erase(it);
			}
			else
			{
				it->second->Found = false;
				++it;
			}
		}
		UI->FillInfoGrid(this);
	}
}

void CScene::RefreshImages(RIMAGE* info, void* pixels, RDR_INITCL* init)
{
	if (!Mesh || !Mesh->Ready()) {
		return;
	}
	float y_0 = GetY0();
	if (ImageSet)
	{
		ImageSet->Refresh(glm::vec4(0, y_0, 0, 1), MPPh, MPPv, rdrinit, info, pixels);
	}
	if (RayObj)
	{
		//RayObj->SetRotateMatrix(glm::rotate((float)(- M_PI * (float(info->d1 + info->d2) / init->MaxNAzm)), glm::vec3(0, 1, 0)));
		RayObj->Refresh((float)(-M_PI * (float(info->d1 + info->d2) / init->MaxNAzm)));
	}
}

void CScene::Init(RDR_INITCL* init)
{
	if (!Mesh || !Mesh->Ready()) {
		return;
	}
	float y_0 = GetY0();
	std::string context = "CScene::Init";
	
	if (!init)
	{
		LOG_ERROR__("parameter init is nullptr");
		return;
	}
	LOG_INFO__("Start... init=%#010x{begAzm=%.4f, dAzm=%.4f, MaxNAzm=%d, begElv=%.4f, dElv=%.4f, Nelv=%d, ViewStep=%d}", init, init->begAzm, init->dAzm, init->MaxNAzm, init->begElv, init->dElv, init->Nelv, init->ViewStep);
	
	if (init->ViewStep == 0)
	{
		LOG_ERROR__("init->ViewStep == 0");
		return;
	}

	rdrinit = init;

	if (UI)
	{
		float ba_deg = glm::degrees(init->begAzm);
		while (ba_deg < 0)
		{
			ba_deg = 360 + ba_deg; //circle magic
		}			
		UI->SetTrackbarValue_BegAzm(100.0 * (ba_deg-CSettings::GetFloat(FloatMinBegAzm)) / (CSettings::GetFloat(FloatMaxBegAzm) - CSettings::GetFloat(FloatMinBegAzm)));
		UI->SetTrackbarValue_ZeroElevation(100.0 * CSettings::GetFloat(FloatZeroElevation) / 90.0);
	}
	else
	{
		LOG_WARN__("UI not initialized");
	}
	int markDistance = CSettings::GetInt(IntMarkupMarkDistance);
	int numCircles = CSettings::GetInt(IntMarkupNumCircles);
	int marksPerCircle = CSettings::GetInt(IntMarkupMarksPerCircle);
	int r = markDistance * numCircles * marksPerCircle;
	
	

	minE = init->begElv;
	maxE = init->begElv + init->dElv * init->Nelv;
	if (minE==0 && maxE==0)
	{
		minE = maxE = CSettings::GetFloat(FloatZeroElevation);
	}

	rayWidth = init->dAzm * init->ViewStep;
	RayObj = new CRay(rayWidth, CSettings::GetFloat(FloatMaxDist), 0);

	CSettings::SetFloat(FloatMaxDist, init->dR * init->maxR);

	SectorsCount = init->MaxNAzm / init->ViewStep;
	Sectors.resize(SectorsCount * CSettings::GetInt(IntPointLifetime));
	for (int i = 0; i < Sectors.size(); i++)
	{
		Sectors[i] = nullptr;
	}

	if (!ImageSet)
	{
		ImageSet = new CRImageSet();
	}
	
	Initialized = true;	
}

CRCPointModel * CScene::GetCRCPointFromRDRTRACK(RDRTRACK * tp) const
{
	if (!Mesh || !Mesh->Ready()) {
		return nullptr;
	}

	//float y_0 = Mesh->GetCenterHeight() / MPPv;
	//TODO: need common formula here
	float r = 0.75 * sqrt(tp->X * tp->X + tp->Y * tp->Y) / 1;
	float a = glm::radians(-120 + 0.02 * 1000 * (atan(tp->X / tp->Y)) / (M_PI / 180));
	float e = glm::radians(30.0f);
	CRCPointModel* p = new CRCPointModel(Main, 0, MPPh, MPPv, r, a, e);
	p->Color = CSettings::GetColor(ColorTrack);
	return p;
}

void CScene::SetCameraPositionFromMiniMapXY(float x, float y, float direction) const
/* x and y from -1 to 1 */
{
	if (Mesh) 
	{
		auto b = Mesh->GetBounds();
		if (b && Camera)
			Camera->SetPositionXZ((b[0].x + b[1].x) / 2 - x * (b[1].x - b[0].x) / 2, (b[0].z + b[1].z) / 2 + y * (b[1].z - b[0].z) / 2);
	}
}
C3DObjectModel * CScene::GetObjectAtMiniMapPosition(int vpId, glm::vec3 p0, glm::vec3 p1) const
{
	if (mmPointer) {
		glm::vec3 orig = p0;
		glm::vec3 dir = p1 - p0;
		glm::vec3 pos;
		if (mmPointer->IntersectLine(vpId, orig, dir, pos)) {
			return mmPointer;
		}
	}
	return nullptr;
}

C3DObjectModel * CScene::GetSectorPoint(CViewPortControl *vpControl, glm::vec2 screenPoint, int& index)
{
#define CScene_GetSectorPoint_LogInfo false
	
	if (!Mesh || !Mesh->Ready()) {
		return nullptr;
	}
	//float y_0 = Mesh->GetCenterHeight() / MPPv;

	index = -1;
	std::string context = "CScene::GetPointOnSurface";

	if (!vpControl)
	{
		LOG_ERROR(requestID, context, (boost::format("vpControl is nullptr, screenPoint=(%1%, %2%)") % screenPoint.x % screenPoint.y).str().c_str());
		return nullptr;
	}

	if (CScene_GetSectorPoint_LogInfo)
	{
		LOG_INFO(requestID, context, (boost::format("Start... vpControl.Id=%1%, screenPoint=(%2%, %3%)") % vpControl->Id % screenPoint.x % screenPoint.y).str().c_str());
	}

	for (int i = 0; i < Sectors.size(); i++)
	{
		if (Sectors[i])
		{
			index = Sectors[i]->GetPoint(vpControl, screenPoint);
			if (index >= 0)
			{
				Sectors[i]->SelectPoint(Main, index);
				CRCPointModel *point = new CRCPointModel(vpControl->Id, 0, this->MPPh, this->MPPv, 0, 0, 0);
				auto coords = Sectors[i]->GetPointCoords(vpControl, index);
				point->SetCartesianCoordinates(coords);
				LOG_INFO(requestID, context, (boost::format("(vpControl.Id=%1%, screenPoint=(%2%, %3%)) -> (Sector %4%, index=%5%, RETURN point=(%6%, %7%, %8%))") 
					% vpControl->Id % screenPoint.x % screenPoint.y % i % index % coords.x % coords.y % coords.z).str().c_str());
				return point;
			}
		}
	}
	if (CScene_GetSectorPoint_LogInfo) 
	{
		LOG_INFO(requestID, context, "Point not found");
	}
	return nullptr;
}

C3DObjectModel* CScene::GetFirstTrackBetweenPoints(CViewPortControl *vpControl, glm::vec2 screenPoint, int& index)
{
#define CScene_GetFirstTrackBetweenPoints_LogInfo false
	index = -1;
	std::string context = "CScene::GetFirstTrackBetweenPoints";

	if (!vpControl)
	{
		LOG_ERROR(requestID, context, (boost::format("vpControl is nullptr, screenPoint=(%1%, %2%)") % screenPoint.x % screenPoint.y).str().c_str());
		return nullptr;
	}

	if (CScene_GetFirstTrackBetweenPoints_LogInfo)
	{
		LOG_INFO(requestID, context, (boost::format("Start... vpControl.Id=%1%, screenPoint=(%2%, %3%)") % vpControl->Id % screenPoint.x % screenPoint.y).str().c_str());
	}

	for (auto it = Tracks.begin(); it != Tracks.end(); ++it)
	{
		if (it->second)
		{			
			index = it->second->GetPoint(vpControl, screenPoint);
			if (index >= 0)
			{
				SelectedTracksIds.push_back(it->second->ID);
				it->second->SelectTrack(Main, true);
				
				LOG_INFO(requestID, context, (boost::format("(vpControl.Id=%1%, screenPoint=(%2%, %3%)) -> (Track ID=%4%, index=%5%)") % vpControl->Id % screenPoint.x % screenPoint.y % it->second->ID % index).str().c_str());
				return it->second;
			}
		}
	}
	if (CScene_GetFirstTrackBetweenPoints_LogInfo)
	{
		LOG_INFO(requestID, context, "Track not found");
	}
	return nullptr;
}

void CScene::AddMeasurePoint(glm::vec3 p0, glm::vec3 p1)
{
	if (!Mesh || !Mesh->Ready()) {
		return;
	}

	std::string context = "CScene::AddMeasurePoint";

	LOG_INFO(requestID, context, (boost::format("Start... p0=(%1%, %2%, %3%), p1=(%4%, %5%, %6%)")
		% p0.x % p0.y % p0.z % p1.x % p1.y % p1.z).str().c_str());

	glm::vec3 dir = p1 - p0, pos;
	if(Mesh->IntersectLine(Main, p0, dir, pos))
	{
		MeasurePoints.push_back(pos);
		if (!MeasurePath)
		{
			MeasurePath = new CPath();
		}
		MeasurePath->AddPoint(glm::vec4(pos, 1));
	}
}

void CScene::ClearMeasure()
{
	if (MeasurePath) {
		auto tmp = MeasurePath;
		MeasurePath = nullptr;
		delete tmp;
	}
	MeasurePoints.clear();
}

float CScene::GetMeasureLength()
{
	float res = 0.0f;
	float MPPh = CSettings::GetFloat(FloatMPPh);
	float MPPv = CSettings::GetFloat(FloatMPPv);

	/*XYZ*/
	if (MeasurePoints.size() >= 2) {
		for (auto i = 0; i < MeasurePoints.size() - 1; i++)
		{
			auto p0 = glm::vec3(MeasurePoints[i].x * MPPh, MeasurePoints[i].y * MPPv, MeasurePoints[i].z * MPPh);
			auto p1 = glm::vec3(MeasurePoints[i + 1].x * MPPh, MeasurePoints[i + 1].y * MPPv, MeasurePoints[i + 1].z * MPPh);
			res += glm::length(p1 - p0);
		}
	}
	return res;
}

glm::vec2 CScene::CameraXYForMiniMap() const
{
	if (Mesh) {
		auto m_Bounds = Mesh->GetBounds();
		if (m_Bounds && Camera) {
			return glm::vec2(-2 * (Camera->GetPosition().x - m_Bounds[0].x) / (m_Bounds[1].x - m_Bounds[0].x) + 1, -1 + 2 * (Camera->GetPosition().z - m_Bounds[0].z) / (m_Bounds[1].z - m_Bounds[0].z));
		}
	}
	return glm::vec2(0);
}
bool CScene::MeshReady() const
{
	return Mesh && Mesh->Ready();
}
float CScene::GetY0() {
	return 0;
}


void CScene::DrawBitmaps() const
{
	if (!Mesh || !Mesh->Ready()) {
		return;
	}
	float y_0 = 0;// Mesh->GetCenterHeight() / MPPv;

	BitmapString(0, y_0, 0, "(" + cnvrt::float2str(position.x) + "; " + cnvrt::float2str(position.y) + ")");
	auto color = CSettings::GetColor(ColorNumbers);
	glColor4f(color.r, color.g, color.b, color.a);
	BitmapString(-10 * markDistance / MPPh, y_0 + 1, 0, "1km");
	BitmapString(10 * markDistance / MPPh, y_0 + 1, 0, "1km");
	BitmapString(0, y_0 + 1, -10 * markDistance / MPPh, "1km");
	BitmapString(0, y_0 + 1, 10 * markDistance / MPPh, "1km");
	BitmapString(0, y_0 + 10 * markDistance / MPPv, 0, "1km");

	BitmapString(-20 * markDistance / MPPh, y_0 + 1, 0, "2km");
	BitmapString(20 * markDistance / MPPh, y_0 + 1, 0, "2km");
	BitmapString(0, y_0 + 1, -20 * markDistance / MPPh, "2km");
	BitmapString(0, y_0 + 1, 20 * markDistance / MPPh, "2km");
	BitmapString(0, y_0 + 20 * markDistance / MPPv, 0, "2km");

	BitmapString(-30 * markDistance / MPPh, y_0 + 1, 0, "3km");
	BitmapString(30 * markDistance / MPPh, y_0 + 1, 0, "3km");
	BitmapString(0, y_0 + 1, -30 * markDistance / MPPh, "3km");
	BitmapString(0, y_0 + 1, 30 * markDistance / MPPh, "3km");
	BitmapString(0, y_0 + 30 * markDistance / MPPv, 0, "3km");

	BitmapString(-40 * markDistance / MPPh, y_0 + 1, 0, "4km");
	BitmapString(40 * markDistance / MPPh, y_0 + 1, 0, "4km");
	BitmapString(0, y_0 + 1, -40 * markDistance / MPPh, "4km");
	BitmapString(0, y_0 + 1, 40 * markDistance / MPPh, "4km");
	BitmapString(0, y_0 + 40 * markDistance / MPPv, 0, "4km");

	BitmapString(-50 * markDistance / MPPh, y_0 + 1, 0, "5km");
	BitmapString(50 * markDistance / MPPh, y_0 + 1, 0, "5km");
	BitmapString(0, y_0 + 1, -50 * markDistance / MPPh, "5km");
	BitmapString(0, y_0 + 1, 50 * markDistance / MPPh, "5km");
	BitmapString(0, y_0 + 50 * markDistance / MPPv, 0, "5km");

	for (int a = 0; a < 360; a += 10) {
		BitmapString(-(CSettings::GetFloat(FloatMaxDist) + 50 * MPPh) * sin(cnvrt::dg2rad(a)) / MPPh, y_0, (CSettings::GetFloat(FloatMaxDist) + 50 * MPPh) * cos(cnvrt::dg2rad(a)) / MPPh, cnvrt::float2str(a) + "�");
	}

	glColor3f(0.8f, 0.8f, 1.0f);
	BitmapString(0, y_0 + markDistance, 60 * markDistance / MPPh, "N");
	glColor3f(1.0f, 0.8f, 0.8f);
	BitmapString(0, y_0 + markDistance, -60 * markDistance / MPPh, "S");
	glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
	BitmapString(60 * markDistance / MPPh, y_0 + markDistance, 0, "W");
	BitmapString(-60 * markDistance / MPPh, y_0 + markDistance, 0, "E");
}

void CScene::SetBegAzm(double begAzm)
{
	/*if (rdrinit)
	{
		rdrinit->begAzm = begAzm;
	}*/
	auto markDistance = CSettings::GetInt(IntMarkupMarkDistance);
	auto numCircles = CSettings::GetInt(IntMarkupNumCircles);
	auto marksPerCircle = CSettings::GetInt(IntMarkupMarksPerCircle);
	auto mpph = CSettings::GetFloat(FloatMPPh);
	auto r = float(markDistance * numCircles * marksPerCircle) / mpph;

	
	
	auto rotate = glm::rotate(-float(begAzm), glm::vec3(0, 1, 0));
	if (ImageSet)
	{		
		ImageSet->SetRotateMatrix(rotate);
		for(auto i=0; i< Sectors.size(); i++)
		{
			if (Sectors[i])
			{
				Sectors[i]->SetRotateMatrix(rotate);
			}
		}
	}
	if (RayObj)
	{
		RayObj->SetRotateMatrix(rotate);
	}
	
	if (begAzmLine) 
	{
		begAzmLine->SetRotateMatrix(rotate);
	}
}

glm::vec3 CScene::GetGeographicCoordinates(glm::vec3 glCoords)
{
	return glm::vec3(position.x + glCoords.x * MPPh / cnvrt::londg2m(1, position.y), position.y + glCoords.z * MPPh / cnvrt::londg2m(1, position.y), glCoords.y);
}
CMesh * CRImage::mesh;
void CScene::LoadMesh()
{
	if (CSettings::GetInt(IntLoadMesh) == 1) 
	{
		CMesh *mesh = new CMesh(false, position, max_range, texsize, resolution, MPPh, MPPv);
		waitingForMesh = true;
		Mesh = mesh;

		CRImage::mesh = mesh;
	}
}

glm::vec3 CScene::GetMeshSize() {
	if (Mesh) return Mesh->GetSize();
	return glm::vec3();
}