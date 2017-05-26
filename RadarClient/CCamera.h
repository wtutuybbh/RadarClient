#pragma once
#include "stdafx.h"

typedef void(*LookAtCallback)(double eyex, double eyey, double eyez, double centerx, double centery, double centerz, double upx, double upy, double upz);
enum MovementType { FORWARD, BACKWARD, STRAFE_L, STRAFE_R };

typedef void(*ApplyPositionBoundsCallback)(float &x, float &y, float &z);

class CCamera {
private:
	static const std::string requestID;
	glm::vec3 Position;
	glm::vec3 Up0;	
	
public:
	
	glm::vec3 Direction;
	glm::vec3 Up;
	glm::vec2 startPosition, newPosition; //for dragging
	float fovy;
	float aspect;
	float zNear;
	float zFar;
	float Speed;
	glm::vec3 RadarPosition {glm::vec3(0,0,0)};
	glm::vec3 MeshSize;
	bool FixViewOnRadar = false;

	CCamera();
	~CCamera();

	void SetAll(float eyex, float eyey, float eyez, float centerx, float centery, float centerz, float upx, float upy, float upz,
		float fovy, float aspect, float zNear, float zFar,
		float speed, LookAtCallback lookAt);
	void SetAll(float eyex, float eyey, float eyez, float centerx, float centery, float centerz, float upx, float upy, float upz,
		float fovy, float aspect, float zNear, float zFar,
		float speed);

	void SetProjection(float fovy, float aspect, float zNear, float zFar);
	void SetPosition(float x, float y, float z);
	void SetDirection(float x, float y, float z);
	void SetUp(float x, float y, float z);
	void Reset();
	void SetPositionXZ(float x, float z);
	void SetAspect(float aspect);
	void SetFovy(float fovy);
	void SetZPlanes(float zNear, float zFar);
	void LookAt();
	void MoveByView(double shift);
	void Rotate(float amount, glm::vec3& axis);
	void Move(glm::vec3 const & direction, bool preserveDirection);
	void ApplyMovement(MovementType movement);

	glm::mat4 GetView();
	glm::mat4 GetProjection();

	glm::mat4 GetMiniMapView();
	glm::mat4 GetMiniMapProjection();

	glm::vec3 GetPosition();
	glm::vec3 GetUp();

	float GetFovy();
	float GetAspect();
	float GetZNear();
	float GetZFar();

	float GetAzimut();

	glm::vec3 GetDirection();

	int flag0;
	float left, right, top, bottom, znear, zfar; //ortho projection
	glm::vec3 mmPosition, mmTo, mmUp;

	void ApplyPositionBounds(ApplyPositionBoundsCallback apb);

private:
	LookAtCallback lookAt {nullptr};
	ApplyPositionBoundsCallback applyPositionBounds{ nullptr };
};