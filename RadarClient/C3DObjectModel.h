#pragma once
#include "stdafx.h"
#include "C3DObjectVertices.h"


class C3DObjectVBO;
class C3DObjectTexture;
class C3DObjectProgram;
class CViewPortControl;
class CScene;
class CUserInterface;
class CViewPortControl;

enum AlphaBehaviour
{
	Constant,
	FadeOut
};

class C3DObjectModel
{	
protected:
	std::mutex m;
	//std::mutex mtxVertices;
	static const std::string requestID;
	std::unordered_map<int, C3DObjectVBO*> vbo;
	std::unordered_map<int, C3DObjectTexture*> tex;
	std::unordered_map<int, C3DObjectProgram*> prog;
	std::unordered_map<int, glm::mat4> scaleMatrix, rotateMatrix, translateMatrix, modelMatrix;
	glm::vec3 cartesianCoords, sphericalCoords, geoCoords;	
	std::string c3DObjectModel_TypeName {"C3DObjectModel"};
	std::string name{ "" };
	float useUniColor{ 0.0f };
	glm::vec4 uniColor{ glm::vec4(1.0, 0.0, 0.0, 1) };

	float alpha{ 1.0f };
	AlphaBehaviour alpha_behaviour{ Constant };
public:
	virtual void SetAlphaBehaviour(AlphaBehaviour ab);
	std::shared_ptr<C3DObjectVertices> vertices{ std::shared_ptr<C3DObjectVertices>(nullptr) };
	static int _id, _testid;
	int id;
	void SetCartesianCoordinates(glm::vec4 c);
	void SetCartesianCoordinates(glm::vec3 c);
	glm::vec3 GetCartesianCoords() const;
	void SetGeoCoords(glm::vec3 c);
	glm::vec3 GetGeoCoords() const;
	void SetCartesianCoordinates(float x, float y, float z);
	


	CScene *Scene {nullptr};
	glm::vec4 Color {glm::vec4()};
	float uniweight{ 0.0f };
	//C3DObjectModel(int vpId, C3DObjectVBO *vbo, C3DObjectTexture *tex, C3DObjectProgram *prog);
	//C3DObjectModel(C3DObjectVBO *vbo, C3DObjectTexture *tex, C3DObjectProgram *prog);
	C3DObjectModel();
	virtual ~C3DObjectModel();
	virtual void Draw(CViewPortControl* vpControl, GLenum mode);
	virtual void BindUniforms(CViewPortControl* vpControl);
	virtual glm::mat4 GetModelMatrix(int vpId);
	virtual glm::mat4 GetScaleMatrix(int vpId);
	virtual glm::mat4 GetRotateMatrix(int vpId);
	virtual glm::mat4 GetTranslateMatrix(int vpId);

	virtual void SetColor(glm::vec4 color); //trash?

	virtual void Translate(glm::vec3 vshift);

	virtual bool IntersectLine(int vpId, glm::vec3 & orig, glm::vec3 & dir, glm::vec3 & position);	
	virtual float DistanceToLine(glm::vec3 p0, glm::vec3 p1);
	virtual float DistanceToPoint(glm::vec3 p);

	virtual void SelectObject(CUserInterface * ui);

	virtual void Init(int vpId);
	virtual void CreateBuffer(C3DObjectVBO *vbo_);
	C3DObjectVBO * GetC3DObjectVBO(int vpId);

	virtual glm::vec3 * GetBounds();

	std::string GetTypeName() const;
	void SetVBO(int vpId, C3DObjectVBO *_vbo);

	void SetRotateMatrix(int vpId, glm::mat4 rotate);
	void SetRotateMatrix(glm::mat4 rotate);

	std::string GetName() const;
	void SetName(std::string name);
	void UseUniColor(float useUniColor);

};