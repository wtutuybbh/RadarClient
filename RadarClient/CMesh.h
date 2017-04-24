#pragma once
#include "stdafx.h"

#include "C3DObjectModel.h"
#include "CRCAltitudeDataFile.h"


class CRCTextureDataFile;
class CScene;
class CRCDataFileSet;

typedef struct {
	GLint sizeX, sizeY;
	short *data{ nullptr };
} AltitudeMap;

typedef struct {
	char *fileName{ nullptr };
	int sizeX, sizeY, iLonStart, iLatStart, iLonEnd, iLatEnd, Nlon, Nlat;
	double lon0, lat0, dlon, dlat, lonSW, latSW;
} AltitudeMapHeader;

typedef struct {
	int sizeX, sizeY;
	double imgLon0, imgLat0, imgLon1, imgLat1;
} ImageMapHeader;

typedef int(_cdecl * GDPALTITUDEMAP)(const char *, double *, int *, short *);
typedef int(_cdecl * GDPALTITUDEMAP_SIZES)(const char *, double *, int *);

class CMesh : public C3DObjectModel
{	
	CRCDataFileSet *altitudes_set{ nullptr };
	CRCDataFileSet *textures_set {nullptr};
	void LoadHeightmap(bool reload_textures, bool rescan_folder_for_textures, bool reload_altitudes, bool rescan_folder_for_altitudes, bool recalculate_blindzones);

	int texsize;

	glm::vec3 * bounds{ nullptr };	

	CRCTextureDataFile *maptexture { nullptr };
	CRCAltitudeDataFile *altitude{ nullptr };

	bool clearAfter;

	glm::vec2 position;
	int resolution;
	double max_range;
	double h0 {1.5};
	double e {0};
	//float MPPh, MPPv;
	float maxh{ FLT_MIN }, minh{ FLT_MAX };
	float maxbz{ FLT_MIN }, minbz{ FLT_MAX };

	unsigned short *idxArray{ nullptr }, *idxArray2{ nullptr };

	bool buffer_ready{ false };
	int index_length{ 0 };
	double centerHeight{ 0 }, averageHeight{ 0 };

	bool ready{ false };

	std::thread * heightMapLoader{ nullptr };
public:
	//glm::vec3 Size;
	~CMesh();
	int UseTexture, UseY0Loc;
	CMesh(bool clearAfter, glm::vec2 position, double max_range, int texsize, int resolution, float MPPh, float MPPv);
	void Refresh();
	float GetCenterHeight();
	float GetAverageHeight();
	glm::vec3 GetSize();
	bool IntersectLine(int vpId, glm::vec3 & orig, glm::vec3 & dir, glm::vec3 & position) override;
	void BindUniforms(CViewPortControl *vpControl) override;
	glm::vec3 * CMesh::GetBounds() override;
	void InitMiniMap();
	bool Ready();
	int GetResolution();
	CRCAltitudeDataFile * GetAltitudeDataFile();
	float GetHeightAtLL(float lon, float lat);
	glm::vec2 XY2LL(glm::vec2 xy);
};