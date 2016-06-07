#pragma once
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut.h>

#include <string>

#include "CRCPoint.h"
#include "CTrack.h"
#include "CScene.h"
#include "CMesh.h"
#include "CMarkup.h"
#include "CSector.h"

#include "CMinimapPointer.h"

#include "Util.h"
#define _USE_MATH_DEFINES 
#include <math.h>

#include "CCamera.h"
#include <vector>
#include "CSettings.h"
#include "CViewPortControl.h"


/*CScene::CScene(float lonc, float latc) {
	geocenter.x = lonc;
	geocenter.y = latc;
}*/
CScene::CScene(std::string altFile, std::string imgFile, std::string datFile, float lonc, float latc, float mpph, float mppv, int texsize, mutex *m) {

	this->m = m;
	CRCPointModel::InitStructure();

	/*old_Sectors = NULL;*/
	SectorsCount = 0;

	this->Camera = new CCamera();

	geocenter.x = lonc;
	geocenter.y = latc;

	this->mppv = mppv;
	this->mpph = mpph;

	this->altFile = altFile;
	this->imgFile = imgFile;
	this->datFile = datFile;

	this->texsize = texsize;

	old_Mesh = new old_CMesh(this);

	m_Bounds = new glm::vec3[2];
	Mesh = new CMesh(Main, this, true, 0.5, 0.5);
	rcutils::takeminmax(Mesh->GetBounds()[0].x, &(m_Bounds[0].x), &(m_Bounds[1].x));
	rcutils::takeminmax(Mesh->GetBounds()[0].y, &(m_Bounds[0].y), &(m_Bounds[1].y));
	rcutils::takeminmax(Mesh->GetBounds()[0].z, &(m_Bounds[0].z), &(m_Bounds[1].z));
	rcutils::takeminmax(Mesh->GetBounds()[1].x, &(m_Bounds[0].x), &(m_Bounds[1].x));
	rcutils::takeminmax(Mesh->GetBounds()[1].y, &(m_Bounds[0].y), &(m_Bounds[1].y));
	rcutils::takeminmax(Mesh->GetBounds()[1].z, &(m_Bounds[0].z), &(m_Bounds[1].z));
	Mesh1 = new CMesh(Main, this, true, 0.5, -0.5);
	rcutils::takeminmax(Mesh1->GetBounds()[0].x, &(m_Bounds[0].x), &(m_Bounds[1].x));
	rcutils::takeminmax(Mesh1->GetBounds()[0].y, &(m_Bounds[0].y), &(m_Bounds[1].y));
	rcutils::takeminmax(Mesh1->GetBounds()[0].z, &(m_Bounds[0].z), &(m_Bounds[1].z));
	rcutils::takeminmax(Mesh1->GetBounds()[1].x, &(m_Bounds[0].x), &(m_Bounds[1].x));
	rcutils::takeminmax(Mesh1->GetBounds()[1].y, &(m_Bounds[0].y), &(m_Bounds[1].y));
	rcutils::takeminmax(Mesh1->GetBounds()[1].z, &(m_Bounds[0].z), &(m_Bounds[1].z));
	Mesh2 = new CMesh(Main, this, true, -0.5, -0.5);
	rcutils::takeminmax(Mesh2->GetBounds()[0].x, &(m_Bounds[0].x), &(m_Bounds[1].x));
	rcutils::takeminmax(Mesh2->GetBounds()[0].y, &(m_Bounds[0].y), &(m_Bounds[1].y));
	rcutils::takeminmax(Mesh2->GetBounds()[0].z, &(m_Bounds[0].z), &(m_Bounds[1].z));
	rcutils::takeminmax(Mesh2->GetBounds()[1].x, &(m_Bounds[0].x), &(m_Bounds[1].x));
	rcutils::takeminmax(Mesh2->GetBounds()[1].y, &(m_Bounds[0].y), &(m_Bounds[1].y));
	rcutils::takeminmax(Mesh2->GetBounds()[1].z, &(m_Bounds[0].z), &(m_Bounds[1].z));
	Mesh3 = new CMesh(Main, this, true, -0.5, 0.5);
	rcutils::takeminmax(Mesh3->GetBounds()[0].x, &(m_Bounds[0].x), &(m_Bounds[1].x));
	rcutils::takeminmax(Mesh3->GetBounds()[0].y, &(m_Bounds[0].y), &(m_Bounds[1].y));
	rcutils::takeminmax(Mesh3->GetBounds()[0].z, &(m_Bounds[0].z), &(m_Bounds[1].z));
	rcutils::takeminmax(Mesh3->GetBounds()[1].x, &(m_Bounds[0].x), &(m_Bounds[1].x));
	rcutils::takeminmax(Mesh3->GetBounds()[1].y, &(m_Bounds[0].y), &(m_Bounds[1].y));
	rcutils::takeminmax(Mesh3->GetBounds()[1].z, &(m_Bounds[0].z), &(m_Bounds[1].z));

	MeshSize = m_Bounds[1] - m_Bounds[0];

	Camera->MeshSize = Mesh->Size = Mesh1->Size = Mesh2->Size = Mesh3->Size = MeshSize;

	Mesh->Init(MiniMap);
	Mesh1->Init(MiniMap);
	Mesh2->Init(MiniMap);
	Mesh3->Init(MiniMap);

	mmPointer = new CMiniMapPointer(MiniMap, this);
	y0 = (Mesh->CenterHeight + Mesh1->CenterHeight + Mesh2->CenterHeight + Mesh3->CenterHeight) / 4 / mppv;
	Markup = new CMarkup(glm::vec4(0, y0, 0, 1));

	testPoint = new CRCPointModel(Main, y0, mpph, mppv, 0, 0, 0);
	testPoint->SetCartesianCoordinates(1000/mpph, y0, 0);

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
	vertexCount_Ray = (rayDensity + 1) * (rayDensity + 1) * 2;
	
	vertexCount_Axis = 38;
	
	vertexCount_Info = 4;
	

	vertexCount = vertexCount_Axis + markCount*2 + numCircles * segmentsPerCircle + vertexCount_Info;



	infoWidth = 370;
	infoHeight = 250;

	markSize = 10.0f;
	AxisGridShift = 250;
	viewAngle = 0;
	markDistance = 100;
	YAxisLength = (markCount / 5) * markDistance / mppv;

	RotatingSpeed = 120;	

	AxisGrid = NULL;
	AxisGridColor = NULL;
	Ray = NULL;
	RayColor = NULL;
	Ray_VBOName = 0;
	Ray_VBOName_c = 0;

	RayVBOisBuilt = VBOisBuilt = MiniMapVBOisBuilt = false;

	MiniMapPointer = new old_CMinimapPointer(this);

	circles = NULL;
	markup = NULL;
	info = NULL;
	ray = NULL;

	Initialized = false;
}
CScene::~CScene() {
	if (circles) {
		for (int c = 0; c < numCircles; c++)
			delete [] circles[c];
		delete[] circles;
	}
	if(markup)
		delete [] markup;
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
	if (old_Mesh)
		delete old_Mesh;
	if (MiniMapPointer)
		delete MiniMapPointer;
	if (info)
		delete[] info;
	/*if (old_Sectors) {
		for (int i = 0; i < SectorsCount; i++) {
			for (auto it = old_Sectors[i].begin(); it != old_Sectors[i].end(); ++it) {
				delete *it;
			}
			old_Sectors[i].clear();
		}
		
		delete[] old_Sectors;
	}*/
	Sectors.clear();
	Tracks.clear();
}

