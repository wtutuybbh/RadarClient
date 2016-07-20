//#include "stdafx.h"
#include "C3DObject.h"
#include "glm/glm.hpp"
#include "glm/gtx/intersect.hpp"
#include "CUserInterface.h"
#include "CViewPortControl.h"
#include "C3DObjectProgram.h"
/*
old_C3DObject::old_C3DObject()
{
}

old_C3DObject::old_C3DObject(bool initMap)
{
	//MiniMapDrawMethodsSequence.insert({ 0, PtrToMethod(&old_C3DObject::MiniMapCreateProgram) });
	//MiniMapDrawMethodsSequence.insert({ 1, (PtrToMethod)(&old_C3DObject::MiniMapBuildVBO) });
	//MiniMapDrawMethodsSequence.insert({ 2, (PtrToMethod)(&old_C3DObject::MiniMapPrepareVBO) });
	//MiniMapDrawMethodsSequence.insert({ 3, (PtrToMethod)(&old_C3DObject::MiniMapAttribBind) });
	//MiniMapDrawMethodsSequence.insert({ 4, (PtrToMethod)(&old_C3DObject::MiniMapBindTextureImage) });
	//MiniMapDrawMethodsSequence.insert({ 5, (PtrToMethod)(&old_C3DObject::MiniMapUnbindAll) });
	MiniMapVBOReady = false;
	MiniMapProgramID = 0;
	MiniMapVBOBuffer.clear(); //destroy all vbo buffer objects
	std::vector<VBOData>().swap(MiniMapVBOBuffer); //free memory used by vector itself

	MiniMapVBOClearAfter = true;
}
old_C3DObject::~old_C3DObject()
{
	//MiniMapDrawMethodsSequence.clear();
}

glm::vec3 * old_C3DObject::GetBounds()
{
	return nullptr;
}

bool old_C3DObject::MiniMapPrepareAndBuildVBO(const char * vShaderFile, const char * fShaderFile, const char * imgFile)
{
	MiniMapImgFile = const_cast<char *>(imgFile), MiniMapVShaderFile = const_cast<char *>(vShaderFile), MiniMapFShaderFile = const_cast<char *>(fShaderFile);

	MiniMapCreateProgram();
	MiniMapBuildVBO();
	MiniMapPrepareVBO();
	MiniMapAttribBind();
	MiniMapBindTextureImage();
	MiniMapUnbindAll();

	//old_C3DObject *obj = this;
	//for (PtrToMethodMap::iterator it = MiniMapDrawMethodsSequence.begin(); it != MiniMapDrawMethodsSequence.end(); ++it) {
	//	CALL_MEMBER_FN(*obj, it->second)();
	//}
	return true;
}

void old_C3DObject::MiniMapBuildVBO()
{
	MiniMapVBOBuffer.push_back({ glm::vec4(-1, -1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	MiniMapVBOBuffer.push_back({ glm::vec4(-1, 1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 1) });
	MiniMapVBOBuffer.push_back({ glm::vec4(1, 1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(1, 1) });

	MiniMapVBOBuffer.push_back({ glm::vec4(1, 1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(1, 1) });
	MiniMapVBOBuffer.push_back({ glm::vec4(1, -1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(1, 0) });
	MiniMapVBOBuffer.push_back({ glm::vec4(-1, -1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });

	MiniMapVBOBufferSize = MiniMapVBOBuffer.size();
}
void old_C3DObject::MiniMapPrepareVBO()
{
	glGenVertexArrays(1, &MiniMapVAOName);
	glBindVertexArray(MiniMapVAOName);

	glGenBuffers(1, &MiniMapVBOName);
	glBindBuffer(GL_ARRAY_BUFFER, MiniMapVBOName);
	glBufferData(GL_ARRAY_BUFFER, MiniMapVBOBufferSize * sizeof(VBOData), &MiniMapVBOBuffer[0], GL_STATIC_DRAW);

	if (MiniMapVBOClearAfter) {
		MiniMapVBOBuffer.clear(); //destroy all vbo buffer objects
		std::vector<VBOData>().swap(MiniMapVBOBuffer); //free memory used by vector itself
	}
}
void old_C3DObject::MiniMapAttribBind()
{
	GLuint vertex_attr_loc;
	GLuint texcoor_attr_loc;

	vertex_attr_loc = glGetAttribLocation(MiniMapProgramID, "vertex");
	texcoor_attr_loc = glGetAttribLocation(MiniMapProgramID, "texcoor");

	glVertexAttribPointer(vertex_attr_loc, 4, GL_FLOAT, GL_FALSE, sizeof(VBOData), (void*)0);
	glVertexAttribPointer(texcoor_attr_loc, 2, GL_FLOAT, GL_FALSE, sizeof(VBOData), (void*)(sizeof(float) * 11));

	glEnableVertexAttribArray(vertex_attr_loc);
	glEnableVertexAttribArray(texcoor_attr_loc);
}*/

