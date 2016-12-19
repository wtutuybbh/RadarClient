#pragma once
#include "stdafx.h"

class C3DObjectVBO;
class C3DObjectTexture;
class C3DObjectProgram;
class CViewPortControl;
class CScene;
class CUserInterface;
class CViewPortControl;

class C3DObjectModel
{	
protected:
	static const std::string requestID;
	std::unordered_map<int, C3DObjectVBO*> vbo;
	std::unordered_map<int, C3DObjectTexture*> tex;
	std::unordered_map<int, C3DObjectProgram*> prog;
	std::unordered_map<int, glm::mat4> scaleMatrix, rotateMatrix, translateMatrix, modelMatrix;
	glm::vec3 cartesianCoords, sphericalCoords, geoCoords;
	void *GetBufferAt(int index);
	std::string c3DObjectModel_TypeName {"C3DObjectModel"};
public:
	static int _id, _testid;
	int id;
	void SetCartesianCoordinates(glm::vec4 c);
	void SetCartesianCoordinates(glm::vec3 c);
	glm::vec3 GetCartesianCoords();
	void SetGeoCoords(glm::vec3 c);
	glm::vec3 GetGeoCoords();
	void SetCartesianCoordinates(float x, float y, float z);


	CScene *Scene {nullptr};
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

	std::string GetTypeName();
};