bool CScene::DrawScene(CViewPortControl * vpControl)
{
	//auto glstr = glGetString(GL_EXTENSIONS);

	if (!VBOisBuilt) {
		PrepareVBOs();
		VBOisBuilt = BuildVBOs();
		Camera->Move(glm::vec3(292, y0 + 235, 310), false);
		Camera->RadarPosition = glm::vec3(0, y0, 0);
	}
	//Mesh->Draw(vpControl, GL_TRIANGLES);
	//return true;
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//goto shader_debug;
	glDisable(GL_LIGHTING);
	//glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	//Mesh->UseTexture = vpControl->DisplayMap;
	glDisable(GL_LINE_SMOOTH);

	Mesh->UseTexture = vpControl->UI->GetCheckboxState_Map();
	Mesh->UseAltitudeMap = vpControl->UI->GetCheckboxState_AltitudeMap();
	Mesh->Draw(vpControl, GL_TRIANGLES);
	Mesh1->UseTexture = vpControl->UI->GetCheckboxState_Map();
	Mesh1->UseAltitudeMap = vpControl->UI->GetCheckboxState_AltitudeMap();
	Mesh1->Draw(vpControl, GL_TRIANGLES);
	Mesh2->UseTexture = vpControl->UI->GetCheckboxState_Map();
	Mesh2->UseAltitudeMap = vpControl->UI->GetCheckboxState_AltitudeMap();
	Mesh2->Draw(vpControl, GL_TRIANGLES);
	Mesh3->UseTexture = vpControl->UI->GetCheckboxState_Map();
	Mesh3->UseAltitudeMap = vpControl->UI->GetCheckboxState_AltitudeMap();
	Mesh3->Draw(vpControl, GL_TRIANGLES);
	//return false;
	glEnable(GL_PROGRAM_POINT_SIZE);
	if (UI->GetCheckboxState_Points()) {
		for (int i = 0; i < Sectors.size(); i++) {
			if (Sectors[i] != NULL)
				Sectors[i]->Draw(vpControl, GL_POINTS);
		}
	}
	//tracks:
	if (UI->GetCheckboxState_Tracks()) {
		for (auto it = Tracks.begin(); it != Tracks.end(); ++it)
		{
			it->second->Draw(vpControl, GL_POINTS);
		}
	}

	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glDisable(GL_DEPTH_TEST);
	if (UI->GetCheckboxState_MarkupLines())
	{
		Markup->Draw(vpControl, 0);
	}


	//testPoint->Draw(vpControl, GL_TRIANGLES);

	if (UI->GetCheckboxState_MarkupLabels()) 
	{
		DrawBitmaps();
	}

	//return false;
	//return false;
	//old_Mesh->Draw(Camera);
	glEnable(GL_LINE_SMOOTH);
	
	glDisable(GL_DEPTH_TEST);									// Disable Depth Testing

	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, AxisGrid_VBOName);
	glVertexPointer(3, GL_FLOAT, 0, 0);

	glEnableClientState(GL_COLOR_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, AxisGrid_VBOName_c);
	glColorPointer(4, GL_FLOAT, 0, 0);		


	/*glDrawElements(GL_LINES, vertexCount_Axis + markCount * 2, GL_UNSIGNED_SHORT, markup);

	for (int c = 0; c < numCircles; c++) {
		glDrawElements(GL_LINE_LOOP, segmentsPerCircle, GL_UNSIGNED_SHORT, circles[c]);
	}*/
	
	
	


	glEnable(GL_DEPTH_TEST);
	//Mesh->Draw(vpControl, GL_TRIANGLES);
	//points:
	/*if (UI->GetCheckboxState_Points()) {
		old_CRCPoint::UseProgram_s();
		for (int i = 0; i < SectorsCount; i++) {			
			for (vector<old_CRCPoint*>::iterator it = old_Sectors[i].begin(); it != old_Sectors[i].end(); ++it) {
				(*it)->Draw(Camera);
			}
		}
		
		glUseProgram(0);
	}*/
	//points:
	//Mesh->Draw(vpControl, GL_TRIANGLES);

	

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glMatrixMode(GL_MODELVIEW);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (Ray_VBOName == 0 && Ray_VBOName_c == 0) {
		PrepareRayVBO();
		BuildRayVBO();
	}
	if (Ray_VBOName > 0 && Ray_VBOName_c > 0 && Socket->IsConnected) {
		glRotatef(-viewAngle, 0.0f, 1.0f, 0.0f);



		glEnableClientState(GL_VERTEX_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, Ray_VBOName);
		glVertexPointer(3, GL_FLOAT, 0, 0);

		glEnableClientState(GL_COLOR_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, Ray_VBOName_c);
		glColorPointer(4, GL_FLOAT, 0, 0);

		//glDrawArrays(GL_TRIANGLES, 0, 3);
		glDrawElements(GL_LINE_LOOP, rayArraySize, GL_UNSIGNED_SHORT, ray);
	}
	//glRasterPos3f

	
	glLoadIdentity();
	

	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//glOrtho(0, width, height, 0, -1, 1);

	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glMatrixMode(GL_MODELVIEW);
	//glDrawElements(GL_POLYGON, vertexCount_Info, GL_UNSIGNED_SHORT, info);

	glDisableClientState(GL_VERTEX_ARRAY);						// Enable Vertex Arrays
	glDisableClientState(GL_COLOR_ARRAY);


	/*glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	BitmapString2D(10, 20, "RadarClient DEMO version 0.001");

	BitmapString2D(10, 60, "RadarPosition (lon, lat):"); 
	BitmapString2D(200, 60, "(" + cnvrt::float2str(geocenter.x) + "; " + cnvrt::float2str(geocenter.y) + ")");

	BitmapString2D(10, 80, "Min. elevation:"); 
	BitmapString2D(200, 80, cnvrt::float2str(minE) + " DEG");
						  
	BitmapString2D(10, 100, "Max. elevation:"); 
	BitmapString2D(200, 100, cnvrt::float2str(maxE) + " DEG");

	BitmapString2D(10, 120, "Ray width:"); 
	BitmapString2D(200, 120, cnvrt::float2str(rayWidth) + " DEG");

	BitmapString2D(10, 150, "Min. distance:");
	BitmapString2D(200, 150, cnvrt::float2str(minDist) + " m");

	BitmapString2D(10, 170, "Max. distance:");
	BitmapString2D(200, 170, cnvrt::float2str(maxDist) + " m");

	BitmapString2D(10, 200, "Azimuth:");
	BitmapString2D(200, 200, cnvrt::float2str(viewAngle) + " DEG");

	BitmapString2D(10, 220, "Angular velocity:");
	BitmapString2D(200, 220, cnvrt::float2str(RotatingSpeed/360.0f) + " RPS");*/

