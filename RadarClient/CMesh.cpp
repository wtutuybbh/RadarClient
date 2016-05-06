#pragma once
#include <fstream>
#include <iostream>

#include <GL/glew.h>
#include <GL/glut.h>

#include <Windows.h>
#include "CMesh.h"

#include "Util.h"



#include "glm/glm.hpp"

#include "CScene.h"

#include "Util.h"

#include <float.h>

#define DATFILE_MAXLINELENGTH 256



CMesh::CMesh(CScene *scn) : C3DObject(true)
{
	// Set Pointers To NULL
	aMap = NULL;
	m_pVertices = NULL;
	m_pTexCoords = NULL;
	m_Bounds = NULL;
	iMapH = NULL;
	aMapH = NULL;
	m_nVertexCount = 0;
	MiniMapVBOName = MiniMapVAOName = MiniMapProgramID = m_nVBOVertices = m_nVBOTexCoords = m_nTextureId = 0;
	MiniMapVBOReady = false;

	this->scn = scn;
	this->texsize = scn->texsize;

	//this->MiniMapDrawMethodsSequence[4] = (PtrToMethod)(&CMesh::BindTextureImage);
}

CMesh :: ~CMesh()
{
	// Delete VBOs
	unsigned int nBuffers[2] = { m_nVBOVertices, m_nVBOTexCoords };
	glDeleteBuffers(2, nBuffers);						// Free The Memory
	
	// Delete Data
	if (m_pVertices)											// Deallocate Vertex Data
		delete[] m_pVertices;
	m_pVertices = NULL;
	if (m_pTexCoords)											// Deallocate Texture Coord Data
		delete[] m_pTexCoords;
	m_pTexCoords = NULL;

	if (m_Bounds)
		delete[] m_Bounds;
	if (iMapH)
		delete iMapH;
	if (aMapH) {
		if (aMapH->fileName)
			delete[] aMapH->fileName;		
		delete aMapH;
	}
	if (aMap) {
		if (aMap->data)
			delete[] aMap->data;
		delete aMap;
	}
}
glm::vec3 * CMesh::GetBounds() {
	return m_Bounds;
}
ImageMapHeader* CMesh::GetImageMapHeader(const char *imgFile, const char *datFile) {
	
	try {
		bitmap = FreeImage_Load(FreeImage_GetFileType(imgFile, 0), imgFile);
	}
	catch (...) {
		return false;
	}
	//access to data: ((FIBITMAP *)bitmap)->data
	
	std::ifstream infile;

	try {
		infile.open(datFile, std::ifstream::in | std::ifstream::binary);
	}
	catch (...) {
		
		return false;
	}

	if (!infile) {
		return false;
	}
	char line[DATFILE_MAXLINELENGTH];
	infile.getline(line, DATFILE_MAXLINELENGTH); //useless 1st line

	double imgLon0=180, imgLat0=90, imgLon1=-180, imgLat1=-90, v;
	char *pch;
	while (infile.good()) {
		infile.getline(line, DATFILE_MAXLINELENGTH); 
		if (strlen(line) > 0 && line[0] != '(') {
			pch = strtok(line, ",");
			v = atof(pch);
			if (v < imgLon0)
				imgLon0 = v;
			if (v > imgLon1)
				imgLon1 = v;
			pch = strtok(NULL, ",");
			v = atof(pch);
			if (v < imgLat0)
				imgLat0 = v;
			if (v > imgLat1)
				imgLat1 = v;
		}
		
	}
	iMapH = new ImageMapHeader;
	iMapH->imgLon0 = imgLon0;
	iMapH->imgLat0 = imgLat0;
	iMapH->imgLon1 = imgLon1;
	iMapH->imgLat1 = imgLat1;
	iMapH->sizeX = FreeImage_GetWidth((FIBITMAP*)bitmap);
	iMapH->sizeY = FreeImage_GetHeight((FIBITMAP*)bitmap);

	return iMapH;
}
void CMesh::Draw(CCamera *cam)
{
	glEnable(GL_DEPTH_TEST);									// Enable Depth Testing
	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_VERTEX_ARRAY);						// Enable Vertex Arrays
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);				// Enable Texture Coord Arrays
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glColor3f(1.0f, 1.0f, 1.0f);
	// Set Pointers To Our Data
	glBindBuffer(GL_ARRAY_BUFFER, m_nVBOVertices);
	glVertexPointer(3, GL_FLOAT, 0, (char *)NULL);		// Set The Vertex Pointer To The Vertex Buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_nVBOTexCoords);
	glTexCoordPointer(2, GL_FLOAT, 0, (char *)NULL);		// Set The TexCoord Pointer To The TexCoord Buffer

															// Render
	glDrawArrays(GL_TRIANGLES, 0, m_nVertexCount);
	glDisableClientState(GL_VERTEX_ARRAY);					// Disable Vertex Arrays
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);				// Disable Texture Coord Arrays
	glDisable(GL_TEXTURE_2D);
}