/*void old_C3DObject::MiniMapBindTextureImage()
{
	try {
		MiniMapImage = FreeImage_Load(FreeImage_GetFileType(MiniMapImgFile, 0), MiniMapImgFile);
	}
	catch (...) {
		return;
	}

	if (FreeImage_GetBPP(MiniMapImage) != 32)
	{
		FIBITMAP* tempImage = MiniMapImage;
		MiniMapImage = FreeImage_ConvertTo32Bits(tempImage);
	}

	int sizeX = FreeImage_GetWidth((FIBITMAP*)MiniMapImage);
	int sizeY = FreeImage_GetHeight((FIBITMAP*)MiniMapImage);

	BYTE *bits = new BYTE[sizeX * sizeY * 4];

	BYTE *pixels = (BYTE*)FreeImage_GetBits(MiniMapImage);

	for (int pix = 0; pix < sizeX * sizeY; pix++)
	{
		bits[pix * 4 + 0] = pixels[pix * 4 + 2];
		bits[pix * 4 + 1] = pixels[pix * 4 + 1];
		bits[pix * 4 + 2] = pixels[pix * 4 + 0];
		bits[pix * 4 + 3] = pixels[pix * 4 + 3];
	}

	glGenTextures(1, &MiniMapTextureId);
	glBindTexture(GL_TEXTURE_2D, MiniMapTextureId);


	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sizeX, sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, bits);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	FreeImage_Unload(MiniMapImage);
	delete bits;
}*/

/*void old_C3DObject::MiniMapUnbindAll()
{
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}*/

/*void old_C3DObject::MiniMapCreateProgram()
{
	if (!MiniMapProgramID) {
		MiniMapProgramID = create_program(MiniMapVShaderFile, MiniMapFShaderFile);
	}
}*/

/*void old_C3DObject::MiniMapDraw(CCamera *cam)
{
	glUseProgram(MiniMapProgramID);
	glBindVertexArray(MiniMapVAOName);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, MiniMapTextureId);

	int iSamplerLoc = glGetUniformLocation(MiniMapProgramID, "tex");
	glUniform1i(iSamplerLoc, 0);
	
	//mvp = cam->GetProjection() * mv;
	//norm = glm::mat3(glm::transpose(glm::inverse(mv)));
	//glm::mat3(1.0f);

	int mvpUniformLoc = glGetUniformLocation(MiniMapProgramID, "mvp");
	//int normUniformLoc = glGetUniformLocation(MiniMapProgramID, "norm");
	MiniMapMVP = cam->GetMiniMapProjection() * cam->GetMiniMapView() * MiniMapModel;
	glUniformMatrix4fv(mvpUniformLoc, 1, GL_FALSE, glm::value_ptr(MiniMapMVP));
	//glUniformMatrix3fv(normUniformLoc, 1, GL_FALSE, glm::value_ptr(norm));
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDrawArrays(GL_TRIANGLES, 0, MiniMapVBOBufferSize);
	glBindVertexArray(0);
	glUseProgram(0);
}*/

