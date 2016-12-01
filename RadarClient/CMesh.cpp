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
#include "CRCAltitudeDataFile.h"
#include "CRCDataFileSet.h"
#include "CRCTextureDataFile.h"

#define LOG_ENABLED true
#define CMesh_LoadHeightmap_LOG true

float CMesh::Y0;

bool CMesh::LoadHeightmap(int vpId)
{
	string context = "CMesh::LoadHeightmap(int vpId)";

	if (LOG_ENABLED && CMesh_LoadHeightmap_LOG)
	{
		CRCLogger::Info(requestID, context, "Start, vpId=" + vpId);
	}


	double lonm = cnvrt::londg2m(1, position.y);
	double latm = cnvrt::latdg2m(1, position.y);
	if (lonm <= 0 || latm <= 0)
	{
		LOG_ERROR__("lonm=%f latm=%f", lonm, latm);
	}
	double max_range_lon = max_range / lonm;
	double max_range_lat = max_range / latm;
	if (LOG_ENABLED && CMesh_LoadHeightmap_LOG)
	{
		LOG_INFO__("lonm=%f latm=%f max_range_lon=%f max_range_lat=%f",
			lonm,
			latm,
			max_range_lon,
			max_range_lat);
	}
	

	double lon0 = position.x - max_range_lon;
	double lat0 = position.y - max_range_lat;
	double lon1 = position.x + max_range_lon;
	double lat1 = position.y + max_range_lat;

	//CRCTextureDataFile iMapH(lon0, lat0, lon1, lat1, texsize, texsize);

	//here we get pixels from our "database"



	//subimage = FreeImage_Copy((FIBITMAP*)bitmap, left, top, right, bottom);

	double px = (lon1 - lon0) / texsize;
	double py = (lat1 - lat0) / texsize;

	CRCDataFileSet set;

	set.AddFiles("AltitudeData", Altitude, "");	

	int width = 200;
	int height = 200;


	CRCAltitudeDataFile alt_(lon0, lat0, lon1, lat1, width, height);

	for (auto i = 0; i < set.Files().size(); i++)
	{
		alt_.ApplyIntersection(set.GetFile(i));
	}

	std::vector<VBOData> * buffer = new std::vector<VBOData>((alt_.Width() - 1) * (alt_.Height() - 1) * 6);

	short *data = (short *)alt_.Data();

	int nX, nZ, nTri, nIndex = 0;									// Create Variables
	float flX, flZ;
	short maxheight = 0, minheight = 10000;
	for (int i = 0; i < alt_.Width() * alt_.Height(); i++) {
		
		if (data[i] > maxheight) maxheight = data[i];
		if (data[i] < minheight) minheight = data[i];
	}
	Bounds = new glm::vec3[2];
	Bounds[0].x = Bounds[0].y = Bounds[0].z = FLT_MAX;
	Bounds[1].x = Bounds[1].y = Bounds[1].z = FLT_MIN;

	double dlon = alt_.DLon();
	double dlat = alt_.DLat();

	double latSW = alt_.Lat0(); 

	float lonStretch = dlon * cnvrt::londg2m(1, latSW + dlat * (alt_.Width() - 1) / 2.0) / MPPh;
	float latStretch = dlat * cnvrt::latdg2m(1, latSW + dlat * (alt_.Width() - 1) / 2.0) / MPPh;

	LocalAverageHeight = 0;

	VBOData tmp;
	float minh = CSettings::GetFloat(FloatMinAltitude), maxh = CSettings::GetFloat(FloatMaxAltitude), h, level;
	glm::vec4 mincolor = CSettings::GetColor(ColorAltitudeLowest), maxcolor = CSettings::GetColor(ColorAltitudeHighest);
	for (nZ = 0; nZ < alt_.Height() - 1; nZ++)
	{
		for (nX = 0; nX < alt_.Width() - 1; nX++)
		{
			for (nTri = 0; nTri < 6; nTri++)
			{
				// Using This Quick Hack, Figure The X,Z Position Of The Point
				flX = (float)nX + ((nTri == 1 || nTri == 2 || nTri == 5) ? 1.0f : 0.0f);
				flZ = (float)nZ + ((nTri == 2 || nTri == 4 || nTri == 5) ? 1.0f : 0.0f);

				// Set The Data, Using PtHeight To Obtain The Y Value
				h = alt_.ValueAt((int)flX, (int)flZ);
				level = (h - minh) / (maxh - minh);
				tmp = {
					glm::vec4(
						lonStretch * (-flX + alt_.Width() / 2.0),
						h / MPPv,
						latStretch * (flZ - alt_.Height() / 2.0),
						1),
					glm::vec3(0, 1, 0),
					mincolor * (1 - level) + maxcolor * level,
					glm::vec2(flX / alt_.Width(), flZ / alt_.Height()) };

				buffer->push_back(tmp);

				rcutils::takeminmax(tmp.vert.x, &(Bounds[0].x), &(Bounds[1].x));
				rcutils::takeminmax(tmp.vert.y, &(Bounds[0].y), &(Bounds[1].y));
				rcutils::takeminmax(tmp.vert.z, &(Bounds[0].z), &(Bounds[1].z));

				if (nTri == 0)
					LocalAverageHeight += tmp.vert.y;
				// Increment Our Index
				nIndex++;


			}
		}
	}
	LocalAverageHeight /= alt_.Height() * alt_.Width();

	prog.insert_or_assign(vpId, new C3DObjectProgram("CMesh.vert", "CMesh.frag", "vertex", "texcoor", nullptr, "color"));

	vbo.insert_or_assign(vpId, new C3DObjectVBO(clearAfter));
	vbo.at(vpId)->SetBuffer(buffer, &(*buffer)[0], buffer->size());

	if (FreeImage_GetBPP(subimage) != 32)
	{
		FIBITMAP* tempImage = subimage;
		subimage = FreeImage_ConvertTo32Bits(tempImage);
	}
	tex.insert_or_assign(vpId, new C3DObjectTexture(subimage, "tex", true, false));



	CenterHeight = alt_.ValueAt(min(max(alt_.Width() / 2 - 1, 0), alt_.Width() - 1), min(max(alt_.Height() / 2, 0), alt_.Height() - 1));
	Y0 = CenterHeight / MPPv;


	return true;
}

