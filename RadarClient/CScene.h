#pragma once
#include "glm/glm.hpp"
//#include "CMesh.h"
#include <string>
#include <vector>
//#include "C3DObject.h"
#include "glm/detail/type_mat.hpp"
//#include "CTrack.h"

//#include "CMesh.h"
//#include "CTrack.h"
class CTrack;

#define ZEROLEVEL_ZERO 0
#define ZEROLEVEL_MAXHEIGHT 1
#define ZEROLEVEL_ACTUALHEIGHT 2

//#define ZERO_ELEVATION 0.523599f //30 degrees
#define RAY_HEIGHT 0.174533f //10 degrees

//using namespace std;

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

#include "CRCSocket.h"
#include "CUserInterface.h"

class old_C3DObject;
class old_CMinimapPointer;
class CMiniMapPointer;
class CMesh;
class CScene {
public:
	float maxAmp = 0;

	float mpph; // meters per pixel vertical
	float mppv; // meters per pixel horizontal
	glm::vec2 geocenter; //geographic coordinates of center point (place of the radar)
	int msize; // area square's side length in meters
	old_CMesh *old_Mesh;
	glm::vec3 *meshBounds;

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

	glm::vec3 *AxisGrid, *Ray;
	CColorRGBA *AxisGridColor, *RayColor;



	unsigned int AxisGrid_VBOName, AxisGrid_VBOName_c;
	unsigned int Ray_VBOName, Ray_VBOName_c;

	unsigned short vertexCount, markCount;

	float viewAngle;

	///arrays for different VBO objects:
	unsigned short *markup;
	unsigned short **circles;
	unsigned short *ray;
	unsigned short *info;

	int zeroLevel = ZEROLEVEL_ACTUALHEIGHT;
	float y0;

	//std::vector<old_CRCPoint*> *old_Sectors;
	std::vector<CSector*> Sectors;


	std::unordered_map<int, CTrack*> Tracks;

	int SectorsCount;

	CCamera *Camera;

	CRCSocket *Socket;
	CUserInterface *UI;

	bool VBOisBuilt, RayVBOisBuilt, MiniMapVBOisBuilt;

	old_CMinimapPointer *MiniMapPointer;

	//RDR_INITCL * Init;
	bool Initialized;

	std::vector<old_C3DObject*> Selection;
	CMesh* Mesh;
	CMesh* Mesh1;
	CMesh* Mesh2;
	CMesh* Mesh3;

	CMiniMapPointer *mmPointer;

	CMarkup *Markup;

	CRCPointModel *testPoint;

	glm::vec3 MeshSize;
	glm::vec3 * m_Bounds;
	mutex *m;
	/*CScene(float lonc, float latc);*/
	CScene(std::string alfFile, std::string imgFile, std::string datFile, float lonc, float latc, float mpph, float mppv, int texsize, mutex *m);
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

	void RefreshTracks(vector<TRK*> *tracks);

	void Init(RDR_INITCL* init);

	CRCPointModel * GetCRCPointFromRDRTRACK(RDRTRACK * tp) const;

	void SetCameraPositionFromMiniMapXY(float x, float y, float direction) const;

	C3DObjectModel *GetObjectAtMiniMapPosition(int vpId, glm::vec3 p0, glm::vec3 p1) const;
	C3DObjectModel *GetSectorPoint(CViewPortControl *vpControl, glm::vec2 screenPoint, int& index);
	C3DObjectModel *GetFirstTrackBetweenPoints(int vpId, glm::vec3 p0, glm::vec3 p1) const; //returns point from track under cursor
	C3DObjectModel *GetPointOnSurface(glm::vec3 p0, glm::vec3 p1) const;

	glm::vec2 CameraXYForMiniMap() const;

	void DrawBitmaps() const;
};