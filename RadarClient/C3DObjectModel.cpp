#include "stdafx.h"

#include "C3DObjectModel.h"
#include "CUserInterface.h"
#include "CViewPortControl.h"
#include "C3DObjectProgram.h"
#include "C3DObjectVBO.h"
#include "C3DObjectTexture.h"

const std::string C3DObjectModel::requestID = "C3DObjectModel";

void* C3DObjectModel::GetBufferAt(int index)
{
	C3DObjectVBO* _vbo;
	try
	{
		_vbo = (C3DObjectVBO*)vbo.at(index);
		if (_vbo)
		{
			return _vbo->GetBuffer();
		}				
	}
	catch (...)
	{
		
	}
	return nullptr;
}

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
C3DObjectModel::C3DObjectModel(int vpId, C3DObjectVBO* vbo, C3DObjectTexture* tex, C3DObjectProgram* prog)
{
	this->id = _id;
	_id++;

	this->vbo.insert_or_assign(vpId, vbo);
	this->tex.insert_or_assign(vpId, tex);
	this->prog.insert_or_assign(vpId, prog);

	translateMatrix.insert_or_assign(vpId, glm::mat4(1.0f));
	scaleMatrix.insert_or_assign(vpId, glm::mat4(1.0f));
	rotateMatrix.insert_or_assign(vpId, glm::mat4(1.0f));	
}

C3DObjectModel::C3DObjectModel(C3DObjectVBO* vbo, C3DObjectTexture* tex, C3DObjectProgram* prog)
{
	this->id = _id;
	_id++;

	this->vbo.insert_or_assign(Main, vbo);
	this->tex.insert_or_assign(Main, tex);
	this->prog.insert_or_assign(Main, prog);

	this->vbo.insert_or_assign(MiniMap, vbo ? vbo->Clone() : nullptr);
	this->tex.insert_or_assign(MiniMap, tex ? tex->Clone() : nullptr);
	this->prog.insert_or_assign(MiniMap, prog ? prog->Clone() : nullptr);

	translateMatrix.insert_or_assign(Main, glm::mat4(1.0f));
	scaleMatrix.insert_or_assign(Main, glm::mat4(1.0f));
	rotateMatrix.insert_or_assign(Main, glm::mat4(1.0f));

	translateMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
	scaleMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
	rotateMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
}

C3DObjectModel::~C3DObjectModel()
{
	if (id == _testid)
	{
		int dummy;
		dummy = 1 + 1;
	}
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

void C3DObjectModel::Draw(CViewPortControl* vpControl, GLenum mode)
{
	C3DObjectVBO *vbo_ = vbo.at(vpControl->Id);
	C3DObjectProgram *prog_ = prog.at(vpControl->Id);
	C3DObjectTexture *tex_ = tex.at(vpControl->Id);

	if(vbo_ && !vbo_->Ready())
	{
		if (!vbo_->HasBuffer())
			return;
		if (prog_) prog_->CreateProgram();
		vbo_->LoadToGPU();
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
		if (vbo_->NeedsReload)
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
		glm::mat4 mvp = vpControl->GetProjMatrix() * vpControl->GetViewMatrix() * GetModelMatrix(vpControl);
		glUniformMatrix4fv(mvpUniformLoc, 1, GL_FALSE, glm::value_ptr(mvp));
	}
}

glm::mat4 C3DObjectModel::GetModelMatrix(CViewPortControl* vpControl)
{
	glm::mat4 mMatrix = GetTranslateMatrix(vpControl) * GetRotateMatrix(vpControl) * GetScaleMatrix(vpControl);
	modelMatrix.insert_or_assign(vpControl->Id, mMatrix);
	return mMatrix;
}

glm::mat4 C3DObjectModel::GetScaleMatrix(CViewPortControl* vpControl)
{
	return scaleMatrix.at(vpControl->Id);
	try
	{
		return scaleMatrix.at(vpControl->Id);
	}
	catch (...)
	{
		return glm::mat4(1.0f);
	}
}

glm::mat4 C3DObjectModel::GetRotateMatrix(CViewPortControl* vpControl)
{
	return rotateMatrix.at(vpControl->Id);
	try 
	{	
		return rotateMatrix.at(vpControl->Id);
	}
	catch (...)
	{
		return glm::mat4(1.0f);
	}
}

glm::mat4 C3DObjectModel::GetTranslateMatrix(CViewPortControl* vpControl)
{
	return translateMatrix.at(vpControl->Id);
	try {
		return translateMatrix.at(vpControl->Id);
	}
	catch (...)
	{
		return glm::mat4(1.0f);
	}
}

bool C3DObjectModel::IntersectLine(int vpId, glm::vec3& orig, glm::vec3& dir, glm::vec3& position)
{
	try {
		glm::vec3 vert0, vert1, vert2;
		std::vector<VBOData> *buffer = (std::vector<VBOData> *)vbo.at(vpId)->GetBuffer();

		for (unsigned int i = 0; i < buffer->size() && i+1 < buffer->size() && i+2 < buffer->size(); i += 3) {
			vert0 = glm::vec3(modelMatrix.at(vpId)*(*buffer)[i].vert);
			vert1 = glm::vec3(modelMatrix.at(vpId)*(*buffer)[i + 1].vert);
			vert2 = glm::vec3(modelMatrix.at(vpId)*(*buffer)[i + 2].vert);
			if (glm::intersectLineTriangle(orig, dir, vert0, vert1, vert2, position)) {
				return true;
			}
		}
	}
	catch (...) {
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

void C3DObjectModel::SetColor(glm::vec4 color)
{
	Color = color;
}
