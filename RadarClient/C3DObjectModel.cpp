#include "stdafx.h"

#include "C3DObjectModel.h"
#include "CUserInterface.h"
#include "CViewPortControl.h"
#include "C3DObjectProgram.h"
#include "C3DObjectVBO.h"
#include "C3DObjectTexture.h"
#include "CRCLogger.h"

const std::string C3DObjectModel::requestID = "C3DObjectModel";

void C3DObjectModel::SetCartesianCoordinates(glm::vec4 c)
{
	cartesianCoords = glm::vec3(c);
}

void C3DObjectModel::SetCartesianCoordinates(glm::vec3 c)
{
	cartesianCoords = c;
}

glm::vec3 C3DObjectModel::GetCartesianCoords()
{
	return cartesianCoords;
}

void C3DObjectModel::SetGeoCoords(glm::vec3 c)
{
	geoCoords = c;
}

glm::vec3 C3DObjectModel::GetGeoCoords()
{
	return geoCoords;
}

void C3DObjectModel::SetCartesianCoordinates(float x, float y, float z)
{
	cartesianCoords = glm::vec3(x, y, z);
}

int C3DObjectModel::_id;
int C3DObjectModel::_testid;
//C3DObjectModel::C3DObjectModel(int vpId, C3DObjectVBO* vbo, C3DObjectTexture* tex, C3DObjectProgram* prog) : C3DObjectModel()
//{
//	this->vbo.insert_or_assign(vpId, vbo);
//	this->tex.insert_or_assign(vpId, tex);
//	this->prog.insert_or_assign(vpId, prog);
//
//	translateMatrix.insert_or_assign(vpId, glm::mat4(1.0f));
//	scaleMatrix.insert_or_assign(vpId, glm::mat4(1.0f));
//	rotateMatrix.insert_or_assign(vpId, glm::mat4(1.0f));	
//}
//
//C3DObjectModel::C3DObjectModel(C3DObjectVBO* vbo, C3DObjectTexture* tex, C3DObjectProgram* prog) : C3DObjectModel()
//{
//	this->vbo.insert_or_assign(Main, vbo);
//	this->tex.insert_or_assign(Main, tex);
//	this->prog.insert_or_assign(Main, prog);
//
//	this->vbo.insert_or_assign(MiniMap, vbo ? vbo->Clone() : nullptr);
//	this->tex.insert_or_assign(MiniMap, tex ? tex->Clone() : nullptr);
//	this->prog.insert_or_assign(MiniMap, prog ? prog->Clone() : nullptr);
//
//	translateMatrix.insert_or_assign(Main, glm::mat4(1.0f));
//	scaleMatrix.insert_or_assign(Main, glm::mat4(1.0f));
//	rotateMatrix.insert_or_assign(Main, glm::mat4(1.0f));
//
//	translateMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
//	scaleMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
//	rotateMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
//}
C3DObjectModel::C3DObjectModel() {
	this->id = _id;
	_id++;
}
C3DObjectModel::~C3DObjectModel()
{
	std::lock_guard<std::mutex> lock(m);

	//if (id == _testid)
	//{
	//	int dummy;
	//	dummy = 1 + 1;
	//}
	if (vbo.size()>0)
	{
		for (auto it = begin(vbo); it != end(vbo); ++it)
			if (it->second)
				delete it->second;
	}
	if (prog.size()>0)
	{
		for (auto it = begin(prog); it != end(prog); ++it)
			if (it->second)
				delete it->second;
	}
	if (tex.size()>0)
	{
		for (auto it = begin(tex); it != end(tex); ++it)
			if (it->second)
				delete it->second;
	}
}
void C3DObjectModel::CreateBuffer(C3DObjectVBO *vbo_) {

}
void C3DObjectModel::Draw(CViewPortControl* vpControl, GLenum mode)
{
	//std::lock_guard<std::mutex> lock(m);

	if (!vpControl) return;
	
	if (vbo.find(vpControl->Id) == vbo.end()) {
		return;
	}
	C3DObjectVBO *vbo_= vbo.at(vpControl->Id);
	if (!vbo_)	return;


	if (prog.find(vpControl->Id) == prog.end()) {
		return;
	}
	C3DObjectProgram *prog_ = prog.at(vpControl->Id);
	if (!prog_)	return;

	C3DObjectTexture *tex_ = nullptr;
	if (tex.find(vpControl->Id) != tex.end()) {
		tex_ = tex.at(vpControl->Id);
	}

	bool can_be_reloaded = true;
	if(vbo_ && !vbo_->Ready())
	{
		if (!vbo_->HasBuffer())
			CreateBuffer(vbo_);
		if (!vbo_->HasBuffer())
			return;
		if (prog_) prog_->CreateProgram();
		vbo_->LoadToGPU();
		can_be_reloaded = false;
		if (prog_) prog_->Bind();
		if (tex_) {
			tex_->LoadToGPU();
			tex_->UnBind();
		}
		vbo_->UnBind();
	}

	if (prog_)
		prog_->UseProgram();
	if (vbo_) {
		vbo_->Bind();
		if (can_be_reloaded && vbo_->vertices && vbo_->vertices.get()->needsReload)
			vbo_->Reload();
	}

	if (tex_)
		tex_->Bind(prog_->ProgramId);

	BindUniforms(vpControl);
	
	if (vbo_)
		vbo_->Draw(mode);

	if (vbo_)
		vbo_->UnBind();
	
	if (prog_)
		prog_->DoNotUseProgram();
}