//shader_debug:
	//glEnable(GL_DEPTH_TEST);

	

	return true;
}

bool CScene::MiniMapDraw(CViewPortControl * vpControl)
{
	//old_Mesh->MiniMapDraw(Camera);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	Mesh->Draw(vpControl, GL_TRIANGLES);
	Mesh1->Draw(vpControl, GL_TRIANGLES);
	Mesh2->Draw(vpControl, GL_TRIANGLES);
	Mesh3->Draw(vpControl, GL_TRIANGLES);

	//MiniMapPointer->MiniMapDraw(Camera);
	glEnable(GL_PROGRAM_POINT_SIZE);
	if (UI->GetCheckboxState_Points()) {
		for (int i = 0; i < Sectors.size(); i++) {
			if (Sectors[i] != NULL)
				Sectors[i]->Draw(vpControl, GL_POINTS);
		}
	}
	if (UI->GetCheckboxState_Tracks()) {
		for (auto it = Tracks.begin(); it != Tracks.end(); ++it)
		{
			it->second->Draw(vpControl, GL_POINTS);
		}
	}
	glDisable(GL_DEPTH_BUFFER);
	Markup->Draw(vpControl, 0);
	//glEnable(GL_DEPTH_BUFFER);
	mmPointer->Draw(vpControl, GL_TRIANGLES);
	return true;
}

