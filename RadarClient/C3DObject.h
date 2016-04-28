#pragma once

#include <vector>
#include "CScene.h"
#include "CCamera.h"

#include "Util.h"
#include "ShaderUtils.h"
#include "FreeImage.h"
#include <unordered_map>

class C3DObject;

typedef void (C3DObject::*PtrToMethod)();

#define CALL_MEMBER_FN(object, ptrToMember)  ((object).*(ptrToMember))



typedef std::unordered_map<int, PtrToMethod> PtrToMethodMap;

class CScene;
class CVert;
typedef CVert CVec;
class C3DObject
{
protected:
	std::vector<VBOData> VBOBuffer;
	int VBOBufferSize;
	unsigned int VBOName, VAOName;
	GLuint ProgramID;
	bool VBOReady;

	GLuint MVPUniformLoc;
	GLuint NormUniformLoc;


	std::vector<VBOData> MiniMapVBOBuffer;
	int MiniMapVBOBufferSize;
	unsigned int MiniMapVBOName, MiniMapVAOName;
	GLuint MiniMapProgramID;
	bool MiniMapVBOReady, MiniMapVBOClearAfter;
	unsigned int	MiniMapTextureId;

	FIBITMAP *MiniMapImage;

	char *ImgFile, *VShaderFile, *FShaderFile;

	PtrToMethodMap map;

	glm::mat4 MiniMapModel, MiniMapView, MiniMapProj, MiniMapMVP;

	glm::mat4 Model, View, Proj, MVP;
public:
	CScene *Scene;
	glm::vec4 Color;
	C3DObject();
	C3DObject(bool initMap);
	~C3DObject();
	virtual glm::vec3 * GetBounds();

	glm::vec3 CartesianCoords, SphericalCoords;

	bool MiniMapPrepareAndBuildVBO(const char * vShaderFile, const char * fShaderFile, const char * imgFile);

	void MiniMapBuildVBO();
	void MiniMapPrepareVBO();
	void MiniMapAttribBind();
	void MiniMapBindTextureImage();
	void MiniMapUnbindAll();
	void MiniMapCreateProgram();

	void MiniMapDraw();

	bool MiniMapIntersectLine(glm::vec3 & orig, glm::vec3 & dir, glm::vec3 & position);

	bool IntersectLine(glm::vec3 & orig, glm::vec3 & dir, glm::vec3 & position);
	
	float DistanceToLine(glm::vec3 p0, glm::vec3 p1);
};

