#pragma once
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut.h>

#include <string>

#include "CRCPoint.h"
#include "CScene.h"
#include "CMesh.h"

#include "CMinimapPointer.h"

#include "Util.h"
#define _USE_MATH_DEFINES 
#include <math.h>

#include "CCamera.h"
#include <vector>



/*CScene::CScene(float lonc, float latc) {
	geocenter.x = lonc;
	geocenter.y = latc;
}*/
CScene::CScene(std::string altFile, std::string imgFile, std::string datFile, float lonc, float latc, float mpph, float mppv, int texsize) {
	this->Camera = new CCamera();

	geocenter.x = lonc;
	geocenter.y = latc;

	this->mppv = mppv;
	this->mpph = mpph;

	this->altFile = altFile;
	this->imgFile = imgFile;
	this->datFile = datFile;

	this->texsize = texsize;

	mesh = new CMesh(this);

	markCount = 50*5;

	marksPerCircle = 10;

	numCircles = markCount / 5 / marksPerCircle;
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
	infoWidth = 370;
	infoHeight = 250;

	vertexCount = /*vertexCount_Ray + */vertexCount_Axis + markCount*2 + numCircles * segmentsPerCircle + vertexCount_Info;

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

	MiniMapPointer = new CMinimapPointer(this);
}
CScene::~CScene() {
	delete markup;
}

bool CScene::DrawScene()
{
	if (!VBOisBuilt) {
		PrepareVBOs();
		VBOisBuilt = BuildVBOs();
		Camera->SetAll(0, y0, 0, 200, mesh->m_Bounds[1].y + 300, -600, 0, 100, 0,
			60.0f, 4.0f / 3.0f, 1.0f, 10000.0f,
			0.01);
	}


	//goto shader_debug;
	mesh->Draw();

	
	
	glDisable(GL_DEPTH_TEST);									// Disable Depth Testing

	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, AxisGrid_VBOName);
	glVertexPointer(3, GL_FLOAT, 0, 0);

	glEnableClientState(GL_COLOR_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, AxisGrid_VBOName_c);
	glColorPointer(4, GL_FLOAT, 0, 0);		


	glDrawElements(GL_LINES, vertexCount_Axis + markCount * 2, GL_UNSIGNED_SHORT, markup);

	for (int c = 0; c < numCircles; c++) {
		glDrawElements(GL_LINE_LOOP, segmentsPerCircle, GL_UNSIGNED_SHORT, circles[c]);
	}

	glDisable(GL_LIGHTING);
	
	BitmapString(0, y0, 0, "(" + cnvrt::float2str(geocenter.x) + "; " + cnvrt::float2str(geocenter.y) + ")");
	glColor4f(1.0f, 1.0f, 0.0f, 0.7f);
	BitmapString(-10*markDistance/mpph, y0 + 1, 0, "1km");
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
		BitmapString(-(maxDist+50*mpph) * sin(cnvrt::dg2rad(a)) / mpph, y0, (maxDist + 50*mpph) * cos(cnvrt::dg2rad(a)) / mpph, cnvrt::float2str(a) + "°");
	}

	glColor3f(0.8f, 0.8f, 1.0f);
	BitmapString(0, y0 + markDistance, 60 * markDistance / mpph, "N");
	glColor3f(1.0f, 0.8f, 0.8f);
	BitmapString(0, y0 + markDistance, -60 * markDistance / mpph, "S");
	glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
	BitmapString(60 * markDistance / mpph, y0 + markDistance, 0, "W");
	BitmapString(-60 * markDistance / mpph, y0 + markDistance, 0, "E");

	glEnable(GL_DEPTH_TEST);

	if (UI->GetCheckboxState_Points()) {
		CRCPoint::UseProgram();
		for (int i = 0; i < Points.size(); i++) {
			Points[i].Draw(Camera);
		}
		glUseProgram(0);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glMatrixMode(GL_MODELVIEW);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);

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

bool CScene::DrawMiniMap()
{
	mesh->DrawMiniMap();
	MiniMapPointer->DrawMiniMap();
	return true;
}

