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

	this->Camera = new CCamera();

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

		

	numCircles = 7;
	marksPerCircle = 10;

	markCount = marksPerCircle * numCircles * 5;

	segmentsPerCircle = 1000;

	rayWidth = 10;
	minE = 10;
	maxE = 30;
	minDist = 100;
	maxDist = 5000;
	

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

	AxisGrid = nullptr;
	AxisGridColor = nullptr;
	Ray = nullptr;
	RayColor = nullptr;
	Ray_VBOName = 0;
	Ray_VBOName_c = 0;

	RayVBOisBuilt = VBOisBuilt = MiniMapVBOisBuilt = false;

	circles = nullptr;
	markup = nullptr;
	info = nullptr;
	ray = nullptr;
}
CScene::~CScene() {
	ClearSelection();
	if (circles) {
		for (int c = 0; c < numCircles; c++)
			delete [] circles[c];
		delete[] circles;
	}
	if(markup)
		delete [] markup;
	if (Markup)
		delete Markup;

	if (mmPointer)
		delete mmPointer;

	if (ray)
		delete[] ray;
	if (AxisGrid)
		delete[] AxisGrid;
	if (AxisGridColor)
		delete[] AxisGridColor;
	if (Ray)
		delete[] Ray;
	if (RayColor)
		delete[] RayColor;
	if (Camera)
		delete Camera;

	if (info)
	{
		delete[] info;
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
	}

	if (ImageSet)
	{
		delete ImageSet;
	}

	if (RayObj)
	{
		delete RayObj;
	}
	if (Mesh) {
		delete Mesh;
	}
}

