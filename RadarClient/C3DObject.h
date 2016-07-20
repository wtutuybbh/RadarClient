#pragma once

#include <vector>
#include "CScene.h"
#include "CCamera.h"

#include "Util.h"
#include "FreeImage.h"
//#include <unordered_map>
#include "C3DObjectVBO.h"
#include "C3DObjectTexture.h"
#include "C3DObjectProgram.h"
#include "CViewPortControl.h"
/*
class old_C3DObject;

typedef void (old_C3DObject::*PtrToMethod)();

#define CALL_MEMBER_FN(object, ptrToMember)  ((object).*(ptrToMember))



typedef std::unordered_map<int, PtrToMethod> PtrToMethodMap;
*/
class CUserInterface;

/*class old_C3DObject
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

	glm::mat4 Scale, Rotate, Translate;
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
	old_C3DObject();
	explicit old_C3DObject(bool initMap);
	virtual ~old_C3DObject();
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

	virtual float DistanceToPoint(glm::vec3 p);

	virtual void SetColor(glm::vec4 color);

	//virtual void SelectObject(CUserInterface * ui);

	virtual glm::mat4 GetModelMatrix(CScene *scn);
};*/

class CViewPortControl;

class C3DObjectModel
{
protected:
	std::unordered_map<int, C3DObjectVBO*> vbo;
	std::unordered_map<int, C3DObjectTexture*> tex;
	std::unordered_map<int, C3DObjectProgram*> prog;
	std::unordered_map<int, glm::mat4> scaleMatrix, rotateMatrix, translateMatrix, modelMatrix;
	glm::vec3 cartesianCoords, sphericalCoords, geoCoords;
public:
	void SetCartesianCoordinates(glm::vec4 c);
	void SetCartesianCoordinates(glm::vec3 c);
	glm::vec3 GetCartesianCoords();
	void SetGeoCoords(glm::vec3 c);
	glm::vec3 GetGeoCoords();
	void SetCartesianCoordinates(float x, float y, float z);


	CScene *Scene {NULL};
	glm::vec4 Color {glm::vec4()};
	C3DObjectModel(int vpId, C3DObjectVBO *vbo, C3DObjectTexture *tex, C3DObjectProgram *prog);
	C3DObjectModel(C3DObjectVBO *vbo, C3DObjectTexture *tex, C3DObjectProgram *prog);
	virtual ~C3DObjectModel();
	virtual void Draw(CViewPortControl *vpControl, GLenum mode);
	virtual void BindUniforms(CViewPortControl* vpControl);
	virtual glm::mat4 GetModelMatrix(CViewPortControl* vpControl);
	virtual glm::mat4 GetScaleMatrix(CViewPortControl* vpControl);
	virtual glm::mat4 GetRotateMatrix(CViewPortControl* vpControl);
	virtual glm::mat4 GetTranslateMatrix(CViewPortControl* vpControl);

	virtual void SetColor(glm::vec4 color); //trash?

	virtual bool IntersectLine(int vpId, glm::vec3 & orig, glm::vec3 & dir, glm::vec3 & position);	
	virtual float DistanceToLine(glm::vec3 p0, glm::vec3 p1);
	virtual float DistanceToPoint(glm::vec3 p);

	virtual void SelectObject(CUserInterface * ui);

	virtual void Init(int vpId);
	C3DObjectVBO * GetC3DObjectVBO(int vpId);

	virtual glm::vec3 * GetBounds();
};