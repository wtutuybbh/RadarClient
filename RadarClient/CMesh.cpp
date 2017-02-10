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

bool CMesh::LoadHeightmap()
{
	string context = "CMesh::LoadHeightmap(int vpId)";

	if (LOG_ENABLED && CMesh_LoadHeightmap_LOG)
	{
		LOG_INFO(requestID, context, "Start");
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

	

	short *data = (short *)alt_.Data();

	int nX, nZ, nTri, nIndex = 0;									// Create Variables
	float flX, flZ;
	short maxheight = 0, minheight = 10000;
	for (int i = 0; i < alt_.Width() * alt_.Height(); i++) {
		
		if (data[i] > maxheight) maxheight = data[i];
		if (data[i] < minheight) minheight = data[i];
	}
	bounds = new glm::vec3[2];
	bounds[0].x = bounds[0].y = bounds[0].z = FLT_MAX;
	bounds[1].x = bounds[1].y = bounds[1].z = FLT_MIN;

	double dlon = alt_.DLon();
	double dlat = alt_.DLat();

	double latSW = alt_.Lat0(); 

	float lonStretch = dlon * cnvrt::londg2m(1, latSW + dlat * (alt_.Width() - 1) / 2.0) / MPPh;
	float latStretch = dlat * cnvrt::latdg2m(1, latSW + dlat * (alt_.Width() - 1) / 2.0) / MPPh;


	VBOData tmp;
	float minh = CSettings::GetFloat(FloatMinAltitude), maxh = CSettings::GetFloat(FloatMaxAltitude), h, level;
	glm::vec4 mincolor = CSettings::GetColor(ColorAltitudeLowest), maxcolor = CSettings::GetColor(ColorAltitudeHighest);

	int H = alt_.Height();
	int W = alt_.Width();

	int N = ((W - 3) * 2 + 6 + 1)*(H - 1) - 1;
	index_length = N;

	//std::vector<VBOData> * buffer = new std::vector<VBOData>((alt_.Width() - 1) * (alt_.Height() - 1) * 6);
	buffer = new std::vector<VBOData>(H*W);

	int sign = -1, loop_length = (W - 2) * 2, step_length = 1, next_step = 0, change_mode = 1, mode_id = 0, special_mode_id = 4, next_big_length = loop_length, x = 0, x_prev = 0, dXtone = 1, X = 0;
	int dYCounter = 0, dYtone = 0, next_step_Ybase_change = 0, next_step_Ybase_change_prev = 0, Ybase = 0, Y = 0;
	int x_before_change;
	averageHeight = 0;
	for (auto i=0; i<H*W; i++)
	{
		Y = (int) i / W;
		X = i % W;

		h = alt_.ValueAt(X, Y);
		averageHeight += h;

		level = (h - minh) / (maxh - minh);

		(*buffer)[i] = {
			glm::vec4(
				lonStretch * (-X + alt_.Width() / 2.0),
				h / MPPv,
				latStretch * (Y - alt_.Height() / 2.0),
				1),
			glm::vec3(0, 1, 0),
			mincolor * (1 - level) + maxcolor * level,
			glm::vec2(((float)X) / W, ((float)Y) / H) };

		rcutils::takeminmax((*buffer)[i].vert.x, &(bounds[0].x), &(bounds[1].x));
		rcutils::takeminmax((*buffer)[i].vert.y, &(bounds[0].y), &(bounds[1].y));
		rcutils::takeminmax((*buffer)[i].vert.z, &(bounds[0].z), &(bounds[1].z));
	}
	averageHeight /= H * W;
	idxArray = new unsigned short[N];
	// SEE GL_LINE_STRIP.xlsx for details
	for (int i = 0; i<N; i++)
	{
		x_before_change = x;
		change_mode = (int)(next_step == i);
		next_big_length = (step_length == loop_length && next_big_length == loop_length ? 3 : (step_length == 3 && next_big_length == 3 ? loop_length : next_big_length));
		special_mode_id = (mode_id == 4 && special_mode_id == 4 ? 5 : (mode_id == 5 && special_mode_id == 5 ? 4 : special_mode_id));
		mode_id = (i == 0 ? 0 : (i == N - 1 ? 6 : (change_mode == 1 ? (mode_id <= 2 ? mode_id + 1 : (mode_id == 3 ? special_mode_id : 1)) : mode_id))); // еякх(C60 = $AO$55 - 1; 6; еякх(T60 = 1; еякх(V59 <= 2; V59 + 1; еякх(V59 = 3; W60; 1)); V59)))
		step_length = (i < 2 || i == N - 1 ? 1 : (change_mode == 1 ? (step_length>1 ? 1 : next_big_length) : step_length));
		next_step = next_step + step_length * change_mode;
		sign = mode_id == 1 ? -1 * sign : sign;
		x = (mode_id == 1 ? x : (mode_id == 2 ? (x == x_prev ? x + sign : x) : (mode_id == 3 ? x + sign : x))); //=еякх(V60=1;Y59;еякх(V60=2;еякх(Y59=Y58;Y59+P60;Y59);еякх(V60=3;Y59+P60;Y59)))
		x_prev = x_before_change;
		X = (mode_id == 4 ? x + dXtone - 1 : (mode_id == 5 ? x + dXtone : x));
		next_step_Ybase_change_prev = next_step_Ybase_change;
		next_step_Ybase_change = mode_id == special_mode_id ? 1 : 0;
		Ybase += next_step_Ybase_change_prev;
		dYCounter = next_step_Ybase_change_prev == 0 && (dYCounter == 0 || dYCounter == 3) ? 0 : dYCounter + 1;
		dYtone = dYCounter == 3 ? dYtone ^ 1 : dYtone;
		Y = Ybase + (next_step_Ybase_change_prev == 0 ? !(dXtone^dYtone) : 0);

		//outfile << i << ";" << change_mode << ";" << next_big_length << ";" << special_mode_id << ";" << mode_id << ";" << step_length << ";" << next_step << ";" << sign << ";" << x << ";" << dXtone << ";" << X << std::endl;
		//outfile << i << ";" << dYCounter << ";" << dYtone << ";" << next_step_Ybase_change << ";" << Ybase << ";" << Y << std::endl;
		
		idxArray[i] = Y * W + X;

		dXtone ^= 1;
	}

	
	
	

		

	centerHeight = alt_.ValueAt(min(max(alt_.Width() / 2 - 1, 0), alt_.Width() - 1), min(max(alt_.Height() / 2, 0), alt_.Height() - 1));

	prog.insert_or_assign(Main, new C3DObjectProgram("CMesh.vert", "CMesh.frag", "vertex", "texcoor", nullptr, "color"));

	auto vbo_ = new C3DObjectVBO(clearAfter);
	vbo_->SetBuffer(buffer, &(*buffer)[0], buffer->size());
	vbo_->AddIndexArray(idxArray, N, GL_TRIANGLE_STRIP);

	FIBITMAP* subimage = (FIBITMAP*)maptexture->Data();

	if (FreeImage_GetBPP(subimage) != 32)
	{
		FIBITMAP* tempImage = subimage;
		subimage = FreeImage_ConvertTo32Bits(tempImage);
	}
	tex.insert_or_assign(Main, new C3DObjectTexture(subimage, "tex", false, false));

	vbo.insert_or_assign(Main, vbo_);

	InitMiniMap();

	return true;
}

CMesh::CMesh(bool clearAfter, glm::vec2 position, double max_range, int texsize, int resolution, float MPPh, float MPPv) : C3DObjectModel()
{
	std::string context = "CMesh::CMesh";
	LOG_INFO(requestID, context, "Start... clearAfter=%d, position=(%f, %f), max_range=%f, texsize=%d, resolution=%d", clearAfter , position.x, position.y, max_range, texsize, resolution);

	bounds = nullptr;
	this->position = position;
	this->texsize = texsize;
	this->resolution = resolution;
	this->max_range = max_range;
	this->clearAfter = clearAfter;
	this->MPPh = MPPh;
	this->MPPv = MPPv;
	
	UseTexture = 1;

	boost::thread t(boost::bind(&CMesh::LoadHeightmap, this));
	t.detach();
}

CMesh::~CMesh()
{
	for (auto it = begin(vbo); it != end(vbo); ++it)
	{
		auto *buffer = (vector<VBOData>*)it->second->GetBuffer();
		delete buffer;
	}
	if (bounds)
		delete[] bounds;
}
glm::vec3 CMesh::GetSize() {
	auto b = GetBounds();
	if (b) return b[1] - b[0];
	return glm::vec3(0.0f, 0.0f, 0.0f);
}

bool CMesh::IntersectLine(int vpId, glm::vec3& orig_, glm::vec3& dir_, glm::vec3& position_)
{
	return false;

	if (!Ready()) {
		return false;
	}

	string context = "CMesh::IntersectLine";
	LOG_INFO(requestID, context, (boost::format("Start... vpId=%1%, orig_=(%2%, %3%, %4%), dir_=(%5%, %6%, %7%")
		% vpId
		% orig_.x
		% orig_.y
		% orig_.z
		% dir_.x
		% dir_.y
		% dir_.z).str().c_str());

	glm::vec4 planeOrig(0, averageHeight, 0, 1), planeNormal(0, 1, 0, 0);
	float distance;
	glm::vec4 orig(orig_, 1);
	glm::vec4 dir(dir_, 0);
	glm::vec4 position;

	bool planeResult = glm::intersectRayPlane(orig, dir, planeOrig, planeNormal, distance);
	glm::vec4 approxPoint = orig + distance * dir;
	CMesh *m;
	glm::vec3 *b = GetBounds(); 
	if (b) {		
		if (approxPoint.x > b[0].x && approxPoint.z > b[0].z && approxPoint.x <= b[1].x && approxPoint.z <= b[1].z) {			
			//break;
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
	if (!Ready()) {
		return;
	}
	glm::mat4 m = GetModelMatrix(vpControl);
	glm::mat4 v = vpControl->GetViewMatrix();
	glm::mat4 p = vpControl->GetProjMatrix();
	glm::mat4 mvp = p*v*m;
	int mvp_loc = prog.at(vpControl->Id)->GetUniformLocation("mvp");

	glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, glm::value_ptr(mvp));

	if (vpControl->Id == Main) {		
		int useTexture_loc = prog.at(vpControl->Id)->GetUniformLocation("useTexture");
		int y0_loc = prog.at(vpControl->Id)->GetUniformLocation("y_0");
		int usey0_loc = prog.at(vpControl->Id)->GetUniformLocation("useY0");

		glUniform1i(useTexture_loc, UseTexture);
		glUniform1i(usey0_loc, UseY0Loc);
		glUniform1f(y0_loc, (int)(centerHeight / MPPv));		
	}
}

glm::vec3 * CMesh::GetBounds()
{
	if (Ready())
		return bounds;
	return nullptr;
}

void CMesh::InitMiniMap()
{
	if (bounds && maptexture)
	{
		C3DObjectVBO *newvbo = new C3DObjectVBO(false);
		
		std::vector<VBOData> *buffer = new std::vector<VBOData>;
		float y = 0;
		buffer->push_back({ glm::vec4(bounds[0].x, y, bounds[0].z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(1, 0) });
		buffer->push_back({ glm::vec4(bounds[0].x, y, bounds[1].z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(1, 1) });
		buffer->push_back({ glm::vec4(bounds[1].x, y, bounds[1].z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 1) });

		buffer->push_back({ glm::vec4(bounds[1].x, y, bounds[1].z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 1) });
		buffer->push_back({ glm::vec4(bounds[1].x, y, bounds[0].z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
		buffer->push_back({ glm::vec4(bounds[0].x, y, bounds[0].z, 1), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1), glm::vec2(1, 0) });

		newvbo->SetBuffer(buffer, &(*buffer)[0], buffer->size());

		

		C3DObjectProgram *newprog = new C3DObjectProgram("Minimap.v.glsl", "Minimap.f.glsl", "vertex", "texcoor", nullptr, nullptr);
		prog.insert_or_assign(MiniMap, newprog);

		int minimapTexSize = CSettings::GetInt(IntMinimapTextureSize);

		FIBITMAP *mmimage = FreeImage_Rescale((FIBITMAP *)maptexture->Data(), minimapTexSize, minimapTexSize, FILTER_BSPLINE);
		if (FreeImage_GetBPP(mmimage) != 32)
		{
			FIBITMAP* tempImage = mmimage;
			mmimage = FreeImage_ConvertTo32Bits(tempImage);
		}
		//FreeImage_Save(FIF_JPEG, mmimage, "minimap.jpg", 0);
		C3DObjectTexture *newtex = new C3DObjectTexture(mmimage, "tex", false, false); //new C3DObjectTexture("video.png", "tex");// 

		tex.insert_or_assign(MiniMap, newtex);

		scaleMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
		rotateMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
		translateMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));

		delete maptexture;
		maptexture = nullptr;

		vbo.insert_or_assign(MiniMap, newvbo);
	}
}
bool CMesh::Ready() {
	try {
		if (vbo.at(Main) && vbo.at(MiniMap)) return true;
	}
	catch (...) {

	}
	return false;
}
float CMesh::GetCenterHeight() {
	return centerHeight;
}

float CMesh::GetAverageHeight()
{
	return averageHeight;
}
