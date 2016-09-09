#pragma once
#include "CRCSocket.h"
#include "glm/glm.hpp"
#include <string>
#include <vector>
#include "glm/detail/type_mat.hpp"

class CTrack;
class CLine;

#define ZEROLEVEL_ZERO 0
#define ZEROLEVEL_MAXHEIGHT 1
#define ZEROLEVEL_ACTUALHEIGHT 2
#define RAY_HEIGHT 0.174533f //10 degrees


class CSector;
class CMarkup;
class C3DObjectModel;
class old_CMesh;
class CVert;													// Vertex Class
typedef CVert CVec;												// The Definitions Are Synonymous
class CTexCoord;												// Texture Coordinate Class
class CColorRGBA;
class old_CRCPoint;
class CRCPointModel;
class CRCTrackModel;
class CCamera;
class old_CTrack;
class CRImageSet;


#include "CUserInterface.h"

class old_C3DObject;
class old_CMinimapPointer;
class CMiniMapPointer;
class CMesh;

class CScene {
	unsigned long mainDrawCount{ 0 };
public:
	CLine *begAzmLine{ NULL };

	float maxAmp = 0;

	float mpph; // meters per pixel vertical
	float mppv; // meters per pixel horizontal
	glm::vec2 geocenter; //geographic coordinates of center point (place of the radar)
	int msize; // area square's side length in meters
	glm::vec3 *meshBounds{ NULL };

	std::string altFile, imgFile, datFile;
	
	int texsize;
	
	int width, height; //window width and height
	float RotatingSpeed; //degrees per second

	// the next variables should be placed in each's class:

	float rayWidth, minE, maxE, minDist, maxDist, minA, maxA;
	int rayDensity, rayArraySize;

	float AxisGridShift = 50.0f;
	float YAxisLength = 1000.0f;
	float markSize = 10.0f;
	int markDistance = 100;


	int vertexCount_Ray;
	int vertexCount_Axis;
	int vertexCount_Info, infoWidth, infoHeight;

	int numCircles, marksPerCircle, segmentsPerCircle;

	glm::vec3 *AxisGrid{ NULL }, *Ray{ NULL };
	CColorRGBA *AxisGridColor{ NULL }, *RayColor{ NULL };



	unsigned int AxisGrid_VBOName, AxisGrid_VBOName_c;
	unsigned int Ray_VBOName, Ray_VBOName_c;

	unsigned short vertexCount, markCount;

	float viewAngle;

	///arrays for different VBO objects:
	unsigned short *markup{ NULL };
	unsigned short **circles{ NULL };
	unsigned short *ray{ NULL };
	unsigned short *info{ NULL };

	int zeroLevel = ZEROLEVEL_ACTUALHEIGHT;
	float y0;

	std::vector<CSector*> Sectors; //points

	std::unordered_map<int, CTrack*> Tracks;
	vector<int> SelectedTracksIds;

	int SectorsCount;

	CCamera *Camera{ NULL };

	CRCSocket *Socket{ NULL };
	CUserInterface *UI{ NULL };

	bool VBOisBuilt, RayVBOisBuilt, MiniMapVBOisBuilt;


	RDR_INITCL * rdrinit{NULL};
	bool Initialized;

	std::vector<C3DObjectModel*> Selection;
	void PushSelection(C3DObjectModel *o);
	void ClearSelection();

	CMesh* Mesh{ NULL };
	CMesh* Mesh1{ NULL };
	CMesh* Mesh2{ NULL };
	CMesh* Mesh3{ NULL };

	CMiniMapPointer *mmPointer {NULL};

	CRImageSet *ImageSet {NULL};

	CMarkup *Markup {NULL};

	glm::vec3 MeshSize;
	glm::vec3 * m_Bounds{ NULL };
	CScene(std::string alfFile, std::string imgFile, std::string datFile, float lonc, float latc, float mpph, float mppv, int texsize);
	~CScene();

	bool DrawScene(CViewPortControl * vpControl);
	bool MiniMapDraw(CViewPortControl * vpControl);
	
	bool PrepareVBOs();
	bool PrepareRayVBO();

	bool MiniMapPrepareAndBuildVBO();

	bool BuildVBOs();
	bool BuildRayVBO();

	void RefreshSector(RPOINTS* info_p, RPOINT* pts, RDR_INITCL* init);
	void ClearSectors();
	void Dump(CViewPortControl *vpControl);

	void RefreshTracks(vector<TRK*> *tracks);

	void RefreshImages(RIMAGE* info, void* pixels);

	void Init(RDR_INITCL* init);

	CRCPointModel * GetCRCPointFromRDRTRACK(RDRTRACK * tp) const;

	void SetCameraPositionFromMiniMapXY(float x, float y, float direction) const;

	C3DObjectModel *GetObjectAtMiniMapPosition(int vpId, glm::vec3 p0, glm::vec3 p1) const;
	C3DObjectModel *GetSectorPoint(CViewPortControl *vpControl, glm::vec2 screenPoint, int& index);
	C3DObjectModel *GetFirstTrackBetweenPoints(CViewPortControl *vpControl, glm::vec2 screenPoint, int& index); //returns point from track under cursor
	C3DObjectModel *GetPointOnSurface(glm::vec3 p0, glm::vec3 p1) const;

	glm::vec2 CameraXYForMiniMap() const;

	void DrawBitmaps() const;

	void SetBegAzm(double begAzm);

	glm::vec3 GetGeographicCoordinates(glm::vec3 glCoords);
};