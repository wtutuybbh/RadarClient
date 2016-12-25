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
		LOG_INFO(requestID, context, "Start, vpId=" + vpId);
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
	CRCDataFileSet set;
	set.AddFiles("TextureData", Texture, "");

	maptexture = new CRCTextureDataFile(lon0, lat0, lon1, lat1, texsize, texsize);

	for (auto i = 0; i < set.Files().size(); i++)
	{
		maptexture->ApplyIntersection(set.GetFile(i));
	}
	
	//subimage = FreeImage_Copy((FIBITMAP*)bitmap, left, top, right, bottom);

	double px = (lon1 - lon0) / texsize;
	double py = (lat1 - lat0) / texsize;

	set.Clear();

	set.AddFiles("AltitudeData", Altitude, "");	


	CRCAltitudeDataFile alt_(lon0, lat0, lon1, lat1, resolution, resolution);

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

	FIBITMAP* subimage = (FIBITMAP*)maptexture->Data();

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

CMesh::CMesh(int vpId, bool clearAfter, glm::vec2 position, double max_range, int texsize, int resolution, float MPPh, float MPPv) : C3DObjectModel(vpId, nullptr, nullptr, nullptr)
{
	std::string context = "CMesh::CMesh";
	LOG_INFO(requestID, context, (boost::format("Start... vpId=%1%, clearAfter=%3%, position=(%4%, %5%), max_range=%6%, texsize=%7%, resolution=%8%") 
		% vpId 
		% 0 //scn 
		% clearAfter 
		% position.x
		% position.y
		% max_range
		% texsize
		% resolution).str());	

	Bounds = nullptr;
	this->position = position;
	this->texsize = texsize;
	this->resolution = resolution;
	this->max_range = max_range;
	this->clearAfter = clearAfter;
	this->MPPh = MPPh;
	this->MPPv = MPPv;
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
}

bool CMesh::IntersectLine(int vpId, glm::vec3& orig_, glm::vec3& dir_, glm::vec3& position_)
{
	string context = "CMesh::IntersectLine";
	LOG_INFO(requestID, context, (boost::format("Start... vpId=%1%, orig_=(%2%, %3%, %4%), dir_=(%5%, %6%, %7%")
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
		b = m->Bounds; // now b is a shortcut for m->Bounds
		//find appropriate part of surface by testing bounds:
		if (approxPoint.x > b[0].x && approxPoint.z > b[0].z && approxPoint.x <= b[1].x && approxPoint.z <= b[1].z) {			
			break;
		}
	}
	
	//now work with pointer m:
	//grid coordinates:
	int ix0 = -resolution * (approxPoint.x - b[1].x) / (b[1].x - b[0].x);
	int iy0 = resolution * (approxPoint.z - b[0].z) / (b[1].z - b[0].z);
	if (ix0 < 0 || ix0 >= resolution || iy0 < 0 || iy0 >= resolution) {
		ix0 = resolution / 2;
		iy0 = resolution / 2;
	}
	// 2. test triangles around approximate intersection point

	vector<VBOData> *buffer = (vector<VBOData> *)m->GetC3DObjectVBO(Main)->GetBuffer();

	int X = resolution - 1, Y = resolution - 1;
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
		glUniform1i(usey0_loc, UseY0Loc);
		glUniform1f(y0_loc, Y0);
		
	}
}

glm::vec3 * CMesh::GetBounds()
{
	return Bounds;
}

void CMesh::Init(int vpId)
{
	if (Bounds && maptexture && vpId == MiniMap)
	{
		C3DObjectVBO *newvbo = new C3DObjectVBO(false);
		
		std::vector<VBOData> *buffer = new std::vector<VBOData>;
		float y = 0;
		buffer->push_back({ glm::vec4(Bounds[0].x, y, Bounds[0].z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(1, 0) });
		buffer->push_back({ glm::vec4(Bounds[0].x, y, Bounds[1].z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(1, 1) });
		buffer->push_back({ glm::vec4(Bounds[1].x, y, Bounds[1].z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 1) });

		buffer->push_back({ glm::vec4(Bounds[1].x, y, Bounds[1].z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 1) });
		buffer->push_back({ glm::vec4(Bounds[1].x, y, Bounds[0].z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
		buffer->push_back({ glm::vec4(Bounds[0].x, y, Bounds[0].z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(1, 0) });

		newvbo->SetBuffer(buffer, &(*buffer)[0], buffer->size());

		vbo.insert_or_assign(vpId, newvbo);

		C3DObjectProgram *newprog = new C3DObjectProgram("Minimap.v.glsl", "Minimap.f.glsl", "vertex", "texcoor", nullptr, nullptr);
		prog.insert_or_assign(vpId, newprog);

		int minimapTexSize = CSettings::GetInt(IntMinimapTextureSize);

		FIBITMAP *mmimage = FreeImage_Rescale((FIBITMAP *)maptexture->Data(), minimapTexSize, minimapTexSize, FILTER_BSPLINE);
		if (FreeImage_GetBPP(mmimage) != 32)
		{
			FIBITMAP* tempImage = mmimage;
			mmimage = FreeImage_ConvertTo32Bits(tempImage);
		}
		//FreeImage_Save(FIF_JPEG, mmimage, "minimap.jpg", 0);
		C3DObjectTexture *newtex = new C3DObjectTexture(mmimage, "tex", false, false); //new C3DObjectTexture("video.png", "tex");// 

		tex.insert_or_assign(vpId, newtex);

		scaleMatrix.insert_or_assign(vpId, glm::mat4(1.0f));
		rotateMatrix.insert_or_assign(vpId, glm::mat4(1.0f));
		translateMatrix.insert_or_assign(vpId, glm::mat4(1.0f));
	}
}
