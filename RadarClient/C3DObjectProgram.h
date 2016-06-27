#pragma once
#include "Util.h"
#include <string>

using namespace std;

class C3DObjectProgram
{
	bool ready;
	string vShaderFile, fShaderFile, vertexAttribName, textureAttribName, normalAttribName, colorAttribName;
	int vertexPlace{ 0 }, texcoorPlace{ 11 }, normalPlace{ 4 }, colorPlace{ 7 };
	int elementSize{ sizeof(VBOData) };
public:	
	~C3DObjectProgram();
	C3DObjectProgram(const char* vShaderFile, const char* fShaderFile, const char* vertexAttribName, const char * textureAttribName, const char * normalAttribName, const char * colorAttribName);
	int GetUniformLocation(const char* uniformName) const;
	void UseProgram();
	void CreateProgram();
	static void DoNotUseProgram();
	void Bind();
	unsigned int ProgramId;
	C3DObjectProgram *Clone() const;
};