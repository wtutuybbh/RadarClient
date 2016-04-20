#pragma once
#include <windows.h>
#include <GL/GL.h>

#pragma comment( lib, "opengl32.lib" )							// Search For OpenGL32.lib While Linking
#pragma comment( lib, "glu32.lib" )								// Search For GLu32.lib While Linking
//#pragma comment( lib, "glaux.lib" )								// Search For GLaux.lib While Linking

#ifndef CDS_FULLSCREEN											// CDS_FULLSCREEN Is Not Defined By Some
#define CDS_FULLSCREEN 4										// Compilers. By Defining It This Way,
#endif															// We Can Avoid Errors

// TUTORIAL
// Mesh Generation Paramaters
#define MESH_RESOLUTION 1.0f									// Pixels Per Vertex
#define MESH_HEIGHTSCALE 2.0f									// Mesh Height Scale
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

#include "C3DObject.h"
//#include "CScene.h"
#include "FreeImage.h"
#include "Util.h"
#include <vector>
#include "ShaderUtils.h"

class CScene;

typedef struct {
	GLint sizeX, sizeY;
	short *data;
} AltitudeMap;

typedef struct {
	char *fileName;
	int sizeX, sizeY, iLonStart, iLatStart, iLonEnd, iLatEnd, Nlon, Nlat;
	double lon0, lat0, dlon, dlat, lonSW, latSW;
} AltitudeMapHeader;

typedef struct {
	int sizeX, sizeY;
	double imgLon0, imgLat0, imgLon1, imgLat1;
} ImageMapHeader;

class CVert;													// Vertex Class

typedef CVert CVec;												// The Definitions Are Synonymous

class CTexCoord;												// Texture Coordinate Class


class CMesh : C3DObject
{
public:
	// Mesh Data
	int				m_nVertexCount;								// Vertex Count
	CVert*			m_pVertices;								// Vertex Data
	CTexCoord*		m_pTexCoords;								// Texture Coordinates
	unsigned int	m_nTextureId;								// Texture ID
	

																// Vertex Buffer Object Names
	unsigned int	m_nVBOVertices;								// Vertex VBO Name
	unsigned int	m_nVBOTexCoords;							// Texture Coordinate VBO Name

	short mpp;

	CScene *scn;
																// Temporary Data
	AltitudeMap* aMap;							// Heightmap Data
	AltitudeMapHeader* aMapH;
	ImageMapHeader* iMapH;

	CVec * m_Bounds;
	CVec * CMesh::GetBounds();

	void *bitmap;

	float lonStretch, latStretch;

	float centerHeight;

	int texsize;

	FIBITMAP *subimage;
	
public:
	CMesh(CScene *scn);													// Mesh Constructor
	~CMesh();													// Mesh Deconstructor

	AltitudeMap* GetAltitudeMap(const char *fileName, double lon1, double lat1, double lon2, double lat2);
	AltitudeMapHeader* GetAltitudeMapHeader(const char *fileName, double lon1, double lat1, double lon2, double lat2);
																// Heightmap Loader
	bool LoadHeightmap();
	//bool PrepareAndBuildMinimapVBO();

	void BindTextureImage();

	// Single Point Height
	float PtHeight(int nX, int nY);
	// VBO Build Function
	void BuildVBOs();

	ImageMapHeader* GetImageMapHeader(const char *imgFile, const char *datFile);

	void Draw();
	void DrawMiniMap();
};


typedef int(_cdecl * GDPALTITUDEMAP)(const char *, double *, int *, short *);
typedef int(_cdecl * GDPALTITUDEMAP_SIZES)(const char *, double *, int *);