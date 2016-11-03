#include "stdafx.h"
#include "CCamera.h"
#include "Util.h"
#include "CRCLogger.h"

CCamera::CCamera() //empty constructor. camera depends on scene, scene depends on camera... oh shit!
{
	std::string context = "CCamera::CCamera";
	CRCLogger::Info(context, "Start");

	flag0 = 1;

	left = -1;
	right = 1;
	bottom = -1;
	top = 1;
	znear = -1;
	zfar = 15000;
	mmPosition = glm::vec3(0.0f, 1000.0f, 0.0f);
	mmTo = glm::vec3(0.0f, 0.0f, 0.0f);
	mmUp = glm::vec3(0.0f, 0.0f, 1.0f);
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
	this->Up0 = glm::vec3(0, 1, 0);


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

void CCamera::SetPosition(float x, float y, float z)
{
	/*glm::vec3 prod = glm::cross(glm::normalize(Up0), glm::normalize(GetDirection()));
	if (glm::length(prod) < 0.000001) { //collinearity test
		Up = glm::vec3(0, 0, 1);
	}
	else {
		Up = glm::normalize(glm::normalize(GetDirection()) + Up0);
	}*/
	Position = glm::vec3(x, y, z);	
}

void CCamera::SetPositionXZ(float x, float z)
{
	SetPosition(x, Position.y, z);
}

void CCamera::LookAt() {
	//m_view = glm::lookAt(m_position, m_position + m_direction, m_up);
	glm::vec3 to = Position + GetDirection();
	lookAt(Position.x, Position.y, Position.z, to.x, to.y, to.z, Up.x, Up.y, Up.z);
	//gluLookAt(Position.x, Position.y, Position.z, to.x, to.y, to.z, Up.x, Up.y, Up.z);
}
void CCamera::MoveByView(double shift) {
	glm::vec3 dir = glm::normalize(GetDirection());
	Position = glm::mat3(shift) * dir + Position;
}

void CCamera::Rotate(float amount, glm::vec3& axis)
{
	Direction = glm::rotate(Direction, amount * Speed, axis);
}

void CCamera::Move(glm::vec3 const & direction, bool preserveDirection)
{
	if (!preserveDirection) {
		glm::vec3 to = Position + GetDirection();
		Position += direction;
		Direction = to - Position;
	}
	else 
	{
		Position += direction;
	}

}

void CCamera::ApplyMovement(MovementType movement)
{
	switch (movement)
	{
	case FORWARD:
		Position += glm::normalize(GetDirection());
		break;
	case BACKWARD:
		Position -= glm::normalize(GetDirection());
		break;
	case STRAFE_L:
		Position += glm::cross(glm::normalize(GetDirection()), Up);
		break;
	case STRAFE_R:
		Position -= glm::cross(glm::normalize(GetDirection()), Up);
		break;
	}
}

glm::mat4 CCamera::GetView()
{
	return glm::lookAt(Position, Position + GetDirection(), Up);
}

glm::mat4 CCamera::GetProjection()
{
	return glm::perspective(glm::radians(fovy), aspect, zNear, zFar);
}

glm::mat4 CCamera::GetMiniMapView()
{	
	return glm::lookAt(mmPosition, mmTo, mmUp);

	//return glm::lookAt(glm::vec3(0.0f, 1000.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
}

glm::mat4 CCamera::GetMiniMapProjection()
{
	if (flag0 == 0) {
		return glm::ortho(-1, 1, -1, 1);
	}
	else {
		return glm::ortho(-MeshSize.x/2, MeshSize.x/2, -MeshSize.z/2, MeshSize.z/2, znear, zfar);
	}
}

glm::vec3 CCamera::GetPosition()
{
	return Position;
}

float CCamera::GetAzimut()
{
	glm::vec3 dir = GetDirection();
	float r = glm::length(dir);
	return sgn(dir.x) *  acos(min(dir.z / sqrt(r*r - dir.y*dir.y), 1.0f));
}

glm::vec3 CCamera::GetDirection()
{
	if (FixViewOnRadar) {
		return RadarPosition - Position;
	}
	return Direction;
}