void CMesh::MiniMapBindTextureImage()
{
	int minimapTexSize = 512;

	MiniMapImage = FreeImage_Rescale(subimage, minimapTexSize, minimapTexSize, FILTER_BSPLINE);
	// Get An Open ID




	glGenTextures(1, &MiniMapTextureId);
	glBindTexture(GL_TEXTURE_2D, MiniMapTextureId);


	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, minimapTexSize, minimapTexSize, 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(MiniMapImage));

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
void CMesh::MiniMapDraw(CCamera *cam)
{
	if (!MiniMapVBOReady) {
		MiniMapVBOReady = MiniMapPrepareAndBuildVBO("Minimap.v.glsl", "Minimap.f.glsl", NULL);
	}	
	
	C3DObject::MiniMapDraw(cam);
}
AltitudeMapHeader* CMesh::GetAltitudeMapHeader(const char *fileName, double lon1, double lat1, double lon2, double lat2) {
	HINSTANCE hDLL;               // Handle to DLL
	GDPALTITUDEMAP_SIZES gdpAltitudeMap_Sizes;    // Function pointer
	double LL[10];
	int size[8], result;
	short *data;


	hDLL = LoadLibrary("GeoDataProvider.dll");
	if (hDLL != NULL)
	{
		gdpAltitudeMap_Sizes = (GDPALTITUDEMAP_SIZES)GetProcAddress(hDLL, "gdpAltitudeMap_Sizes");
		if (!gdpAltitudeMap_Sizes)
		{
			FreeLibrary(hDLL);
			return NULL;
		}
		else
		{
			LL[0] = lon1;
			LL[1] = lat1;
			LL[2] = lon2;
			LL[3] = lat2;
			result = gdpAltitudeMap_Sizes(fileName, LL, size);
			if (result == 0) {
				if (!aMapH) {
					aMapH = new AltitudeMapHeader;
					aMapH->fileName = NULL;
				}
				if (aMapH->fileName)
					delete[] aMapH->fileName;
				aMapH->fileName = new char[strlen(fileName) + 1];				
				strncpy(aMapH->fileName, fileName, strlen(fileName));
				aMapH->fileName[strlen(fileName)] = 0;
				aMapH->sizeX = size[0];
				aMapH->sizeY = size[1];
				aMapH->iLonStart = size[2];
				aMapH->iLatStart = size[3];
				aMapH->iLonEnd = size[4];
				aMapH->iLatEnd = size[5];
				aMapH->Nlon = size[6];
				aMapH->Nlat = size[7];
				aMapH->lon0 = LL[4];
				aMapH->lat0 = LL[5];
				aMapH->dlon = LL[6];
				aMapH->dlat = LL[7];
				aMapH->lonSW = LL[8];
				aMapH->latSW = LL[9];
				return aMapH;
			}
		}
		FreeLibrary(hDLL);
	}
	return NULL;
}
AltitudeMap* CMesh::GetAltitudeMap(const char *fileName, double lon1, double lat1, double lon2, double lat2) {
	HINSTANCE hDLL;               // Handle to DLL
	GDPALTITUDEMAP gdpAltitudeMap;    // Function pointer
	GDPALTITUDEMAP_SIZES gdpAltitudeMap_Sizes;    // Function pointer
	double LL[10];
	int size[8], result;
	short *data;

	

	hDLL = LoadLibrary("GeoDataProvider.dll");
	if (hDLL != NULL)
	{
		gdpAltitudeMap = (GDPALTITUDEMAP)GetProcAddress(hDLL, "gdpAltitudeMap");
		gdpAltitudeMap_Sizes = (GDPALTITUDEMAP_SIZES)GetProcAddress(hDLL, "gdpAltitudeMap_Sizes");
		if (!gdpAltitudeMap)
		{
			FreeLibrary(hDLL);
			return NULL;
		}
		else
		{
			LL[0] = lon1;
			LL[1] = lat1;
			LL[2] = lon2;
			LL[3] = lat2;
			result = gdpAltitudeMap_Sizes(fileName, LL, size);
			if (result == 0) {
				if (!aMapH) {
					aMapH = new AltitudeMapHeader;
					aMapH->fileName = NULL;
				}
				if (aMapH->fileName)
					delete[] aMapH->fileName;
				aMapH->fileName = new char[strlen(fileName)+1];
				strncpy(aMapH->fileName, fileName, strlen(fileName)); 
				aMapH->fileName[strlen(fileName)] = 0;
				aMapH->sizeX = size[0];
				aMapH->sizeY = size[1];
				aMapH->iLonStart = size[2];
				aMapH->iLatStart = size[3];
				aMapH->iLonEnd = size[4];
				aMapH->iLatEnd = size[5];
				aMapH->Nlon = size[6];
				aMapH->Nlat = size[7];
				aMapH->lon0 = LL[4];
				aMapH->lat0 = LL[5];
				aMapH->dlon = LL[6];
				aMapH->dlat = LL[7];
				aMapH->lonSW = LL[8];
				aMapH->latSW = LL[9];
				short *data;
				data = new short[size[0] * size[1]];
				if (data) {
					result = gdpAltitudeMap(fileName, LL, size, data);
					if (result == 0) {
						aMap = new AltitudeMap;
						aMap->sizeX = size[0];
						aMap->sizeY = size[1];
						aMap->data = data;

						FreeLibrary(hDLL);

						return aMap;
					}
				}
			}
		}
		FreeLibrary(hDLL);
	}
	return NULL;
}
bool CMesh::LoadHeightmap()
{
		
	//float flHeightScale = 2.0;


	iMapH = GetImageMapHeader(scn->imgFile.data(), scn->datFile.data());
	if (!iMapH)
		return false;
	aMapH = GetAltitudeMapHeader(scn->altFile.data(), iMapH->imgLon0, iMapH->imgLat0, iMapH->imgLon1, iMapH->imgLat1);

	if (!aMapH)
		return false;

	int cx = iMapH->sizeX * (scn->geocenter.x - iMapH->imgLon0) / (iMapH->imgLon1 - iMapH->imgLon0);
	int cy = iMapH->sizeY * (scn->geocenter.y - iMapH->imgLat0) / (iMapH->imgLat1 - iMapH->imgLat0);

	int top = iMapH->sizeY - cy - texsize / 2 + 1;
	int left = cx - texsize / 2 + 1;
	int bottom = top + texsize;
	int right = left + texsize;

	subimage = FreeImage_Copy((FIBITMAP*)bitmap, left, top, right, bottom);

	double px = (iMapH->imgLon1 - iMapH->imgLon0) / iMapH->sizeX;
	double py = (iMapH->imgLat1 - iMapH->imgLat0) / iMapH->sizeY;

	//int w = FreeImage_GetWidth((FIBITMAP*)subimage), h = FreeImage_GetHeight((FIBITMAP*)subimage);

	aMap = GetAltitudeMap(scn->altFile.data(), scn->geocenter.x - px * texsize /2, scn->geocenter.y - py * texsize / 2, scn->geocenter.x + px * texsize / 2, scn->geocenter.y + py * texsize / 2);

	//std::ofstream outfile("new.txt", std::ofstream::binary);

	
	// Generate Vertex Field
	m_nVertexCount = (int)((aMap->sizeX - 1) * (aMap->sizeY - 1) * 6 );
	m_pVertices = new CVec[m_nVertexCount];						// Allocate Vertex Data
	m_pTexCoords = new CTexCoord[m_nVertexCount];				// Allocate Tex Coord Data

	

	int nX, nZ, nTri, nIndex = 0;									// Create Variables
	float flX, flZ;
//	char buff[100];
	short maxheight = 0, minheight = 10000;
	for (int i = 0; i < aMap->sizeX * aMap->sizeY; i++) {
		if (aMap->data[i] > maxheight) maxheight = aMap->data[i];
		if (aMap->data[i] < minheight) minheight = aMap->data[i];
	}
	m_Bounds = new glm::vec3[2];
	m_Bounds[0].x = m_Bounds[0].y = m_Bounds[0].z = FLT_MAX;
	m_Bounds[1].x = m_Bounds[1].y = m_Bounds[1].z = FLT_MIN;

	lonStretch = aMapH->dlon * cnvrt::londg2m(1, aMapH->latSW + aMapH->dlat * aMapH->Nlat / 2.0) / scn->mpph;
	latStretch = aMapH->dlat * cnvrt::latdg2m(1, aMapH->latSW + aMapH->dlat * aMapH->Nlat / 2.0) / scn->mpph;

	for (nZ = 0; nZ < aMap->sizeY - 1; nZ ++)
	{
		for (nX = 0; nX < aMap->sizeX - 1; nX ++)
		{
			//sprintf(buff, "%d;", aMap->data[((nX % aMap->sizeX) * aMap->sizeY + ((nZ % aMap->sizeY) ))]);
			//outfile.write(buff, strlen(buff));			
			for (nTri = 0; nTri < 6; nTri++)
			{
				// Using This Quick Hack, Figure The X,Z Position Of The Point
				flX = (float)nX +((nTri == 1 || nTri == 2 || nTri == 5) ? 1.0f : 0.0f);
				flZ = (float)nZ +((nTri == 2 || nTri == 4 || nTri == 5) ? 1.0f : 0.0f);

				// Set The Data, Using PtHeight To Obtain The Y Value
				

				m_pVertices[nIndex].x = lonStretch * ( -flX + (aMap->sizeX / 2.0));
				rcutils::takeminmax(m_pVertices[nIndex].x, &(m_Bounds[0].x), &(m_Bounds[1].x));
				
				m_pVertices[nIndex].y = (PtHeight((int)flX, (int)flZ) /*- (maxheight+minheight)/2*/) / scn->mppv;
				rcutils::takeminmax(m_pVertices[nIndex].y, &(m_Bounds[0].y), &(m_Bounds[1].y));

				m_pVertices[nIndex].z = latStretch * (flZ - (aMap->sizeY / 2.0));
				rcutils::takeminmax(m_pVertices[nIndex].z, &(m_Bounds[0].z), &(m_Bounds[1].z));

				// Stretch The Texture Across The Entire Mesh
				m_pTexCoords[nIndex].u = flX / aMap->sizeX;
				m_pTexCoords[nIndex].v = flZ / aMap->sizeY;

				// Increment Our Index
				nIndex++;
			}
		}
		//buff[0] = endl;
		//outfile << std::endl;
		//outfile.write(buff, 2);
	}
	//outfile.close();
	// Load The Texture Into OpenGL
	glGenTextures(1, &m_nTextureId);							// Get An Open ID
	glBindTexture(GL_TEXTURE_2D, m_nTextureId);				// Bind The Texture

	
	//ofstream outfile 
	//unsigned char * tmpdata = new unsigned char[aMap->sizeX * aMap->sizeY * 3];

	/*for (int i = 0; i < aMap->sizeX * aMap->sizeY; i++) {
		tmpdata[i * 3] = aMap->data[i]*255.0 / maxheight;
		tmpdata[i * 3 + 1] = tmpdata[i * 3];
		tmpdata[i * 3 + 2] = tmpdata[i * 3];
	}*/

	glTexImage2D(GL_TEXTURE_2D, 0, 3, texsize, texsize, 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(subimage));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//delete tmpdata;
	// Free The Altitude Data

	centerHeight = PtHeight(aMap->sizeX / 2, aMap->sizeY / 2)  /scn->mppv;

	
	return true;
}