bool CScene::PrepareVBOs()
{	
	mesh->LoadHeightmap();

	CRCPoint::PrepareVBO();

	AxisGrid = new CVec[vertexCount];
	

	glm::vec3 *b = meshBounds = mesh->GetBounds();

	//y0 - is a height of a radar, in OpenGl units ("pixels"), above the sea level
	switch (zeroLevel) {
	case ZEROLEVEL_ZERO:
		y0 = 0;
		break;
	case ZEROLEVEL_MAXHEIGHT:
		y0 = b[1].y;
		break;
	case ZEROLEVEL_ACTUALHEIGHT:
		y0 = mesh->centerHeight;
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

	

	//axis and markup color array
	AxisGridColor = new CColorRGBA[vertexCount];	
	for (int i = 0/*vertexCount_Ray*/; i < vertexCount - vertexCount_Info; i++) {
		AxisGridColor[i].r = 1.0f;
		AxisGridColor[i].g = 1.0f;
		AxisGridColor[i].b = 0.0f;
		AxisGridColor[i].a = 0.5f;
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

bool CScene::PrepareRayVBO(RDR_INITCL * init)
{
	rayWidth = init->dAzm * init->ViewStep;

	Ray = new CVec[vertexCount_Ray]; //ray vertex array
	int i0 = 0;

	float d_rayW = rayWidth / rayDensity; //one arc step - 'width' dimension
	float d_rayH = RAY_HEIGHT / rayDensity; // one arc step - 'height' dimension



	float a, e;
	int oneside = (rayDensity + 1) * (rayDensity + 1);
	rayArraySize = rayDensity*rayDensity * 6 * 2 + rayDensity * 6 * 4;
	
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

bool CScene::PrepareAndBuildMinimapVBO()
{

	return false;
}

bool CScene::BuildVBOs()
{
	mesh->BuildVBOs();

	CRCPoint::BuildVBO();

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
	if (!info_p)
		return; //do nothing if no RPOINTS structure provided

	viewAngle = glm::degrees(init->begAzm + init->dAzm * (info_p->d1 + info_p->d2) / 2);

	if (Ray_VBOName == 0 && Ray_VBOName_c == 0) {
		PrepareRayVBO(init);
		BuildRayVBO();
	}



	float a_start = init->begAzm + info_p->d1 * init->dAzm;
	float a_end = init->begAzm + info_p->d2 * init->dAzm;

	//removes all points in sector and sets up new points received from serfer
	for (vector<CRCPoint>::iterator it = Points.begin(); it != Points.end();)
	{
		if (it->SphericalCoords.y >= a_start && it->SphericalCoords.y <= a_end || it->SphericalCoords.y <= a_start && it->SphericalCoords.y >= a_end)
		{
			it = Points.erase(it);
		}
		else
		{
			++it;
		}
	}

	//add new points
	for (int i = 0; i < info_p->N; i++) {
		CRCPoint p(y0, mpph, mppv, pts[i].R * init->dR, init->begAzm + pts[i].B * init->dAzm, ZERO_ELEVATION + init->begElv + pts[i].E * init->dElv);
		Points.push_back(p);
		/*p.PrepareVBO();
		p.BuildVBO();*/
	}
}

void CScene::SetCameraPositionFromMiniMapXY(float x, float y, float direction) /* x and y from -1 to 1 */
{
	glm::vec3 *b = mesh->GetBounds();
	Camera->SetPositionXZ((b[0].x + b[1].x)/2 - x * (b[1].x - b[0].x)/2, (b[0].z + b[1].z) / 2 + y * (b[1].z - b[0].z) / 2);
}
C3DObject * CScene::GetObjectAtMiniMapPosition(float x, float y)
{
	glm::vec3 orig(x, y, 1);
	glm::vec3 dir(0, 0, 1);
	glm::vec3 pos;
	if (MiniMapPointer->IntersectLine(orig, dir, pos)) {
		return MiniMapPointer;
	}
	return NULL;
}

glm::vec2 CScene::CameraXYForMiniMap()
{
	glm::vec3 *b = mesh->GetBounds();
	if (b) {
		return glm::vec2(- 2 * (Camera->Position.x - b[0].x) / (b[1].x - b[0].x) + 1, -1 + 2 * (Camera->Position.z - b[0].z) / (b[1].z - b[0].z));
	}
	return glm::vec2(0);
}

