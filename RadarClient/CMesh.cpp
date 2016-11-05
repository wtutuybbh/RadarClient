#include "stdafx.h"
#include "CMesh.h"
#include "Util.h"
#include "CScene.h"

#define DATFILE_MAXLINELENGTH 256

#include "CViewPortControl.h"
#include "CSettings.h"
#include "C3DObjectProgram.h"
#include "C3DObjectVBO.h"
#include "C3DObjectTexture.h"
#include "CRCLogger.h"

float CMesh::Y0;

//int CMesh::TotalVertexCount;
bool CMesh::LoadHeightmap(int vpId)
{
	string context = "CMesh::LoadHeightmap(int vpId)";
	CRCLogger::Info(context, "Start, vpId=" + vpId);

	iMapH = GetImageMapHeader(scn->imgFile.data(), scn->datFile.data());
	if (!iMapH)
		return false;
	aMapH = GetAltitudeMapHeader(scn->altFile.data(), iMapH->imgLon0, iMapH->imgLat0, iMapH->imgLon1, iMapH->imgLat1);

	if (!aMapH)
		return false;

	int cx = iMapH->sizeX * (scn->geocenter.x - iMapH->imgLon0) / (iMapH->imgLon1 - iMapH->imgLon0);
	int cy = iMapH->sizeY * (scn->geocenter.y - iMapH->imgLat0) / (iMapH->imgLat1 - iMapH->imgLat0);

	int top = iMapH->sizeY - cy - texsize / 2 + 1 - texsize * shiftZ;
	int left = cx - texsize / 2 + 1 - texsize * shiftX;
	int bottom = top + texsize;
	int right = left + texsize;

	subimage = FreeImage_Copy((FIBITMAP*)bitmap, left, top, right, bottom);

	double px = (iMapH->imgLon1 - iMapH->imgLon0) / iMapH->sizeX;
	double py = (iMapH->imgLat1 - iMapH->imgLat0) / iMapH->sizeY;

	aMap = GetAltitudeMap(scn->altFile.data(),
		scn->geocenter.x - px * texsize / 2 - px * texsize * shiftX, 
		scn->geocenter.y - py * texsize / 2 + py * texsize * shiftZ, 
		scn->geocenter.x + px * texsize / 2 - px * texsize * shiftX, 
		scn->geocenter.y + py * texsize / 2 + py * texsize * shiftZ);

	if (!aMap)
		return false;
	//std::ofstream outfile("new.txt", std::ofstream::binary);


	// Generate Vertex Field
	//int m_nVertexCount = (int)((aMap->sizeX - 1) * (aMap->sizeY - 1) * 6);
	//	m_pVertices = new CVec[m_nVertexCount];						// Allocate Vertex Data
	//m_pVertices = new glm::vec3[m_nVertexCount];						// Allocate Vertex Data
																		//m_pTexCoords = new CTexCoord[m_nVertexCount];				// Allocate Tex Coord Data
	//m_pTexCoords = new glm::vec2[m_nVertexCount];				// Allocate Tex Coord Data

	std::vector<VBOData> * buffer = new std::vector<VBOData> ((aMap->sizeX - 1) * (aMap->sizeY - 1) * 6);


	int nX, nZ, nTri, nIndex = 0;									// Create Variables
	float flX, flZ;
	//	char buff[100];
	short maxheight = 0, minheight = 10000;
	for (int i = 0; i < aMap->sizeX * aMap->sizeY; i++) {
		if (aMap->data[i] > maxheight) maxheight = aMap->data[i];
		if (aMap->data[i] < minheight) minheight = aMap->data[i];
	}
	Bounds = new glm::vec3[2];
	Bounds[0].x = Bounds[0].y = Bounds[0].z = FLT_MAX;
	Bounds[1].x = Bounds[1].y = Bounds[1].z = FLT_MIN;

	float lonStretch = aMapH->dlon * cnvrt::londg2m(1, aMapH->latSW + aMapH->dlat * aMapH->Nlat / 2.0) / scn->mpph;
	float latStretch = aMapH->dlat * cnvrt::latdg2m(1, aMapH->latSW + aMapH->dlat * aMapH->Nlat / 2.0) / scn->mpph;

	float absoluteShiftX = shiftX * lonStretch * (aMap->sizeX - 1);
	float absoluteShiftZ = shiftZ * latStretch * (aMap->sizeY - 1);

	LocalAverageHeight = 0;

	VBOData tmp;
	float minh = CSettings::GetFloat(FloatMinAltitude), maxh = CSettings::GetFloat(FloatMaxAltitude), h, level;
	glm::vec4 mincolor = CSettings::GetColor(ColorAltitudeLowest), maxcolor = CSettings::GetColor(ColorAltitudeHighest);
	for (nZ = 0; nZ < aMap->sizeY - 1; nZ++)
	{
		for (nX = 0; nX < aMap->sizeX - 1; nX++)
		{
			//sprintf(buff, "%d;", aMap->data[((nX % aMap->sizeX) * aMap->sizeY + ((nZ % aMap->sizeY) ))]);
			//outfile.write(buff, strlen(buff));			
			for (nTri = 0; nTri < 6; nTri++)
			{
				// Using This Quick Hack, Figure The X,Z Position Of The Point
				flX = (float)nX + ((nTri == 1 || nTri == 2 || nTri == 5) ? 1.0f : 0.0f);
				flZ = (float)nZ + ((nTri == 2 || nTri == 4 || nTri == 5) ? 1.0f : 0.0f);

				// Set The Data, Using PtHeight To Obtain The Y Value
				h = PtHeight((int)flX, (int)flZ);
				level = (h - minh) / (maxh - minh);
				tmp = {
					glm::vec4(
						absoluteShiftX + lonStretch * (-flX + aMap->sizeX / 2.0),
						h / scn->mppv,
						absoluteShiftZ + latStretch * (flZ - aMap->sizeY / 2.0),
						1),
					glm::vec3(0, 1, 0),
					mincolor * (1-level) + maxcolor * level,
					glm::vec2(flX / aMap->sizeX, flZ / aMap->sizeY) };
				
				buffer->push_back(tmp);

				rcutils::takeminmax(tmp.vert.x, &(Bounds[0].x), &(Bounds[1].x));
				rcutils::takeminmax(tmp.vert.y, &(Bounds[0].y), &(Bounds[1].y));
				rcutils::takeminmax(tmp.vert.z, &(Bounds[0].z), &(Bounds[1].z));

				/*m_pVertices[nIndex].x = lonStretch * (-flX + (aMap->sizeX / 2.0));
				rcutils::takeminmax(m_pVertices[nIndex].x, &(m_Bounds[0].x), &(m_Bounds[1].x));

				m_pVertices[nIndex].y = (PtHeight((int)flX, (int)flZ) ) / scn->mppv;
				rcutils::takeminmax(m_pVertices[nIndex].y, &(m_Bounds[0].y), &(m_Bounds[1].y));

				m_pVertices[nIndex].z = latStretch * (flZ - (aMap->sizeY / 2.0));
				rcutils::takeminmax(m_pVertices[nIndex].z, &(m_Bounds[0].z), &(m_Bounds[1].z));

				// Stretch The Texture Across The Entire Mesh
				m_pTexCoords[nIndex].x = flX / aMap->sizeX;
				m_pTexCoords[nIndex].y = flZ / aMap->sizeY;*/

				if (nTri == 0)
					LocalAverageHeight += tmp.vert.y;
				// Increment Our Index
				nIndex++;


			}
		}
		//buff[0] = endl;
		//outfile << std::endl;
		//outfile.write(buff, 2);
	}
	LocalAverageHeight /= aMap->sizeY * aMap->sizeX;

	prog.insert_or_assign(vpId, new C3DObjectProgram("CMesh.vert", "CMesh.frag", "vertex", "texcoor", NULL, "color"));
	//prog->CreateProgram();
	
	/*for (auto it = buffer->begin(); it != buffer->end(); ++it)
	{
		float level = ((*it).vert.y - m_Bounds[0].y) / (m_Bounds[1].y - m_Bounds[0].y);
		(*it).color = CSettings::GetColor(ColorAltitudeLowest)*(1-level) + CSettings::GetColor(ColorAltitudeHighest)*level;
		(*it).color.a = 0.95;
	}*/


	vbo.insert_or_assign(vpId, new C3DObjectVBO(clearAfter));
	vbo.at(vpId)->SetBuffer(buffer, &(*buffer)[0], buffer->size());

	if (FreeImage_GetBPP(subimage) != 32)
	{
		FIBITMAP* tempImage = subimage;
		subimage = FreeImage_ConvertTo32Bits(tempImage);
	}
	tex.insert_or_assign(vpId, new C3DObjectTexture(subimage, "tex", true, false));


	//outfile.close();
	// Load The Texture Into OpenGL
	/*glGenTextures(1, &m_nTextureId);							// Get An Open ID
	glBindTexture(GL_TEXTURE_2D, m_nTextureId);*/				// Bind The Texture


															//ofstream outfile 
															//unsigned char * tmpdata = new unsigned char[aMap->sizeX * aMap->sizeY * 3];

															/*for (int i = 0; i < aMap->sizeX * aMap->sizeY; i++) {
															tmpdata[i * 3] = aMap->data[i]*255.0 / maxheight;
															tmpdata[i * 3 + 1] = tmpdata[i * 3];
															tmpdata[i * 3 + 2] = tmpdata[i * 3];
															}*/

	/*glTexImage2D(GL_TEXTURE_2D, 0, 3, texsize, texsize, 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(subimage));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/

	//delete tmpdata;
	// Free The Altitude Data

	CenterHeight = PtHeight(min(max(aMap->sizeX / 2 + aMap->sizeX * shiftX - 1, 0), aMap->sizeX - 1), min(max(aMap->sizeY / 2 - aMap->sizeY * shiftZ, 0), aMap->sizeY-1));
	Y0 = CenterHeight / scn->mppv;


	return true;
}

AltitudeMap* CMesh::GetAltitudeMap(const char* fileName, double lon1, double lat1, double lon2, double lat2)
{
	string context = "CMesh::GetAltitudeMap";
	CRCLogger::Info(context, (boost::format("Start: fileName=%1%, lon1=%2%, lat1=%3%, lon2=%4%, lat2=%5%")
		% fileName
		% lon1
		% lat1
		% lon2
		% lat2).str());

	HINSTANCE hDLL;               // Handle to DLL
	GDPALTITUDEMAP gdpAltitudeMap;    // Function pointer
	GDPALTITUDEMAP_SIZES gdpAltitudeMap_Sizes;    // Function pointer
	double LL[10];
	int size[8], result;



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

			CRCLogger::Info(context, (boost::format("aMapH overwrite? sizeX=%1%, sizeY=%2%, iLonStart=%3%, iLatStart=%4%, iLonEnd=%5%, iLatEnd=%6%, Nlon=%7%, Nlat=%8%, lon0=%9%, lat0=%10%, dlon=%11%, dlat=%12%, lonSW=%13%, latSW=%14%")
				% aMapH->sizeX % aMapH->sizeY % aMapH->iLonStart % aMapH->iLatStart % aMapH->iLonEnd % aMapH->iLatEnd % aMapH->Nlon % aMapH->Nlat % aMapH->lon0	% aMapH->lat0 % aMapH->dlon	% aMapH->dlat % aMapH->lonSW % aMapH->latSW).str());

			short *data;
			data = new short[size[0] * size[1]];
			
			if (data) {
				CRCLogger::Info(context, (boost::format("size of data is %1%")
					% (size[0] * size[1])).str());
				result = gdpAltitudeMap(fileName, LL, size, data);
				if (result == 0) {
					aMap = new AltitudeMap;
					aMap->sizeX = size[0];
					aMap->sizeY = size[1];
					aMap->data = data;
					FreeLibrary(hDLL);
					CRCLogger::Info(context, "Return, success.");
					return aMap;
				}
			}
			CRCLogger::Error(context, (boost::format("new short[size[0] * size[1]] failed. (size[0] * size[1] = %1%)")
				% (size[0] * size[1])).str());
		}
		FreeLibrary(hDLL);
	}
	CRCLogger::Error(context, "LoadLibrary failed.");
	return NULL;
}

AltitudeMapHeader* CMesh::GetAltitudeMapHeader(const char* fileName, double lon1, double lat1, double lon2, double lat2)
{
	string context = "CMesh::GetAltitudeMapHeader";
	CRCLogger::Info(context, (boost::format("Start: fileName=%1%, lon1=%2%, lat1=%3%, lon2=%4%, lat2=%5%")
		% fileName
		% lon1
		% lat1
		% lon2
		% lat2).str());

	HINSTANCE hDLL;               // Handle to DLL
	GDPALTITUDEMAP_SIZES gdpAltitudeMap_Sizes;    // Function pointer
	double LL[10];
	int size[8], result;

	hDLL = LoadLibrary("GeoDataProvider.dll");
	if (hDLL != NULL)
	{
		gdpAltitudeMap_Sizes = (GDPALTITUDEMAP_SIZES)GetProcAddress(hDLL, "gdpAltitudeMap_Sizes");
		if (!gdpAltitudeMap_Sizes)
		{
			FreeLibrary(hDLL);
			return NULL;
		}
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

			CRCLogger::Info(context, (boost::format("Return: sizeX=%1%, sizeY=%2%, iLonStart=%3%, iLatStart=%4%, iLonEnd=%5%, iLatEnd=%6%, Nlon=%7%, Nlat=%8%, lon0=%9%, lat0=%10%, dlon=%11%, dlat=%12%, lonSW=%13%, latSW=%14%")
				% aMapH->sizeX % aMapH->sizeY % aMapH->iLonStart % aMapH->iLatStart % aMapH->iLonEnd % aMapH->iLatEnd % aMapH->Nlon % aMapH->Nlat % aMapH->lon0 % aMapH->lat0 % aMapH->dlon % aMapH->dlat % aMapH->lonSW % aMapH->latSW).str());

			return aMapH;
		}
		CRCLogger::Error(context, "gdpAltitudeMap_Sizes returned 0.");
		FreeLibrary(hDLL);
	}
	CRCLogger::Error(context, "LoadLibrary failed.");
	return NULL;
}

ImageMapHeader* CMesh::GetImageMapHeader(const char* imgFile, const char* datFile)
{
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

	double imgLon0 = 180, imgLat0 = 90, imgLon1 = -180, imgLat1 = -90, v;
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

float CMesh::PtHeight(int nX, int nY) const
{
	// Calculate The Position In The Texture, Careful Not To Overflow
	int nPos = ((nX % aMap->sizeX) * aMap->sizeY + ((nY % aMap->sizeY)));
	return (float)aMap->data[nPos];
}

CMesh::CMesh(int vpId, CScene* scn, bool clearAfter, float shiftX, float shiftZ) : C3DObjectModel(vpId, NULL, NULL, NULL)
{
	std::string context = "CMesh::CMesh";
	CRCLogger::Info(context, (boost::format("Start: vpId=%1%, scn=%2%, clearAfter=%3%, shiftX=%4%, shiftZ=%5%") 
		% vpId 
		% scn 
		% clearAfter 
		% shiftX 
		% shiftZ).str());

	this->scn = scn;
	aMap = NULL;
	Bounds = NULL;
	iMapH = NULL;
	aMapH = NULL;
	this->texsize = scn->texsize;
	this->clearAfter = clearAfter;
	this->shiftX = shiftX;
	this->shiftZ = shiftZ;
	LoadHeightmap(vpId);
	UseTexture = 1;
}

CMesh::~CMesh()
{
	for (auto it = begin(vbo); it != end(vbo); ++it)
	{
		vector<VBOData> *buffer = (vector<VBOData>*)it->second->GetBuffer();
		delete buffer;
	}
	if (Bounds)
		delete[] Bounds;
	if (aMap) {
		if (aMap->data)
			delete[] aMap->data;		
		delete aMap;
	}
	if (aMapH)
	{
		if (aMapH->fileName)
			delete[] aMapH->fileName;
		delete aMapH;
	}
	if (iMapH)
		delete iMapH;
}

bool CMesh::IntersectLine(int vpId, glm::vec3& orig_, glm::vec3& dir_, glm::vec3& position_)
{
	string context = "CMesh::IntersectLine";
	CRCLogger::Info(context, (boost::format("Start: vpId=%1%, orig_=(%2%, %3%, %4%), dir_=(%5%, %6%, %7%")
		% vpId
		% orig_.x
		% orig_.y
		% orig_.z
		% dir_.x
		% dir_.y
		% dir_.z).str());

	glm::vec4 planeOrig(0, AverageHeight, 0, 1), planeNormal(0, 1, 0, 0);
	float distance;
	glm::vec4 orig(orig_, 1);
	glm::vec4 dir(dir_, 0);
	glm::vec4 position;

	bool planeResult = glm::intersectRayPlane(orig, dir, planeOrig, planeNormal, distance);
	glm::vec4 approxPoint = orig + distance * dir;
	CMesh *m;
	glm::vec3 *b;
	for (int i = 0; i < CMesh::TotalMeshsCount; i++) {
		m = CMesh::Meshs[i];
		b = m->Bounds;
		//find appropriate part of surface by testing bounds:
		if (approxPoint.x > b[0].x && approxPoint.z > b[0].z && approxPoint.x <= b[1].x && approxPoint.z <= b[1].z) {			
			break;
		}
	}
	
	//now work with pointer m:
	//grid coordinates:
	int ix0 = -aMap->sizeX * (approxPoint.x - b[1].x) / (b[1].x - b[0].x);
	int iy0 = aMap->sizeY * (approxPoint.z - b[0].z) / (b[1].z - b[0].z);
	if (ix0 < 0 || ix0 >= aMap->sizeX || iy0 < 0 || iy0 >= aMap->sizeY) {
		ix0 = aMap->sizeX / 2;
		iy0 = aMap->sizeY / 2;
	}
	// 2. test triangles around approximate intersection point

	vector<VBOData> *buffer = (vector<VBOData> *)m->GetC3DObjectVBO(Main)->GetBuffer();

	int X = aMap->sizeX - 1, Y = aMap->sizeY - 1;
	/*
	if (glm::intersectLineTriangle(orig, dir, buffer->at(6 * (iy0 * X + ix0)).vert, buffer->at(6 * (iy0 * X + ix0) + 1).vert, buffer->at(6 * (iy0 * X + ix0) + 2).vert, position))
		return true;
	if (glm::intersectLineTriangle(orig, dir, buffer->at(6 * (iy0 * X + ix0) + 3).vert, buffer->at(6 * (iy0 * X + ix0) + 4).vert, buffer->at(6 * (iy0 * X + ix0) + 5).vert, position))
		return true;

	int level = 1;
	int x, y;
	bool found = false;
	

	while (ix0 - level >= 0 || ix0 + level < X || iy0 - level >= 0 || iy0 + level < Y)
	{
		if (iy0 - level >= 0) {
			y = iy0 - level;
			for (x = max(ix0 - level, 0); x < min(ix0 + level, X); x++)
			{
				if (glm::intersectLineTriangle(orig, dir, buffer->at(6 * (y * X + x)).vert, buffer->at(6 * (y * X + x) + 1).vert, buffer->at(6 * (y * X + x) + 2).vert, position))
				{
					found = true; break;
				}
				if (glm::intersectLineTriangle(orig, dir, buffer->at(6 * (y * X + x) + 3).vert, buffer->at(6 * (y * X + x) + 4).vert, buffer->at(6 * (y * X + x) + 5).vert, position))
				{
					found = true; break;
				}
			}
			if (found) break;
		}
		if (ix0 + level < X) {
			x = ix0 + level;
			for (y = max(iy0 - level, 0); y < min(iy0 + level, Y); y++)
			{
				if (glm::intersectLineTriangle(orig, dir, buffer->at(6 * (y * X + x)).vert, buffer->at(6 * (y * X + x) + 1).vert, buffer->at(6 * (y * X + x) + 2).vert, position))
				{
					found = true; break;
				}
				if (glm::intersectLineTriangle(orig, dir, buffer->at(6 * (y * X + x) + 3).vert, buffer->at(6 * (y * X + x) + 4).vert, buffer->at(6 * (y * X + x) + 5).vert, position))
				{
					found = true; break;
				}
			}
			if (found) break;
		}
		if (iy0 + level < Y)
		{
			y = iy0 + level;
			for (x = min(ix0 + level, X - 1); x > max(ix0 - level, 0); x--)
			{
				if (glm::intersectLineTriangle(orig, dir, buffer->at(6 * (y * X + x)).vert, buffer->at(6 * (y * X + x) + 1).vert, buffer->at(6 * (y * X + x) + 2).vert, position))
				{
					found = true; break;
				}
				if (glm::intersectLineTriangle(orig, dir, buffer->at(6 * (y * X + x) + 3).vert, buffer->at(6 * (y * X + x) + 4).vert, buffer->at(6 * (y * X + x) + 5).vert, position))
				{
					found = true; break;
				}
			}
			if (found) break;
		}
		if (ix0 - level >= 0)
		{
			x = ix0 - level;
			for (y = min(iy0 + level, Y - 1); y > max(iy0 - level, 0); y--)
			{
				if (glm::intersectLineTriangle(orig, dir, buffer->at(6 * (y * X + x)).vert, buffer->at(6 * (y * X + x) + 1).vert, buffer->at(6 * (y * X + x) + 2).vert, position))
				{
					found = true; break;
				}
				if (glm::intersectLineTriangle(orig, dir, buffer->at(6 * (y * X + x) + 3).vert, buffer->at(6 * (y * X + x) + 4).vert, buffer->at(6 * (y * X + x) + 5).vert, position))
				{
					found = true; break;
				}
			}
			if (found) break;
		}
		level++;
	}
	level++;
	position = position / position.w;
	position_.x = position.x;
	position_.y = position.y;
	position_.z = position.z;
	return found;*/
	return false;
}