void C3DObjectModel::BindUniforms(CViewPortControl* vpControl)
{
	if (prog.at(vpControl->Id)) {
		int mvpUniformLoc = prog.at(vpControl->Id)->GetUniformLocation("mvp");
		auto p = vpControl->GetProjMatrix();
		auto v = vpControl->GetViewMatrix();
		auto m = GetModelMatrix(vpControl->Id);
		glm::mat4 mvp = p * v * m;
		glUniformMatrix4fv(mvpUniformLoc, 1, GL_FALSE, glm::value_ptr(mvp));
	}
}

glm::mat4 C3DObjectModel::GetModelMatrix(int vpId)
{
	auto t = GetTranslateMatrix(vpId);
	auto r = GetRotateMatrix(vpId);
	auto s = GetScaleMatrix(vpId);
	glm::mat4 mMatrix = t * r * s;
	//modelMatrix.insert_or_assign(vpControl->Id, mMatrix);
	return mMatrix;
}

glm::mat4 C3DObjectModel::GetScaleMatrix(int vpId)
{
	if (scaleMatrix.find(vpId) == scaleMatrix.end()) {
		scaleMatrix.insert_or_assign(vpId, glm::mat4(1.0f));
		return glm::mat4(1.0f);
	}
	return scaleMatrix.at(vpId);	
}

glm::mat4 C3DObjectModel::GetRotateMatrix(int vpId)
{
	if (rotateMatrix.find(vpId) == rotateMatrix.end()) {
		rotateMatrix.insert_or_assign(vpId, glm::mat4(1.0f));
	}
	return rotateMatrix.at(vpId);
}

glm::mat4 C3DObjectModel::GetTranslateMatrix(int vpId)
{
	if (translateMatrix.find(vpId) == translateMatrix.end()) {
		translateMatrix.insert_or_assign(vpId, glm::mat4(1.0f));
	}
	return translateMatrix.at(vpId);
}

bool C3DObjectModel::IntersectLine(int vpId, glm::vec3& orig, glm::vec3& dir, glm::vec3& position)
{
	try {
		glm::vec3 vert0, vert1, vert2;
		if (vbo.find(vpId) == vbo.end()) {
			return false;
		}

		if (!vbo.at(vpId))
			return false;
		if (!vertices) {
			vertices = vbo.at(vpId)->vertices;
			if (!vertices)
				return false;
		}

		auto vbuffer = vertices.get()->GetBuffer();
		auto vertexSize = vertices.get()->vertexSize;
		auto vertexCount = vertices.get()->vertexCount;

		if (!vbuffer || vertexCount == 0)
			return false;

		auto mm = GetModelMatrix(vpId);

		for (unsigned int i = 0; i < vertexCount && i+1 < vertexCount && i+2 < vertexCount; i += 3) {
			vert0 = glm::vec3(mm* *vertices.get()->getv(i));
			vert1 = glm::vec3(mm* *vertices.get()->getv(i + 1));
			vert2 = glm::vec3(mm* *vertices.get()->getv(i + 2));
			if (glm::intersectLineTriangle(orig, dir, vert0, vert1, vert2, position)) {
				return true;
			}
		}
	}
	catch (const std::exception &ex) {
		LOG_WARN("exception", "C3DObjectModel::IntersectLine", ex.what());
		return false;
	}
	return false;
}

float C3DObjectModel::DistanceToLine(glm::vec3 p0, glm::vec3 p1)
{
	return MinimumDistance(p0, p1, cartesianCoords);
}

float C3DObjectModel::DistanceToPoint(glm::vec3 p)
{
	return glm::distance(p, cartesianCoords);
}

void C3DObjectModel::SelectObject(CUserInterface* ui)
{
}

void C3DObjectModel::Init(int vpId)
{
}

C3DObjectVBO* C3DObjectModel::GetC3DObjectVBO(int vpId)
{
	return vbo.at(vpId);
}

glm::vec3* C3DObjectModel::GetBounds()
{
	return nullptr;
}

std::string C3DObjectModel::GetTypeName()
{
	return c3DObjectModel_TypeName;
}

void C3DObjectModel::SetVBO(int vpId, C3DObjectVBO* _vbo)
{
	vbo.insert_or_assign(vpId, _vbo);
}

void C3DObjectModel::SetRotateMatrix(int vpId, glm::mat4 rotate)
{
	rotateMatrix.insert_or_assign(vpId, rotate);
}

void C3DObjectModel::SetRotateMatrix(glm::mat4 rotate)
{
	for (auto iter = rotateMatrix.begin(); iter != rotateMatrix.end(); ++iter)
	{
		auto vpId = iter->first;
		SetRotateMatrix(vpId, rotate);
	}
}

std::string C3DObjectModel::GetName()
{
	return name;
}

void C3DObjectModel::SetName(std::string name)
{
	this->name = name;
}

void C3DObjectModel::SetColor(glm::vec4 color)
{
	Color = color;
}

void C3DObjectModel::Translate(glm::vec3 vshift)
{
	for(auto it = vbo.begin(); it != vbo.end(); ++it)
	{
		if(it->second && it->second->vertices && it->second->vertices.get()->vertexCount>0 && (!vertices || vertices != it->second->vertices))
		{
			it->second->vertices.get()->Translate(vshift, 0);
		}
	}
	if (vertices)
		vertices.get()->Translate(vshift, 0);
}
