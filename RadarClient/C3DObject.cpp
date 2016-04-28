//#include "stdafx.h"
#include "C3DObject.h"
#include "glm/glm.hpp"
#include "glm/gtx/intersect.hpp"


C3DObject::C3DObject()
{
}

C3DObject::C3DObject(bool initMap)
{
	map.insert({ 0, (PtrToMethod)(&C3DObject::MiniMapCreateProgram) });
	map.insert({ 1, (PtrToMethod)(&C3DObject::MiniMapBuildVBO) });
	map.insert({ 2, (PtrToMethod)(&C3DObject::MiniMapPrepareVBO) });
	map.insert({ 3, (PtrToMethod)(&C3DObject::MiniMapAttribBind) });
	map.insert({ 4, (PtrToMethod)(&C3DObject::MiniMapBindTextureImage) });
	map.insert({ 5, (PtrToMethod)(&C3DObject::MiniMapUnbindAll) });
	MiniMapVBOReady = false;
	MiniMapProgramID = 0;
	MiniMapVBOBuffer.clear(); //destroy all vbo buffer objects
	std::vector<VBOData>().swap(MiniMapVBOBuffer); //free memory used by vector itself

	MiniMapVBOClearAfter = true;
}
C3DObject::~C3DObject()
{
	map.clear();
}

glm::vec3 * C3DObject::GetBounds()
{
	return nullptr;
}

bool C3DObject::MiniMapPrepareAndBuildVBO(const char * vShaderFile, const char * fShaderFile, const char * imgFile)
{
	ImgFile = (char *)imgFile, VShaderFile = (char *)vShaderFile, FShaderFile = (char *)fShaderFile;

	/*CreateProgram();
	BuildMinimapVBO();
	MiniMapAttribBind();
	BindTextureImage();
	UnbindAll();*/

	C3DObject *obj = this;
	for (PtrToMethodMap::iterator it = map.begin(); it != map.end(); ++it) {
		CALL_MEMBER_FN(*obj, it->second)();
	}
	
	return true;
}

void C3DObject::MiniMapBuildVBO()
{
	MiniMapVBOBuffer.push_back({ glm::vec4(-1, -1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	MiniMapVBOBuffer.push_back({ glm::vec4(-1, 1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 1) });
	MiniMapVBOBuffer.push_back({ glm::vec4(1, 1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(1, 1) });

	MiniMapVBOBuffer.push_back({ glm::vec4(1, 1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(1, 1) });
	MiniMapVBOBuffer.push_back({ glm::vec4(1, -1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(1, 0) });
	MiniMapVBOBuffer.push_back({ glm::vec4(-1, -1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });

	MiniMapVBOBufferSize = MiniMapVBOBuffer.size();
}
void C3DObject::MiniMapPrepareVBO()
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
void C3DObject::MiniMapAttribBind()
{
	GLuint vertex_attr_loc;
	GLuint texcoor_attr_loc;

	vertex_attr_loc = glGetAttribLocation(MiniMapProgramID, "vertex");
	texcoor_attr_loc = glGetAttribLocation(MiniMapProgramID, "texcoor");

	glVertexAttribPointer(vertex_attr_loc, 4, GL_FLOAT, GL_FALSE, sizeof(VBOData), (void*)0);
	glVertexAttribPointer(texcoor_attr_loc, 2, GL_FLOAT, GL_FALSE, sizeof(VBOData), (void*)(sizeof(float) * 11));

	glEnableVertexAttribArray(vertex_attr_loc);
	glEnableVertexAttribArray(texcoor_attr_loc);
}

void C3DObject::MiniMapBindTextureImage()
{
	try {
		MiniMapImage = FreeImage_Load(FreeImage_GetFileType(ImgFile, 0), ImgFile);
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
}

void C3DObject::MiniMapUnbindAll()
{
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void C3DObject::MiniMapCreateProgram()
{
	if (!MiniMapProgramID) {
		MiniMapProgramID = create_program(VShaderFile, FShaderFile);
	}
}

void C3DObject::MiniMapDraw()
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
	MiniMapMVP = MiniMapProj * MiniMapView * MiniMapModel;
	glUniformMatrix4fv(mvpUniformLoc, 1, GL_FALSE, glm::value_ptr(MiniMapMVP));
	//glUniformMatrix3fv(normUniformLoc, 1, GL_FALSE, glm::value_ptr(norm));
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDrawArrays(GL_TRIANGLES, 0, MiniMapVBOBufferSize);
	glBindVertexArray(0);
	glUseProgram(0);
}

bool C3DObject::MiniMapIntersectLine(glm::vec3 & orig, glm::vec3 & dir, glm::vec3 & position)
{
	glm::vec3 vert0, vert1, vert2;
	for (int i = 0; i < MiniMapVBOBuffer.size(); i += 3) {
		vert0 = glm::vec3(MiniMapModel*MiniMapVBOBuffer[i].vert);
		vert1 = glm::vec3(MiniMapModel*MiniMapVBOBuffer[i + 1].vert);
		vert2 = glm::vec3(MiniMapModel*MiniMapVBOBuffer[i + 2].vert);
		if (glm::intersectLineTriangle(orig, dir, vert0, vert1, vert2, position)) {
			return true;
		}
	}
	return false;
}

bool C3DObject::IntersectLine(glm::vec3 & orig, glm::vec3 & dir, glm::vec3 & position)
{
	glm::vec3 vert0, vert1, vert2;
	for (int i = 0; i < VBOBuffer.size(); i += 3) {
		vert0 = glm::vec3(Model*VBOBuffer[i].vert);
		vert1 = glm::vec3(Model*VBOBuffer[i + 1].vert);
		vert2 = glm::vec3(Model*VBOBuffer[i + 2].vert);
		if (glm::intersectLineTriangle(orig, dir, vert0, vert1, vert2, position)) {
			return true;
		}
	}
	
	return false;
}

float C3DObject::DistanceToLine(glm::vec3 p0, glm::vec3 p1)
{
	return minimum_distance(p0, p1, CartesianCoords);
}