void CMesh::BindUniforms(CViewPortControl* vpControl)
{
	glm::mat4 m = GetModelMatrix(vpControl);
	glm::mat4 v = vpControl->GetViewMatrix();
	glm::mat4 p = vpControl->GetProjMatrix();
	glm::mat4 mvp = p*v*m;
	int mvp_loc = prog.at(vpControl->Id)->GetUniformLocation("mvp");

	glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, glm::value_ptr(mvp));

	if (vpControl->Id == Main) {		
		int useTexture_loc = prog.at(vpControl->Id)->GetUniformLocation("useTexture");
		int y0_loc = prog.at(vpControl->Id)->GetUniformLocation("y0");
		int usey0_loc = prog.at(vpControl->Id)->GetUniformLocation("useY0");

		glUniform1i(useTexture_loc, UseTexture);
		glUniform1i(usey0_loc, UseAltitudeMap);
		glUniform1f(y0_loc, Y0);
		
	}
}

glm::vec3 * CMesh::GetBounds()
{
	return Bounds;
}

void CMesh::Init(int vpId)
{
	if (vpId == MiniMap)
	{
		C3DObjectVBO *newvbo = new C3DObjectVBO(false);
		
		std::vector<VBOData> *buffer = new std::vector<VBOData>;
		float y = 0;// (m_Bounds[0].y + m_Bounds[1].y) / 2;
		buffer->push_back({ glm::vec4(Bounds[0].x, y, Bounds[0].z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(1, 0) });
		buffer->push_back({ glm::vec4(Bounds[0].x, y, Bounds[1].z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(1, 1) });
		buffer->push_back({ glm::vec4(Bounds[1].x, y, Bounds[1].z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 1) });

		buffer->push_back({ glm::vec4(Bounds[1].x, y, Bounds[1].z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 1) });
		buffer->push_back({ glm::vec4(Bounds[1].x, y, Bounds[0].z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
		buffer->push_back({ glm::vec4(Bounds[0].x, y, Bounds[0].z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(1, 0) });

		newvbo->SetBuffer(buffer, &(*buffer)[0], buffer->size());

		//newvbo->InitStructure();

		vbo.insert_or_assign(vpId, newvbo);

		C3DObjectProgram *newprog = new C3DObjectProgram("Minimap.v.glsl", "Minimap.f.glsl", "vertex", "texcoor", NULL, NULL);
		prog.insert_or_assign(vpId, newprog);

		int minimapTexSize = CSettings::GetInt(IntMinimapTextureSize);

		FIBITMAP *mmimage = FreeImage_Rescale(subimage, minimapTexSize, minimapTexSize, FILTER_BSPLINE);
		C3DObjectTexture *newtex = new C3DObjectTexture(mmimage, "tex", false, false);
		tex.insert_or_assign(vpId, newtex);

		scaleMatrix.insert_or_assign(vpId, glm::mat4(1.0f));
		rotateMatrix.insert_or_assign(vpId, glm::mat4(1.0f));
		translateMatrix.insert_or_assign(vpId, glm::mat4(1.0f));
	}
}
