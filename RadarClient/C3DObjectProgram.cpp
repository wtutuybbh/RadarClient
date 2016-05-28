#include "C3DObjectProgram.h"
#include "ShaderUtils.h"
#include "Util.h"

C3DObjectProgram::~C3DObjectProgram()
{
}

C3DObjectProgram::C3DObjectProgram(const char* vShaderFile, const char* fShaderFile, const char* vertexAttribName, const char * textureAttribName, const char * normalAttribName, const char * colorAttribName)
{
	this->vShaderFile = vShaderFile;
	this->fShaderFile = fShaderFile;
	this->vertexAttribName = vertexAttribName;
	this->textureAttribName = textureAttribName;
	this->normalAttribName = normalAttribName;
	this->colorAttribName = colorAttribName;
	this->ready = false;
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
		ProgramId = create_program(vShaderFile, fShaderFile);
		ready = true;
	}
}

void C3DObjectProgram::DoNotUseProgram()
{
	glUseProgram(0);
}

void C3DObjectProgram::Bind()
{
	if (this->ProgramId)
	{
		//UseProgram();
		if (vertexAttribName) {
			GLint vertex_attr_loc = glGetAttribLocation(this->ProgramId, vertexAttribName);
			glVertexAttribPointer(vertex_attr_loc, 4, GL_FLOAT, GL_FALSE, sizeof(VBOData), (void*)0);
			glEnableVertexAttribArray(vertex_attr_loc);
		}
		if (textureAttribName) {
			GLint texcoor_attr_loc = glGetAttribLocation(this->ProgramId, textureAttribName);
			glVertexAttribPointer(texcoor_attr_loc, 2, GL_FLOAT, GL_FALSE, sizeof(VBOData), (void*)(sizeof(float) * 11));
			glEnableVertexAttribArray(texcoor_attr_loc);
		}
		if (normalAttribName) {
			GLint normal_attr_loc = glGetAttribLocation(this->ProgramId, normalAttribName);
			glVertexAttribPointer(normal_attr_loc, 3, GL_FLOAT, GL_FALSE, sizeof(VBOData), (void*)(sizeof(float) * 4));
			glEnableVertexAttribArray(normal_attr_loc);
		}
		if (colorAttribName) {
			GLint color_attr_loc = glGetAttribLocation(this->ProgramId, colorAttribName);
			glVertexAttribPointer(color_attr_loc, 3, GL_FLOAT, GL_FALSE, sizeof(VBOData), (void*)(sizeof(float) * 7));
			glEnableVertexAttribArray(color_attr_loc);
		}
	}
}

C3DObjectProgram* C3DObjectProgram::Clone() const
{
	return new C3DObjectProgram(vShaderFile, fShaderFile, vertexAttribName, textureAttribName, normalAttribName, colorAttribName);
}
