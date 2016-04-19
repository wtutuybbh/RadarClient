#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"
typedef void(*LookAtCallback)(double eyex, double eyey, double eyez, double centerx, double centery, double centerz, double upx, double upy, double upz);
enum MovementType { FORWARD, BACKWARD, STRAFE_L, STRAFE_R };

class CCamera {
public:
	glm::vec3 Position;
	glm::vec3 Direction;
	glm::vec3 Up;
	glm::vec2 startPosition, newPosition;
	float fovy;
	float aspect;
	float zNear;
	float zFar;
	float Speed;
public:
	CCamera();
	CCamera(float eyex, float eyey, float eyez, float centerx, float centery, float centerz, float upx, float upy, float upz, 
		float fovy, float aspect, float zNear, float zFar,
		float speed, LookAtCallback lookAt);
	~CCamera();

	void SetAll(float eyex, float eyey, float eyez, float centerx, float centery, float centerz, float upx, float upy, float upz,
		float fovy, float aspect, float zNear, float zFar,
		float speed, LookAtCallback lookAt);
	void SetAll(float eyex, float eyey, float eyez, float centerx, float centery, float centerz, float upx, float upy, float upz,
		float fovy, float aspect, float zNear, float zFar,
		float speed);

	void SetProjection(float fovy, float aspect, float zNear, float zFar);

	void LookAt();
	void MoveByView(double shift);
	void Rotate(float amount, glm::vec3& axis);
	void Translate(glm::vec3& direction);
	void ApplyMovement(MovementType movement);

	glm::mat4 GetView();
	glm::mat4 GetProjection();
private:
	LookAtCallback lookAt;
};