bool CScene::DrawScene(CViewPortControl * vpControl)
{
	if (MeshReady() && !VBOisBuilt) {
		PrepareVBOs();
		VBOisBuilt = BuildVBOs();
		Camera->Move(glm::vec3(0, 235, -310), false);
		Camera->RadarPosition = glm::vec3(0, GetY0(), 0);
	}
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
	if (MeshReady())
	{
		Mesh->UseTexture = vpControl->DisplayMap;
		Mesh->UseY0Loc = !vpControl->DisplayLandscape;
		Mesh->Draw(vpControl, GL_TRIANGLES);
	}
	glDisable(GL_DEPTH_TEST);
	if (Markup && UI && UI->GetCheckboxState_MarkupLines())
	{
		Markup->Draw(vpControl, 0);
	}

	if (begAzmLine) {
		begAzmLine->Draw(vpControl, GL_LINES);
	}


	if (UI->GetCheckboxState_MarkupLabels())
	{
		DrawBitmaps();
	}

	if (RayObj)
	{
		RayObj->Draw(vpControl, GL_TRIANGLES);
	}
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_PROGRAM_POINT_SIZE);
	if (UI->GetCheckboxState_Points()) {
		for (int i = 0; i < Sectors.size(); i++) {
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
			ImageSet->Draw(vpControl, GL_POINTS);
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
	if (RayObj)
	{
		RayObj->Draw(vpControl, GL_TRIANGLES);
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

	return true;
}

bool CScene::PrepareVBOs()
{	
	if (!Mesh || !Mesh->Ready()) {
		return false;
	}
	float y_0 = GetY0();

	
	if (!Markup) {
		Markup = new CMarkup(glm::vec4(0, y_0, 0, 1));
	}

	AxisGrid = new glm::vec3[vertexCount];

	int i0 = 0;
	//vertical axis

	AxisGrid[i0].x = 0; AxisGrid[i0].y = y_0, AxisGrid[i0].z = 0;
	AxisGrid[i0 + 1].x = 0; AxisGrid[i0 + 1].y = y_0 + YAxisLength, AxisGrid[i0 + 1].z = 0;
	i0 += 2;
	//horizontal axis
	for (int a = 0; a < (vertexCount_Axis - 2)/2 *10; a += 10) {
		AxisGrid[i0].x = maxDist * sin(cnvrt::dg2rad(a)) / MPPh; AxisGrid[i0].y = y_0, AxisGrid[i0].z = maxDist * cos(cnvrt::dg2rad(a)) / MPPh;
		AxisGrid[i0+1].x = -maxDist * sin(cnvrt::dg2rad(a)) / MPPh; AxisGrid[i0+1].y = y_0, AxisGrid[i0+1].z = -maxDist * cos(cnvrt::dg2rad(a)) / MPPh;
		i0+=2;
	}			
	
	//marks:
	for (int i = 0; i < markCount / 5; i+=1) {
		
		AxisGrid[i0 + 1].x = AxisGrid[i0].x = (i + 1) * markDistance / MPPh;
		AxisGrid[i0 + 1].y = AxisGrid[i0].y = y_0;
		AxisGrid[i0].z = -markSize / 2.0;
		AxisGrid[i0 + 1].z = markSize / 2.0;
		i0 += 2;
	}
	for (int i = 0; i < markCount / 5; i += 1) {

		AxisGrid[i0 + 1].x = AxisGrid[i0].x = -(i + 1) * markDistance / MPPh;
		AxisGrid[i0 + 1].y = AxisGrid[i0].y = y_0;
		AxisGrid[i0].z = -markSize / 2.0;
		AxisGrid[i0 + 1].z = markSize / 2.0;
		i0 += 2;
	}
	for (int i = 0; i < markCount / 5; i += 1) {

		AxisGrid[i0 + 1].z = AxisGrid[i0].z = (i + 1) * markDistance / MPPh;
		AxisGrid[i0 + 1].y = AxisGrid[i0].y = y_0;
		AxisGrid[i0].x = -markSize / 2.0;
		AxisGrid[i0 + 1].x = markSize / 2.0;
		i0 += 2;
	}
	for (int i = 0; i < markCount / 5; i += 1) {

		AxisGrid[i0 + 1].z = AxisGrid[i0].z = -(i + 1) * markDistance / MPPh;
		AxisGrid[i0 + 1].y = AxisGrid[i0].y = y_0;
		AxisGrid[i0].x = -markSize / 2.0;
		AxisGrid[i0 + 1].x = markSize / 2.0;
		i0 += 2;
	}
	for (int i = 0; i < markCount / 5; i += 1) {

		AxisGrid[i0 + 1].z = AxisGrid[i0].z = 0;
		AxisGrid[i0 + 1].y = AxisGrid[i0].y = y_0 + (i + 1) * markDistance / MPPv;
		AxisGrid[i0].x = -markSize / 2.0;
		AxisGrid[i0 + 1].x = markSize / 2.0;
		i0 += 2;
	}
	//circles:
	float R = 0;
	for (int c = 0; c < numCircles; c++) {
		R += markDistance * marksPerCircle;
		for (int i = 0; i < segmentsPerCircle; i++) {
			AxisGrid[i0].x = R * cos(2 * M_PI * i / segmentsPerCircle) / MPPh;
			AxisGrid[i0].y = y_0;
			AxisGrid[i0].z = R * sin(2 * M_PI * i / segmentsPerCircle) / MPPh;
			i0++;
		}
	}

	//infobox index array
	info = new unsigned short[vertexCount_Info];


	//infobox
	AxisGrid[i0].x = 0;
	AxisGrid[i0].y = 0;
	AxisGrid[i0].z = -1;
	info[0] = i0;

	i0++;
	AxisGrid[i0].x = infoWidth;
	AxisGrid[i0].y = 0;
	AxisGrid[i0].z = -1;
	info[1] = i0;

	i0++;
	AxisGrid[i0].x = infoWidth;
	AxisGrid[i0].y = infoHeight;
	AxisGrid[i0].z = -1;
	info[2] = i0;

	i0++;
	AxisGrid[i0].x = 0;
	AxisGrid[i0].y = infoHeight;
	AxisGrid[i0].z = -1;
	info[3] = i0;

	/*std::ofstream outfile("old.txt", std::ofstream::binary);
	for (int i = 0; i < i0; i++) {
		outfile << AxisGrid[i].x << ";" << AxisGrid[i].y << ";" << AxisGrid[i].z << "\r\n";
	}
	outfile.close();*/

	glm::vec4 axisColor = CSettings::GetColor(ColorAxis);

	//axis and markup color array
	AxisGridColor = new CColorRGBA[vertexCount];	
	for (int i = 0/*vertexCount_Ray*/; i < vertexCount - vertexCount_Info; i++) {
		AxisGridColor[i].r = axisColor.r;
		AxisGridColor[i].g = axisColor.g;
		AxisGridColor[i].b = axisColor.b;
		AxisGridColor[i].a = axisColor.a;
	}
	//info box color array
	for (int i = vertexCount - vertexCount_Info; i < vertexCount; i++) {
		AxisGridColor[i].r = 0.3f;
		AxisGridColor[i].g = 0.3f;
		AxisGridColor[i].b = 0.3f;
		AxisGridColor[i].a = 0.1f;
	}

	//ray color array
	RayColor = new CColorRGBA[vertexCount_Ray];
	for (int i = 0; i < vertexCount_Ray; i++) {
		RayColor[i].r = 1.0f;
		RayColor[i].g = 0.0f;
		RayColor[i].b = 0.0f;
		RayColor[i].a = 1.0f;
	}
	
	//index array for axis markup
	markup = new unsigned short[vertexCount/* - vertexCount_Ray*/];
	for (int i = 0; i < vertexCount/* - vertexCount_Ray*/; i++) {
		markup[i] = /*vertexCount_Ray +*/ i;
	}

	//index array for circles
	circles = new unsigned short*[numCircles];
	for (int c = 0; c < numCircles; c++) {
		circles[c] = new unsigned short[segmentsPerCircle];
		for (int i = 0; i < segmentsPerCircle; i++) {
			circles[c][i] = /*vertexCount_Ray +*/ vertexCount_Axis + markCount * 2 + segmentsPerCircle*c + i;
		}
	}

	return true;

}

bool CScene::PrepareRayVBO()
{
	if (!Mesh || !Mesh->Ready()) {
		return false;
	}
	//float y_0 = Mesh->GetCenterHeight() / MPPv;


		if (!RayObj)
		{
			//RayObj = new C3DObjectModel()
			RayObj = new CRay(rayWidth, maxDist, 0);
			return true;
		}

		
	return false;
}

bool CScene::MiniMapPrepareAndBuildVBO()
{

	return false;
}

bool CScene::BuildVBOs()
{
	if (AxisGrid && AxisGridColor) {
		glGenBuffers(1, &AxisGrid_VBOName);
		glBindBuffer(GL_ARRAY_BUFFER, AxisGrid_VBOName);
		glBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * sizeof(float), AxisGrid, GL_STATIC_DRAW);

		glGenBuffers(1, &AxisGrid_VBOName_c);
		glBindBuffer(GL_ARRAY_BUFFER, AxisGrid_VBOName_c);
		glBufferData(GL_ARRAY_BUFFER, vertexCount * 4 * sizeof(float), AxisGridColor, GL_STATIC_DRAW);

		return true;
	}

	return false;
}

bool CScene::BuildRayVBO()
{
	if (Ray && RayColor) {
		glGenBuffers(1, &Ray_VBOName);
		glBindBuffer(GL_ARRAY_BUFFER, Ray_VBOName);
		glBufferData(GL_ARRAY_BUFFER, vertexCount_Ray * 3 * sizeof(float), Ray, GL_STATIC_DRAW);
	
		glGenBuffers(1, &Ray_VBOName_c);
		glBindBuffer(GL_ARRAY_BUFFER, Ray_VBOName_c);
		glBufferData(GL_ARRAY_BUFFER, vertexCount_Ray * 4 * sizeof(float), RayColor, GL_STATIC_DRAW);

		delete Ray; Ray = nullptr;
		delete RayColor; RayColor = nullptr;

		return true;
	}
	
	return false;
}

void CScene::RefreshSector(RPOINTS * info_p, RPOINT * pts, RDR_INITCL* init)
{
	if (!Mesh || !Mesh->Ready()) {
		return;
	}
	float y_0 = GetY0();

	if (!info_p || !pts || !init)
		return; //do nothing if no RPOINTS structure provided
	if (init->MaxNAzm <= 0 || info_p->N<=0) //TODO: full defence against bad data need to be there
		return;

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

	if (Sectors[currentSector] == nullptr)
	{
		Sectors[currentSector] = new CSector(currentSector);
	}

	//LOG_INFO_("CScene::RefreshSector", "currentSector=%d", currentSector);
	if (RayObj) 
	{
		RayObj->SetRotateMatrix(glm::rotate((float)(- 2.0f * M_PI * (currentSector + 0.5) / SectorsCount), glm::vec3(0, 1, 0)));
	}

	MPPh = CSettings::GetFloat(FloatMPPh);
	MPPv = CSettings::GetFloat(FloatMPPv);
	Sectors[currentSector]->Refresh(glm::vec4(0, y_0, 0, 1), MPPh, MPPv, info_p, pts, init);	
	
	UI->FillInfoGrid(this);
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

void CScene::RefreshImages(RIMAGE* info, void* pixels)
{
	if (!Mesh || !Mesh->Ready()) {
		return;
	}
	float y_0 = GetY0();
	if (ImageSet)
	{
		ImageSet->Refresh(glm::vec4(0, y_0, 0, 1), MPPh, MPPv, rdrinit, info, pixels);
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
	
	begAzmLine = new CLine(Main, glm::vec4(0, y_0, 0, 1), glm::vec4(-r * sin(init->begAzm), y_0, r * cos(init->begAzm), 1), Simple);
	begAzmLine->SetName("begAzmLine");

	minE = init->begElv;
	maxE = init->begElv + init->dElv * init->Nelv;
	if (minE==0 && maxE==0)
	{
		minE = maxE = CSettings::GetFloat(FloatZeroElevation);
	}
	rayWidth = init->dAzm * init->ViewStep;
	maxDist = init->dR * init->maxR;
	SectorsCount = init->MaxNAzm / init->ViewStep;
	Sectors.resize(SectorsCount);
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

C3DObjectModel* CScene::GetPointOnSurface(glm::vec3 p0, glm::vec3 p1) const
{
	if (!Mesh || !Mesh->Ready()) {
		return nullptr;
	}

	std::string context = "CScene::GetPointOnSurface";
	LOG_INFO(requestID, context, (boost::format("Start... p0=(%1%, %2%, %3%), p1=(%4%, %5%, %6%)")
		% p0.x % p0.y % p0.z % p1.x % p1.y % p1.z).str().c_str());
	glm::vec3 dir = p1 - p0, position;
	Mesh->IntersectLine(Main, p0, dir, position);
	
	LOG_ERROR(requestID, context, "Not implemented!");
	return nullptr;
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
		BitmapString(-(maxDist + 50 * MPPh) * sin(cnvrt::dg2rad(a)) / MPPh, y_0, (maxDist + 50 * MPPh) * cos(cnvrt::dg2rad(a)) / MPPh, cnvrt::float2str(a) + "°");
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
	if (rdrinit && MeshReady())
	{
		rdrinit->begAzm = begAzm;
		
		int markDistance = CSettings::GetInt(IntMarkupMarkDistance);
		int numCircles = CSettings::GetInt(IntMarkupNumCircles);
		int marksPerCircle = CSettings::GetInt(IntMarkupMarksPerCircle);
		int r = markDistance * numCircles * marksPerCircle;

		if (begAzmLine)
			begAzmLine->SetPoints(glm::vec4(0, GetY0(), 0, 1), glm::vec4(- r * sin(rdrinit->begAzm), GetY0(), r * cos(rdrinit->begAzm), 1), Simple);
		else 
			begAzmLine = new CLine(Main, glm::vec4(0, GetY0(), 0, 1), glm::vec4(- r * sin(rdrinit->begAzm), GetY0(), r * cos(rdrinit->begAzm), 1), Simple);
	}
}

glm::vec3 CScene::GetGeographicCoordinates(glm::vec3 glCoords)
{
	return glm::vec3(position.x + glCoords.x * MPPh / cnvrt::londg2m(1, position.y), position.y + glCoords.z * MPPh / cnvrt::londg2m(1, position.y), glCoords.y);
}

void CScene::LoadMesh()
{
	CMesh *mesh = new CMesh(false, position, max_range, texsize, resolution, MPPh, MPPv);	
	waitingForMesh = true;
	Mesh = mesh;
}

glm::vec3 CScene::GetMeshSize() {
	if (Mesh) return Mesh->GetSize();
	return glm::vec3();
}