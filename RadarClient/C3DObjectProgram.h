#pragma once
class C3DObjectProgram
{
	bool ready;
	const char *vShaderFile, *fShaderFile, *vertexAttribName, *textureAttribName, *normalAttribName, *colorAttribName;
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