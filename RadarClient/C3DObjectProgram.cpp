#include "stdafx.h"
#include "C3DObjectProgram.h"
#include "ShaderUtils.h"
#include "C3DObjectVBO.h"


C3DObjectProgram::~C3DObjectProgram()
{
}

C3DObjectProgram::C3DObjectProgram(const char* vShaderFile, const char* fShaderFile, const char* vertexAttribName, 
	const char * textureAttribName, const char * normalAttribName, const char * colorAttribName, const char *  color2AttribName)
{
	SetNames(vShaderFile, fShaderFile, vertexAttribName, textureAttribName, normalAttribName, colorAttribName, color2AttribName);
	
	this->ready = false;
	SetPlaces(17 * sizeof(float), 0, 15, 4, 7, 11);
}

C3DObjectProgram::C3DObjectProgram(const char* vShaderFile, const char* fShaderFile, const char* vertexAttribName, const char* textureAttribName, const char* normalAttribName, const char* colorAttribName)
{
	SetNames(vShaderFile, fShaderFile, vertexAttribName, textureAttribName, normalAttribName, colorAttribName, string().c_str());

	this->ready = false;
	SetPlaces(13 * sizeof(float), 0, 11, 4, 7, 0);
}

void C3DObjectProgram::SetPlaces(int elementSize, int vertexPlace, int texcoorPlace, int normalPlace, int colorPlace, int color2Place)
{
	this->elementSize = elementSize;

	this->vertexPlace = vertexPlace;
	this->texcoorPlace = texcoorPlace;
	this->normalPlace = normalPlace;
	this->colorPlace = colorPlace;
	this->color2Place = color2Place;
}

void C3DObjectProgram::SetNames(const char* vShaderFile, const char* fShaderFile, const char* vertexAttribName, const char* textureAttribName, const char* normalAttribName, const char* colorAttribName, const char* color2AttribName)
{
	this->vShaderFile = vShaderFile ? string(vShaderFile) : string();
	this->fShaderFile = fShaderFile ? string(fShaderFile) : string();
	this->vertexAttribName = vertexAttribName ? string(vertexAttribName) : string();
	this->textureAttribName = textureAttribName ? string(textureAttribName) : string();
	this->normalAttribName = normalAttribName ? string(normalAttribName) : string();
	this->colorAttribName = colorAttribName ? string(colorAttribName) : string();
	this->color2AttribName = color2AttribName ? string(color2AttribName) : string();
}

int C3DObjectProgram::GetUniformLocation(const char* uniformName) const
{
	return glGetUniformLocation(ProgramId, uniformName);
}

void C3DObjectProgram::UseProgram()
{	
	glUseProgram(ProgramId);
}

void C3DObjectProgram::CreateProgram()
{
	if (!ready)
	{
		ProgramId = create_program(vShaderFile.c_str(), fShaderFile.c_str());
		ready = true;
	}
}

void C3DObjectProgram::DoNotUseProgram()
{
	glUseProgram(0);
}

void C3DObjectProgram::Bind()
{
	if (elementSize <= 0)
		throw std::exception("C3DObjectProgram not initialized");
	if (this->ProgramId)
	{
		//UseProgram();
		if (vertexAttribName.length()>0) {
			GLint vertex_attr_loc = glGetAttribLocation(this->ProgramId, vertexAttribName.c_str());
			glVertexAttribPointer(vertex_attr_loc, 4, GL_FLOAT, GL_FALSE, elementSize, (void*)(sizeof(float) * vertexPlace));
			glEnableVertexAttribArray(vertex_attr_loc);
		}
		if (textureAttribName.length()>0) {
			GLint texcoor_attr_loc = glGetAttribLocation(this->ProgramId, textureAttribName.c_str());
			glVertexAttribPointer(texcoor_attr_loc, 2, GL_FLOAT, GL_FALSE, elementSize, (void*)(sizeof(float) * texcoorPlace));
			glEnableVertexAttribArray(texcoor_attr_loc);
		}
		if (normalAttribName.length()>0) {
			GLint normal_attr_loc = glGetAttribLocation(this->ProgramId, normalAttribName.c_str());
			glVertexAttribPointer(normal_attr_loc, 3, GL_FLOAT, GL_FALSE, elementSize, (void*)(sizeof(float) * normalPlace));
			glEnableVertexAttribArray(normal_attr_loc);
		}
		if (colorAttribName.length()>0) {
			GLint color_attr_loc = glGetAttribLocation(this->ProgramId, colorAttribName.c_str());
			glVertexAttribPointer(color_attr_loc, 4, GL_FLOAT, GL_TRUE, elementSize, (void*)(sizeof(float) * colorPlace));
			glEnableVertexAttribArray(color_attr_loc);
		}
		if (color2AttribName.length()>0) {
			GLint color2_attr_loc = glGetAttribLocation(this->ProgramId, color2AttribName.c_str());
			glVertexAttribPointer(color2_attr_loc, 4, GL_FLOAT, GL_TRUE, elementSize, (void*)(sizeof(float) * color2Place));
			glEnableVertexAttribArray(color2_attr_loc);
		}
	}
}
