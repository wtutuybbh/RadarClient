#pragma once
#include "stdafx.h"

//#pragma comment( lib, "opengl32.lib" )							// Search For OpenGL32.lib While Linking
//#pragma comment( lib, "glu32.lib" )								// Search For GLu32.lib While Linking
//#pragma comment( lib, "glaux.lib" )								// Search For GLaux.lib While Linking

//#ifndef CDS_FULLSCREEN											// CDS_FULLSCREEN Is Not Defined By Some
//#define CDS_FULLSCREEN 4										// Compilers. By Defining It This Way,
//#endif															// We Can Avoid Errors

// TUTORIAL
// Mesh Generation Paramaters
//#define MESH_RESOLUTION 1.0f									// Pixels Per Vertex
//#define MESH_HEIGHTSCALE 2.0f									// Mesh Height Scale
//#define NO_VBOS												// If Defined, VBOs Will Be Forced Off

// VBO Extension Definitions, From glext.h
/*
#define GL_ARRAY_BUFFER_ARB 0x8892
#define GL_STATIC_DRAW_ARB 0x88E4
typedef void (APIENTRY * PFNGLBINDBUFFERARBPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRY * PFNGLDELETEBUFFERSARBPROC) (GLsizei n, const GLuint *buffers);
typedef void (APIENTRY * PFNGLGENBUFFERSARBPROC) (GLsizei n, GLuint *buffers);
typedef void (APIENTRY * PFNGLBUFFERDATAARBPROC) (GLenum target, int size, const GLvoid *data, GLenum usage);
*/
// VBO Extension Function Pointers
/*PFNGLGENBUFFERSARBPROC glGenBuffersARB = NULL;					// VBO Name Generation Procedure
PFNGLBINDBUFFERARBPROC glBindBufferARB = NULL;					// VBO Bind Procedure
PFNGLBUFFERDATAARBPROC glBufferDataARB = NULL;					// VBO Data Loading Procedure
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB = NULL;			// VBO Deletion Procedure
*/

#include "C3DObjectModel.h"
#include "CRCAltitudeDataFile.h"
//#include "CScene.h"
//#include "FreeImage.h"
//#include <vector>
//#include "ShaderUtils.h"

class CRCTextureDataFile;
class CScene;

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

struct VBOData;

class CMesh : public C3DObjectModel
{	
	bool LoadHeightmap();
	AltitudeMapHeader* GetAltitudeMapHeader(const char *fileName, double lon1, double lat1, double lon2, double lat2);
	ImageMapHeader* GetImageMapHeader(const char *imgFile, const char *datFile);
	float PtHeight(int nX, int nY) const;
	int texsize;

	glm::vec3 * Bounds{ nullptr };	

	CRCTextureDataFile *maptexture { nullptr };
	CRCAltitudeDataFile *altitude{ nullptr };

	bool clearAfter;

	glm::vec2 position;
	int resolution;
	double max_range;

	float MPPh, MPPv;

	unsigned short *idxArray{ nullptr };

	std::vector<VBOData> * buffer{ nullptr };
	bool buffer_ready{ false };
	int index_length{ 0 };
	float centerHeight, averageHeight;
public:
	glm::vec3 Size;
	
	int UseTexture, UseY0Loc;
	CMesh(bool clearAfter, glm::vec2 position, double max_range, int texsize, int resolution, float MPPh, float MPPv);
	~CMesh() override;
	float GetCenterHeight();
	float GetAverageHeight();
	glm::vec3 GetSize();
	bool IntersectLine(int vpId, glm::vec3 & orig, glm::vec3 & dir, glm::vec3 & position) override;
	void BindUniforms(CViewPortControl *vpControl) override;
	glm::vec3 * CMesh::GetBounds() override;
	void InitMiniMap();
	void LoadToGPU(int vpId);
	bool Ready();
};