/*bool old_C3DObject::PrepareAndBuildVBO(const char* vShaderFile, const char* fShaderFile, const char* imgFile)
{
	return true;
}

void old_C3DObject::BuildVBO()
{
}

void old_C3DObject::PrepareVBO()
{
}

void old_C3DObject::AttribBind()
{
}

void old_C3DObject::BindTextureImage()
{
}

void old_C3DObject::UnbindAll()
{
}

void old_C3DObject::CreateProgram()
{
}

void old_C3DObject::Draw(CCamera* cam)
{
}

bool old_C3DObject::MiniMapIntersectLine(glm::vec3 & orig, glm::vec3 & dir, glm::vec3 & position)
{
	glm::vec3 vert0, vert1, vert2;
	for (unsigned int i = 0; i < MiniMapVBOBuffer.size(); i += 3) {
		vert0 = glm::vec3(MiniMapModel*MiniMapVBOBuffer[i].vert);
		vert1 = glm::vec3(MiniMapModel*MiniMapVBOBuffer[i + 1].vert);
		vert2 = glm::vec3(MiniMapModel*MiniMapVBOBuffer[i + 2].vert);
		if (glm::intersectLineTriangle(orig, dir, vert0, vert1, vert2, position)) {
			return true;
		}
	}
	return false;
}

bool old_C3DObject::IntersectLine(glm::vec3 & orig, glm::vec3 & dir, glm::vec3 & position)
{
	glm::vec3 vert0, vert1, vert2;
	for (unsigned int i = 0; i < VBOBuffer.size(); i += 3) {
		vert0 = glm::vec3(Model*VBOBuffer[i].vert);
		vert1 = glm::vec3(Model*VBOBuffer[i + 1].vert);
		vert2 = glm::vec3(Model*VBOBuffer[i + 2].vert);
		if (glm::intersectLineTriangle(orig, dir, vert0, vert1, vert2, position)) {
			return true;
		}
	}
	
	return false;
}

float old_C3DObject::DistanceToLine(glm::vec3 p0, glm::vec3 p1)
{
	return MinimumDistance(p0, p1, CartesianCoords);
}

float old_C3DObject::DistanceToPoint(glm::vec3 p)
{
	return glm::distance(p, CartesianCoords);
}

void old_C3DObject::SetColor(glm::vec4 color)
{
	Color = color;
}

//void old_C3DObject::SelectObject(CUserInterface * ui)
//{
//}

glm::mat4 old_C3DObject::GetModelMatrix(CScene* scn)
{
	return Scale * Rotate * Translate;
}*/

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

C3DObjectModel::C3DObjectModel(int vpId, C3DObjectVBO* vbo, C3DObjectTexture* tex, C3DObjectProgram* prog)
{
	this->vbo.insert_or_assign(vpId, vbo);
	this->tex.insert_or_assign(vpId, tex);
	this->prog.insert_or_assign(vpId, prog);

	translateMatrix.insert_or_assign(vpId, glm::mat4(1.0f));
	scaleMatrix.insert_or_assign(vpId, glm::mat4(1.0f));
	rotateMatrix.insert_or_assign(vpId, glm::mat4(1.0f));	
}

C3DObjectModel::C3DObjectModel(C3DObjectVBO* vbo, C3DObjectTexture* tex, C3DObjectProgram* prog)
{
	this->vbo.insert_or_assign(Main, vbo);
	this->tex.insert_or_assign(Main, tex);
	this->prog.insert_or_assign(Main, prog);

	this->vbo.insert_or_assign(MiniMap, vbo ? vbo->Clone() : NULL);
	this->tex.insert_or_assign(MiniMap, tex ? tex->Clone() : NULL);
	this->prog.insert_or_assign(MiniMap, prog ? prog->Clone() : NULL);

	translateMatrix.insert_or_assign(Main, glm::mat4(1.0f));
	scaleMatrix.insert_or_assign(Main, glm::mat4(1.0f));
	rotateMatrix.insert_or_assign(Main, glm::mat4(1.0f));

	translateMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
	scaleMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
	rotateMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
}

C3DObjectModel::~C3DObjectModel()
{
	
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

		for (unsigned int i = 0; i < buffer->size(); i += 3) {
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
	return NULL;
}

void C3DObjectModel::SetColor(glm::vec4 color)
{
	Color = color;
}
