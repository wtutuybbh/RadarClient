#pragma once
#include "stdafx.h"

using namespace std;

class C3DObjectProgram
{
	bool ready;
	string vShaderFile, fShaderFile, vertexAttribName, textureAttribName, normalAttribName, colorAttribName, color2AttribName;
	int vertexPlace{ 0 }, texcoorPlace{ 11 }, normalPlace{ 4 }, colorPlace{ 7 }, color2Place { 11 };
	int elementSize{ 0 };
public:	
	~C3DObjectProgram();
	C3DObjectProgram(const char* vShaderFile, const char* fShaderFile, const char* vertexAttribName, const char * textureAttribName, const char * normalAttribName, const char * colorAttribName, const char *  color2AttribName);
	C3DObjectProgram(const char* vShaderFile, const char* fShaderFile, const char* vertexAttribName, const char * textureAttribName, const char * normalAttribName, const char * colorAttribName);

	void SetPlaces(int elementSize, int vertexPlace, int texcoorPlace, int normalPlace, int colorPlace, int color2Place);
	void SetNames(const char* vShaderFile, const char* fShaderFile, const char* vertexAttribName,
		const char * textureAttribName, const char * normalAttribName, const char * colorAttribName, const char *  color2AttribName);

	int GetUniformLocation(const char* uniformName) const;
	void UseProgram();
	void CreateProgram();
	static void DoNotUseProgram();
	void Bind();
	unsigned int ProgramId;
};