bool CScene::PrepareVBOs()
{	
	old_Mesh->LoadHeightmap();
	
	old_CRCPoint::PrepareVBO_s();

	AxisGrid = new glm::vec3[vertexCount];
	

	glm::vec3 *b = meshBounds = old_Mesh->GetBounds();

	//y0 - is a height of a radar, in OpenGl units ("pixels"), above the sea level
	switch (zeroLevel) {
	case ZEROLEVEL_ZERO:
		y0 = 0;
		break;
	case ZEROLEVEL_MAXHEIGHT:
		y0 = b[1].y;
		break;
	case ZEROLEVEL_ACTUALHEIGHT:
		y0 = old_Mesh->CenterHeight;
		break;
	}

	



	int i0 = 0;
	//vertical axis

	AxisGrid[i0].x = 0; AxisGrid[i0].y = y0, AxisGrid[i0].z = 0;
	AxisGrid[i0 + 1].x = 0; AxisGrid[i0 + 1].y = y0 + YAxisLength, AxisGrid[i0 + 1].z = 0;
	i0 += 2;
	//horizontal axis
	for (int a = 0; a < (vertexCount_Axis - 2)/2 *10; a += 10) {
		AxisGrid[i0].x = maxDist * sin(cnvrt::dg2rad(a)) / mpph; AxisGrid[i0].y = y0, AxisGrid[i0].z = maxDist * cos(cnvrt::dg2rad(a)) / mpph;
		AxisGrid[i0+1].x = -maxDist * sin(cnvrt::dg2rad(a)) / mpph; AxisGrid[i0+1].y = y0, AxisGrid[i0+1].z = -maxDist * cos(cnvrt::dg2rad(a)) / mpph;
		i0+=2;
	}			
	
	//marks:
	for (int i = 0; i < markCount / 5; i+=1) {
		
		AxisGrid[i0 + 1].x = AxisGrid[i0].x = (i + 1) * markDistance / mpph;
		AxisGrid[i0 + 1].y = AxisGrid[i0].y = y0;
		AxisGrid[i0].z = -markSize / 2.0;
		AxisGrid[i0 + 1].z = markSize / 2.0;
		i0 += 2;
	}
	for (int i = 0; i < markCount / 5; i += 1) {

		AxisGrid[i0 + 1].x = AxisGrid[i0].x = -(i + 1) * markDistance / mpph;
		AxisGrid[i0 + 1].y = AxisGrid[i0].y = y0;
		AxisGrid[i0].z = -markSize / 2.0;
		AxisGrid[i0 + 1].z = markSize / 2.0;
		i0 += 2;
	}
	for (int i = 0; i < markCount / 5; i += 1) {

		AxisGrid[i0 + 1].z = AxisGrid[i0].z = (i + 1) * markDistance / mpph;
		AxisGrid[i0 + 1].y = AxisGrid[i0].y = y0;
		AxisGrid[i0].x = -markSize / 2.0;
		AxisGrid[i0 + 1].x = markSize / 2.0;
		i0 += 2;
	}
	for (int i = 0; i < markCount / 5; i += 1) {

		AxisGrid[i0 + 1].z = AxisGrid[i0].z = -(i + 1) * markDistance / mpph;
		AxisGrid[i0 + 1].y = AxisGrid[i0].y = y0;
		AxisGrid[i0].x = -markSize / 2.0;
		AxisGrid[i0 + 1].x = markSize / 2.0;
		i0 += 2;
	}
	for (int i = 0; i < markCount / 5; i += 1) {

		AxisGrid[i0 + 1].z = AxisGrid[i0].z = 0;
		AxisGrid[i0 + 1].y = AxisGrid[i0].y = y0 + (i + 1) * markDistance / mppv;
		AxisGrid[i0].x = -markSize / 2.0;
		AxisGrid[i0 + 1].x = markSize / 2.0;
		i0 += 2;
	}
	//circles:
	float R = 0;
	for (int c = 0; c < numCircles; c++) {
		R += markDistance * marksPerCircle;
		for (int i = 0; i < segmentsPerCircle; i++) {
			AxisGrid[i0].x = R * cos(2 * M_PI * i / segmentsPerCircle) / mpph;
			AxisGrid[i0].y = y0;
			AxisGrid[i0].z = R * sin(2 * M_PI * i / segmentsPerCircle) / mpph;
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
		RayColor[i].g = 1.0f;
		RayColor[i].b = 1.0f;
		RayColor[i].a = 0.2f;
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
	if (!Initialized)
		return false;

	Ray = new glm::vec3[vertexCount_Ray]; //ray vertex array
	int i0 = 0;

	float d_rayW = rayWidth / rayDensity; //one arc step - 'width' dimension
	float d_rayH = RAY_HEIGHT / rayDensity; // one arc step - 'height' dimension



	float a, e;
	int oneside = (rayDensity + 1) * (rayDensity + 1);
	rayArraySize = rayDensity*rayDensity * 6 * 2 + rayDensity * 6 * 4;
	

	float ZERO_ELEVATION = glm::radians(CSettings::GetFloat(FloatZeroElevation));
	a = -rayWidth / 2;
	for (int i = 0; i <= rayDensity; i++) {
		e = ZERO_ELEVATION - RAY_HEIGHT/2.0f;
		for (int j = 0; j <= rayDensity; j++) {

			Ray[i0].x = minDist * cos(e) * sin(a) / mpph;
			Ray[i0].y = y0 + minDist * sin(e) / mppv;
			Ray[i0].z = minDist * cos(e) * cos(a) / mpph;
			Ray[oneside + i0].x = maxDist * cos(e) * sin(a) / mpph;
			Ray[oneside + i0].y = y0 + maxDist * sin(e) / mppv;
			Ray[oneside + i0].z = maxDist * cos(e) * cos(a) / mpph;
			i0++;
			e += d_rayH;
		}
		a += d_rayW;
	}

	ray = new unsigned short[rayArraySize]; //ray index array
	int k1 = 0, k2 = 6 * rayDensity * rayDensity, k3 = 6 * rayDensity * rayDensity * 2;
	for (int i = 0; i < rayDensity; i++) {
		for (int j = 0; j < rayDensity; j++) {
			ray[k1++] = i * (rayDensity + 1) + j;
			ray[k1++] = i * (rayDensity + 1) + j + 1;
			ray[k1++] = (i + 1) * (rayDensity + 1) + j;
			ray[k1++] = (i + 1) * (rayDensity + 1) + j;
			ray[k1++] = i * (rayDensity + 1) + j + 1;
			ray[k1++] = (i + 1) * (rayDensity + 1) + j + 1;


			ray[k2++] = oneside + i * (rayDensity + 1) + j;
			ray[k2++] = oneside + i * (rayDensity + 1) + j + 1;
			ray[k2++] = oneside + (i + 1) * (rayDensity + 1) + j;
			ray[k2++] = oneside + (i + 1) * (rayDensity + 1) + j;
			ray[k2++] = oneside + i * (rayDensity + 1) + j + 1;
			ray[k2++] = oneside + (i + 1) * (rayDensity + 1) + j + 1;
		}
		ray[k3++] = i * (rayDensity + 1);
		ray[k3++] = oneside + (i + 1) * (rayDensity + 1);
		ray[k3++] = oneside + i * (rayDensity + 1);
		ray[k3++] = i * (rayDensity + 1);
		ray[k3++] = (i + 1) * (rayDensity + 1);
		ray[k3++] = oneside + (i + 1) * (rayDensity + 1);

		ray[k3++] = rayDensity * (rayDensity + 1) + i;
		ray[k3++] = rayDensity * (rayDensity + 1) + i + 1;
		ray[k3++] = oneside + rayDensity * (rayDensity + 1) + i;
		ray[k3++] = oneside + rayDensity * (rayDensity + 1) + i;
		ray[k3++] = rayDensity * (rayDensity + 1) + i + 1;
		ray[k3++] = oneside + rayDensity * (rayDensity + 1) + i + 1;

		ray[k3++] = (rayDensity + 1) * i + rayDensity;
		ray[k3++] = oneside + (rayDensity + 1) * i + rayDensity;
		ray[k3++] = (rayDensity + 1) * (i + 1) + rayDensity;
		ray[k3++] = (rayDensity + 1) * (i + 1) + rayDensity;
		ray[k3++] = oneside + (rayDensity + 1) * i + rayDensity;
		ray[k3++] = oneside + (rayDensity + 1) * (i + 1) + rayDensity;

		ray[k3++] = i;
		ray[k3++] = oneside + i;
		ray[k3++] = oneside + i + 1;
		ray[k3++] = i;
		ray[k3++] = oneside + i + 1;
		ray[k3++] = i + 1;
	}
	return true;
}

bool CScene::MiniMapPrepareAndBuildVBO()
{

	return false;
}

bool CScene::BuildVBOs()
{
	old_Mesh->BuildVBOs();

	old_CRCPoint::BuildVBO_s();

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

		delete Ray; Ray = NULL;
		delete RayColor; RayColor = NULL;

		return true;
	}
	
	return false;
}

void CScene::RefreshSector(RPOINTS * info_p, RPOINT * pts, RDR_INITCL* init)
{
	if (!info_p || !pts || !init)
		return; //do nothing if no RPOINTS structure provided
	if (init->Nazm <= 0 || info_p->N<=0) //TODO: full defence against bad data need to be there
		return;

	//Init = init;
	if (!Initialized)
		Init(init);	

	viewAngle = glm::degrees(init->begAzm + init->dAzm * (info_p->d1 + info_p->d2) / 2);


	
	/*if(!old_Sectors) {
		if (init->Nazm % init->ViewStep != 0) {
			return;
			//throw new std::exception("invalid data");
		}
		SectorsCount = init->Nazm / init->ViewStep;
		old_Sectors = new std::vector<old_CRCPoint*>[SectorsCount];
	}*/

	if (info_p->d1 == info_p->d2)
		return;

	float min = std::fmin(info_p->d1, info_p->d2);

	if (min < 0)
		return;

	if (std::fmax(info_p->d1, info_p->d2) > init->Nazm)
		return;

	if (SectorsCount <= 0)
		return;
	
	int currentSector = SectorsCount * (min / init->Nazm);

	if (Sectors[currentSector] == NULL)
	{
		Sectors[currentSector] = new CSector();
	}

	Sectors[currentSector]->Refresh(glm::vec4(0, y0, 0, 1), mpph, mppv, info_p, pts, init);
	
	
	UI->FillInfoGrid(this);
}

void CScene::ClearSectors()
{
	/*if (!old_Sectors)
		return;
	for (int i = 0; i < SectorsCount; i++) {
		old_Sectors[i].clear();
	}
	delete[] old_Sectors;
	old_Sectors = NULL;*/

	Sectors.clear();

	SectorsCount = 0;
}

void CScene::RefreshTracks(vector<TRK*>* tracks)
{	
	if(tracks->size() == 0)
		return;

	m->lock();

	for (int i = 0; i < tracks->size(); i++)
	{
		bool insertNew = true;
		for (auto it = Tracks.begin(); it != Tracks.end(); ++it)
		{
			if (it->first == tracks->at(i)->id) {
				it->second->Found = tracks->at(i)->Found = true;
				it->second->Refresh(glm::vec4(0, y0, 0, 1), mpph, mppv, &tracks->at(i)->P);
				insertNew = false;
			}
		}
		if (insertNew)
		{
			CTrack *t = new CTrack(tracks->at(i)->id);
			t->Refresh(glm::vec4(0, y0, 0, 1), mpph, mppv, &tracks->at(i)->P);
			t->Found = true;
			Tracks.insert_or_assign(tracks->at(i)->id, t);			
		}
	}
	for (auto it = Tracks.begin(); it != Tracks.end();) {
		if (!it->second->Found) {
			it = Tracks.erase(it);
		}
		else {
			it->second->Found = false;
			++it;
		}
	}
	UI->FillInfoGrid(this);

	m->unlock();
}

void CScene::Init(RDR_INITCL* init)
{
	if (!init)
		return;
	minE = init->begElv;
	maxE = init->begElv + init->dElv;
	rayWidth = init->dAzm * init->ViewStep;
	SectorsCount = init->Nazm / init->ViewStep;
	Sectors.resize(SectorsCount);
	Initialized = true;
}

CRCPointModel * CScene::GetCRCPointFromRDRTRACK(RDRTRACK * tp) const
{
	//TODO: need common formula here
	float r = 0.75 * sqrt(tp->X * tp->X + tp->Y * tp->Y) / 1;
	float a = glm::radians(-120 + 0.02 * 1000 * (atan(tp->X / tp->Y)) / (M_PI / 180));
	float e = glm::radians(30.0f);
	CRCPointModel* p = new CRCPointModel(Main, y0, mpph, mppv, r, a, e);
	p->Color = CSettings::GetColor(ColorTrack);
	return p;
}

void CScene::SetCameraPositionFromMiniMapXY(float x, float y, float direction) const
/* x and y from -1 to 1 */
{
	glm::vec3 *b = m_Bounds;
	Camera->SetPositionXZ((b[0].x + b[1].x)/2 - x * (b[1].x - b[0].x)/2, (b[0].z + b[1].z) / 2 + y * (b[1].z - b[0].z) / 2);
}
C3DObjectModel * CScene::GetObjectAtMiniMapPosition(int vpId, glm::vec3 p0, glm::vec3 p1) const
{
	glm::vec3 orig = p0;
	glm::vec3 dir = p1 - p0;
	glm::vec3 pos;
	if (mmPointer->IntersectLine(vpId, orig, dir, pos)) {
		return mmPointer;
	}
	return NULL;
}
C3DObjectModel * CScene::GetSectorPoint(CViewPortControl *vpControl, glm::vec2 screenPoint, int& index)
{
	/*if (MiniMapPointer->MiniMapIntersectLine(orig, dir, pos)) {
		return MiniMapPointer;	
	}*/	
	for (int i = 0; i < Sectors.size(); i++)
	{
		if (Sectors[i])
		{
			index = Sectors[i]->GetPoint(vpControl, screenPoint);
			if (index >= 0)
			{
				Sectors[i]->SelectPoint(Main, index);
				Sectors[i]->SelectPoint(MiniMap, index);
			}
		}
	}
	return NULL;	
}

C3DObjectModel* CScene::GetFirstTrackBetweenPoints(int vpId, glm::vec3 p0, glm::vec3 p1) const
{
	return NULL;
}

old_C3DObject* CScene::GetPointOnSurface(glm::vec3 p0, glm::vec3 p1) const
{
	glm::vec3 dir = p1 - p0, position;
	old_Mesh->IntersectLine(p0, dir, position);
	return NULL;
}

glm::vec2 CScene::CameraXYForMiniMap() const
{
	glm::vec3 *b = old_Mesh->GetBounds();
	if (b) {
		return glm::vec2(- 2 * (Camera->GetPosition().x - b[0].x) / (b[1].x - b[0].x) + 1, -1 + 2 * (Camera->GetPosition().z - b[0].z) / (b[1].z - b[0].z));
	}
	return glm::vec2(0);
}

void CScene::DrawBitmaps() const
{
	BitmapString(0, y0, 0, "(" + cnvrt::float2str(geocenter.x) + "; " + cnvrt::float2str(geocenter.y) + ")");
	glColor4f(1.0f, 1.0f, 0.0f, 0.7f);
	BitmapString(-10 * markDistance / mpph, y0 + 1, 0, "1km");
	BitmapString(10 * markDistance / mpph, y0 + 1, 0, "1km");
	BitmapString(0, y0 + 1, -10 * markDistance / mpph, "1km");
	BitmapString(0, y0 + 1, 10 * markDistance / mpph, "1km");
	BitmapString(0, y0 + 10 * markDistance / mppv, 0, "1km");

	BitmapString(-20 * markDistance / mpph, y0 + 1, 0, "2km");
	BitmapString(20 * markDistance / mpph, y0 + 1, 0, "2km");
	BitmapString(0, y0 + 1, -20 * markDistance / mpph, "2km");
	BitmapString(0, y0 + 1, 20 * markDistance / mpph, "2km");
	BitmapString(0, y0 + 20 * markDistance / mppv, 0, "2km");

	BitmapString(-30 * markDistance / mpph, y0 + 1, 0, "3km");
	BitmapString(30 * markDistance / mpph, y0 + 1, 0, "3km");
	BitmapString(0, y0 + 1, -30 * markDistance / mpph, "3km");
	BitmapString(0, y0 + 1, 30 * markDistance / mpph, "3km");
	BitmapString(0, y0 + 30 * markDistance / mppv, 0, "3km");

	BitmapString(-40 * markDistance / mpph, y0 + 1, 0, "4km");
	BitmapString(40 * markDistance / mpph, y0 + 1, 0, "4km");
	BitmapString(0, y0 + 1, -40 * markDistance / mpph, "4km");
	BitmapString(0, y0 + 1, 40 * markDistance / mpph, "4km");
	BitmapString(0, y0 + 40 * markDistance / mppv, 0, "4km");

	BitmapString(-50 * markDistance / mpph, y0 + 1, 0, "5km");
	BitmapString(50 * markDistance / mpph, y0 + 1, 0, "5km");
	BitmapString(0, y0 + 1, -50 * markDistance / mpph, "5km");
	BitmapString(0, y0 + 1, 50 * markDistance / mpph, "5km");
	BitmapString(0, y0 + 50 * markDistance / mppv, 0, "5km");

	for (int a = 0; a < 360; a += 10) {
		BitmapString(-(maxDist + 50 * mpph) * sin(cnvrt::dg2rad(a)) / mpph, y0, (maxDist + 50 * mpph) * cos(cnvrt::dg2rad(a)) / mpph, cnvrt::float2str(a) + "°");
	}

	glColor3f(0.8f, 0.8f, 1.0f);
	BitmapString(0, y0 + markDistance, 60 * markDistance / mpph, "N");
	glColor3f(1.0f, 0.8f, 0.8f);
	BitmapString(0, y0 + markDistance, -60 * markDistance / mpph, "S");
	glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
	BitmapString(60 * markDistance / mpph, y0 + markDistance, 0, "W");
	BitmapString(-60 * markDistance / mpph, y0 + markDistance, 0, "E");
}
