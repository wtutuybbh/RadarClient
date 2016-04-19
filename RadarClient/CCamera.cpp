#include "CCamera.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"


CCamera::CCamera() //empty constructor. camera depends on scene, scene depends on camera... oh shit!
{
}

CCamera::CCamera(float eyex, float eyey, float eyez, float centerx, float centery, float centerz, float upx, float upy, float upz,
	float fovy, float aspect, float zNear, float zFar, 
	float speed, LookAtCallback lookAt) {
	
	Position.x = eyex;
	Position.y = eyey;
	Position.z = eyez;
	Direction.x = centerx - eyex;
	Direction.y = centery - eyey;
	Direction.z = centerz - eyez;
	Up.x = upx;
	Up.y = upy;
	Up.z = upz;

	this->fovy = fovy;
	this->aspect = aspect;
	this->zNear = zNear;
	this->zFar = zFar;

	this->lookAt = lookAt;
	Speed = speed;
}

CCamera::~CCamera() {

}

void CCamera::SetAll(float eyex, float eyey, float eyez, float centerx, float centery, float centerz, float upx, float upy, float upz, float fovy, float aspect, float zNear, float zFar, float speed, LookAtCallback lookAt)
{
	SetAll(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz, fovy, aspect, zNear, zFar, speed);
	this->lookAt = lookAt;
}
void CCamera::SetAll(float eyex, float eyey, float eyez, float centerx, float centery, float centerz, float upx, float upy, float upz, float fovy, float aspect, float zNear, float zFar, float speed)
{
	this->Position = glm::vec3(eyex, eyey, eyez);

	this->Direction = glm::vec3(centerx - eyex, centery - eyey, centerz - eyez);

	this->Up = glm::vec3(upx, upy, upz);

	this->fovy = fovy;
	this->aspect = aspect;
	this->zNear = zNear;
	this->zFar = zFar;

	this->Speed = speed;
}
void CCamera::SetProjection(float fovy, float aspect, float zNear, float zFar)
{
	this->fovy = fovy;
	this->aspect = aspect;
	this->zNear = zNear;
	this->zFar = zFar;
}

void CCamera::LookAt() {
	//m_view = glm::lookAt(m_position, m_position + m_direction, m_up);
	glm::vec3 to = Position + Direction;
	lookAt(Position.x, Position.y, Position.z, to.x, to.y, to.z, Up.x, Up.y, Up.z);
	//gluLookAt(Position.x, Position.y, Position.z, to.x, to.y, to.z, Up.x, Up.y, Up.z);
}
void CCamera::MoveByView(double shift) {
	Direction = glm::normalize(Direction);
	Position = glm::mat3(shift) * glm::normalize(Direction) + Position;
}

void CCamera::Rotate(float amount, glm::vec3& axis)
{
	Direction = glm::rotate(Direction, amount * Speed, axis);
}

void CCamera::Translate(glm::vec3 & direction)
{
	Position += direction;
}

void CCamera::ApplyMovement(MovementType movement)
{
	switch (movement)
	{
	case FORWARD:
		Position += Direction;
		break;
	case BACKWARD:
		Position -= Direction;
		break;
	case STRAFE_L:
		Position += glm::cross(Direction, Up);
		break;
	case STRAFE_R:
		Position -= glm::cross(Direction, Up);
		break;
	}
}

glm::mat4 CCamera::GetView()
{
	return glm::lookAt(Position, Position + Direction, Up);
}

glm::mat4 CCamera::GetProjection()
{
	return glm::perspective(glm::radians(fovy), aspect, zNear, zFar);
}