AltitudeMap* CMesh::GetAltitudeMap(const char* fileName, double lon1, double lat1, double lon2, double lat2)
{
	string context = "CMesh::GetAltitudeMap";
	CRCLogger::Info(requestID, context, (boost::format("Start... fileName=%1%, lon1=%2%, lat1=%3%, lon2=%4%, lat2=%5%")
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
	if (hDLL != nullptr)
	{
		gdpAltitudeMap = (GDPALTITUDEMAP)GetProcAddress(hDLL, "gdpAltitudeMap");
		gdpAltitudeMap_Sizes = (GDPALTITUDEMAP_SIZES)GetProcAddress(hDLL, "gdpAltitudeMap_Sizes");
		if (!gdpAltitudeMap)
		{
			FreeLibrary(hDLL);
			return nullptr;
		}
		LL[0] = lon1;
		LL[1] = lat1;
		LL[2] = lon2;
		LL[3] = lat2;
		result = gdpAltitudeMap_Sizes(fileName, LL, size);
		if (result == 0) {
			if (!aMapH) {
				aMapH = new AltitudeMapHeader;
				aMapH->fileName = nullptr;
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

			CRCLogger::Info(requestID, context, (boost::format("aMapH overwrite? sizeX=%1%, sizeY=%2%, iLonStart=%3%, iLatStart=%4%, iLonEnd=%5%, iLatEnd=%6%, Nlon=%7%, Nlat=%8%, lon0=%9%, lat0=%10%, dlon=%11%, dlat=%12%, lonSW=%13%, latSW=%14%")
				% aMapH->sizeX % aMapH->sizeY % aMapH->iLonStart % aMapH->iLatStart % aMapH->iLonEnd % aMapH->iLatEnd % aMapH->Nlon % aMapH->Nlat % aMapH->lon0	% aMapH->lat0 % aMapH->dlon	% aMapH->dlat % aMapH->lonSW % aMapH->latSW).str());

			short *data;
			data = new short[size[0] * size[1]];
			
			if (data) {
				CRCLogger::Info(requestID, context, (boost::format("size of data is %1% (%2% * %3%)")
					% (size[0] * size[1]) % size[0] % size[1]).str());
				result = gdpAltitudeMap(fileName, LL, size, data);
				if (result == 0) {
					aMap = new AltitudeMap;
					aMap->sizeX = size[0];
					aMap->sizeY = size[1];
					aMap->data = data;
					FreeLibrary(hDLL);
					CRCLogger::Info(requestID, context, "Return, success.");
					return aMap;
				}
			}
			CRCLogger::Error(requestID, context, (boost::format("new short[size[0] * size[1]] failed. (size[0] * size[1] = %1%)")
				% (size[0] * size[1])).str());
		}
		FreeLibrary(hDLL);
	}
	CRCLogger::Error(requestID, context, "LoadLibrary failed.");
	return nullptr;
}

AltitudeMapHeader* CMesh::GetAltitudeMapHeader(const char* fileName, double lon1, double lat1, double lon2, double lat2)
{
	string context = "CMesh::GetAltitudeMapHeader";
	CRCLogger::Info(requestID, context, (boost::format("Start... fileName=%1%, lon1=%2%, lat1=%3%, lon2=%4%, lat2=%5%")
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
	if (hDLL != nullptr)
	{
		gdpAltitudeMap_Sizes = (GDPALTITUDEMAP_SIZES)GetProcAddress(hDLL, "gdpAltitudeMap_Sizes");
		if (!gdpAltitudeMap_Sizes)
		{
			FreeLibrary(hDLL);
			return nullptr;
		}
		LL[0] = lon1;
		LL[1] = lat1;
		LL[2] = lon2;
		LL[3] = lat2;
		result = gdpAltitudeMap_Sizes(fileName, LL, size);
		if (result == 0) {
			if (!aMapH) {
				aMapH = new AltitudeMapHeader;
				aMapH->fileName = nullptr;
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

			CRCLogger::Info(requestID, context, (boost::format("Return: sizeX=%1%, sizeY=%2%, iLonStart=%3%, iLatStart=%4%, iLonEnd=%5%, iLatEnd=%6%, Nlon=%7%, Nlat=%8%, lon0=%9%, lat0=%10%, dlon=%11%, dlat=%12%, lonSW=%13%, latSW=%14%")
				% aMapH->sizeX % aMapH->sizeY % aMapH->iLonStart % aMapH->iLatStart % aMapH->iLonEnd % aMapH->iLatEnd % aMapH->Nlon % aMapH->Nlat % aMapH->lon0 % aMapH->lat0 % aMapH->dlon % aMapH->dlat % aMapH->lonSW % aMapH->latSW).str());

			return aMapH;
		}
		CRCLogger::Error(requestID, context, "gdpAltitudeMap_Sizes returned 0.");
		FreeLibrary(hDLL);
	}
	CRCLogger::Error(requestID, context, "LoadLibrary failed.");
	return nullptr;
}

ImageMapHeader* CMesh::GetImageMapHeader(const char* imgFile, const char* datFile)
{
	try 
	{
		bitmap = FreeImage_Load(FreeImage_GetFileType(imgFile, 0), imgFile);
	}
	catch (...) 
	{
		return nullptr;
	}
	//access to data: ((FIBITMAP *)bitmap)->data

	std::ifstream infile;

	try 
	{
		infile.open(datFile, std::ifstream::in | std::ifstream::binary);
	}
	catch (...) 
	{
		return nullptr;
	}

	if (!infile) 
	{
		return nullptr;
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
			pch = strtok(nullptr, ",");
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

CMesh::CMesh(int vpId, bool clearAfter, glm::vec2 position, double max_range, int texsize, int resolution) : C3DObjectModel(vpId, nullptr, nullptr, nullptr)
{
	std::string context = "CMesh::CMesh";
	CRCLogger::Info(requestID, context, (boost::format("Start... vpId=%1%, clearAfter=%3%, position=(%4%, %5%), max_range=%6%, texsize=%7%, resolution=%8%") 
		% vpId 
		% 0 //scn 
		% clearAfter 
		% position.x
		% position.y
		% max_range
		% texsize
		% resolution).str());	

	aMap = nullptr;
	Bounds = nullptr;
	iMapH = nullptr;
	aMapH = nullptr;
	this->texsize = texsize;
	this->resolution = resolution;
	this->max_range = max_range;
	this->clearAfter = clearAfter;
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
	CRCLogger::Info(requestID, context, (boost::format("Start... vpId=%1%, orig_=(%2%, %3%, %4%), dir_=(%5%, %6%, %7%")
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

		C3DObjectProgram *newprog = new C3DObjectProgram("Minimap.v.glsl", "Minimap.f.glsl", "vertex", "texcoor", nullptr, nullptr);
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
