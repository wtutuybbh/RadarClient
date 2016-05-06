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

/*class CVert;
typedef CVert CVec;*/
class C3DObject
{
protected:
	std::vector<VBOData> VBOBuffer;
	int VBOBufferSize;
	unsigned int VBOName, VAOName;
	GLuint ProgramID;
	bool VBOReady, VBOClearAfter;
	unsigned int TextureId;

	FIBITMAP *Image;
	char *ImgFile, *VShaderFile, *FShaderFile;

	//PtrToMethodMap DrawMethodsSequence;

	glm::mat4 Model, View, Proj, MVP;

	std::vector<VBOData> MiniMapVBOBuffer;
	int MiniMapVBOBufferSize;
	unsigned int MiniMapVBOName, MiniMapVAOName;
	GLuint MiniMapProgramID;
	bool MiniMapVBOReady, MiniMapVBOClearAfter;
	unsigned int	MiniMapTextureId;

	FIBITMAP *MiniMapImage;

	char *MiniMapImgFile, *MiniMapVShaderFile, *MiniMapFShaderFile;

	//PtrToMethodMap MiniMapDrawMethodsSequence;

	glm::mat4 MiniMapModel, MiniMapView, MiniMapProj, MiniMapMVP;

	
public:
	CScene *Scene;
	glm::vec4 Color;
	C3DObject();
	explicit C3DObject(bool initMap);
	virtual ~C3DObject();
	virtual glm::vec3 * GetBounds();

	glm::vec3 CartesianCoords, SphericalCoords;

	virtual bool MiniMapPrepareAndBuildVBO(const char * vShaderFile, const char * fShaderFile, const char * imgFile);
	virtual void MiniMapBuildVBO();
	virtual void MiniMapPrepareVBO();
	virtual void MiniMapAttribBind();
	virtual void MiniMapBindTextureImage();
	virtual void MiniMapUnbindAll();
	virtual void MiniMapCreateProgram();
	virtual void MiniMapDraw(CCamera *cam);

	virtual bool PrepareAndBuildVBO(const char * vShaderFile, const char * fShaderFile, const char * imgFile);
	virtual void BuildVBO();
	virtual void PrepareVBO();
	virtual void AttribBind();
	virtual void BindTextureImage();
	virtual void UnbindAll();
	virtual void CreateProgram();
	virtual void Draw(CCamera *cam);


	virtual bool MiniMapIntersectLine(glm::vec3 & orig, glm::vec3 & dir, glm::vec3 & position);
	virtual bool IntersectLine(glm::vec3 & orig, glm::vec3 & dir, glm::vec3 & position);
	
	virtual float DistanceToLine(glm::vec3 p0, glm::vec3 p1);
};

