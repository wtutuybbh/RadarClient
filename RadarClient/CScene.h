#pragma once
#include "glm/glm.hpp"
//#include "CMesh.h"
#include <string>
#include <vector>

#define ZEROLEVEL_ZERO 0
#define ZEROLEVEL_MAXHEIGHT 1
#define ZEROLEVEL_ACTUALHEIGHT 2

#define ZERO_ELEVATION 0.523599f //30 degrees
#define RAY_HEIGHT 0.174533f //10 degrees

//using namespace std;

class CMesh;
class CVert;													// Vertex Class
typedef CVert CVec;												// The Definitions Are Synonymous
class CTexCoord;												// Texture Coordinate Class
class CColorRGBA;
class CRCPoint;
class CCamera;

#include "CRCSocket.h"
#include "CUserInterface.h"

class C3DObject;
class CMinimapPointer;

class CScene {
public:
	float mpph; // meters per pixel vertical
	float mppv; // meters per pixel horizontal
	glm::tvec2<float, glm::precision::defaultp> geocenter; //geographic coordinates of center point (place of the radar)
	int msize; // area square's side length in meters
	CMesh *mesh;
	glm::vec3 *meshBounds;

	std::string altFile, imgFile, datFile;
	
	int texsize;
	
	int width, height; //window width and height
	float RotatingSpeed; //degrees per second

	// the next variables should be placed in each's class:

	float rayWidth, minE, maxE, minDist, maxDist;
	int rayDensity, rayArraySize;

	float AxisGridShift = 50.0f;
	float YAxisLength = 1000.0f;
	float markSize = 10.0f;
	int markDistance = 100;


	int vertexCount_Ray;
	int vertexCount_Axis;
	int vertexCount_Info, infoWidth, infoHeight;

	int numCircles, marksPerCircle, segmentsPerCircle;

	CVert *AxisGrid, *Ray;
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

	std::vector<CRCPoint*> *Sectors;
	int SectorsCount;

	CCamera *Camera;

	CRCSocket *Socket;
	CUserInterface *UI;

	bool VBOisBuilt, RayVBOisBuilt, MiniMapVBOisBuilt;

	CMinimapPointer *MiniMapPointer;

public:
	/*CScene(float lonc, float latc);*/
	CScene(std::string alfFile, std::string imgFile, std::string datFile, float lonc, float latc, float mpph, float mppv, int texsize);
	~CScene();

	bool DrawScene();
	bool MiniMapDraw();
	
	bool PrepareVBOs();
	bool PrepareRayVBO(RDR_INITCL* init);

	bool MiniMapPrepareAndBuildVBO();

	bool BuildVBOs();
	bool BuildRayVBO();

	void RefreshSector(RPOINTS* info_p, RPOINT* pts, RDR_INITCL* init);
	void ClearSectors();

	void SetCameraPositionFromMiniMapXY(float x, float y, float direction) const;

	C3DObject *GetObjectAtMiniMapPosition(float x, float y) const;
	C3DObject *GetFirstObjectBetweenPoints(glm::vec3 p0, glm::vec3 p1) const;

	glm::vec2 CameraXYForMiniMap() const;
};