float CMesh::PtHeight(int nX, int nY)
{
	// Calculate The Position In The Texture, Careful Not To Overflow
	int nPos = ((nX % aMap->sizeX) * aMap->sizeY + ((nY % aMap->sizeY) ));
	return (float)aMap->data[nPos];
}

void CMesh::BuildVBOs()
{
	// Generate And Bind The Vertex Buffer
	glGenBuffers(1, &m_nVBOVertices);							// Get A Valid Name
	glBindBuffer(GL_ARRAY_BUFFER, m_nVBOVertices);			// Bind The Buffer
																	// Load The Data
	glBufferData(GL_ARRAY_BUFFER, m_nVertexCount * 3 * sizeof(float), m_pVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// Generate And Bind The Texture Coordinate Buffer
	glGenBuffers(1, &m_nVBOTexCoords);							// Get A Valid Name
	glBindBuffer(GL_ARRAY_BUFFER, m_nVBOTexCoords);		// Bind The Buffer
																// Load The Data
	glBufferData(GL_ARRAY_BUFFER, m_nVertexCount * 2 * sizeof(float), m_pTexCoords, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	// Our Copy Of The Data Is No Longer Necessary, It Is Safe In The Graphics Card
	delete[] m_pVertices; m_pVertices = NULL;
	delete[] m_pTexCoords; m_pTexCoords = NULL;
}
