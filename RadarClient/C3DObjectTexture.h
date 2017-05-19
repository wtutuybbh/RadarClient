#pragma once
#include "stdafx.h"

using namespace std;

class C3DObjectTexture
{
protected:
	string imgFile {string()};
	unsigned int textureId {0};
	char *textureUniformName {nullptr};
	FIBITMAP *image { nullptr };
	int sizeX {0};
	int sizeY {0};
	unsigned char *bits { nullptr };
	void InitBits();
	
	bool ready {false};
	bool useBits {false};
	bool clearAfter {false};
	bool deleteBits{ true };
	int dim{ 2 };
	GLint internalFormat{ GL_RGBA };
	GLenum pdformat{ GL_RGBA };
	GLenum type{ GL_UNSIGNED_BYTE };
public:
	virtual ~C3DObjectTexture();

	C3DObjectTexture();
	explicit C3DObjectTexture(const char *textureUniformName);
	C3DObjectTexture(const char *imgFile, const char *textureUniformName);
	C3DObjectTexture(FIBITMAP *image, const char *textureUniformName, bool clearAfter, bool useBits);
	void Reload(FIBITMAP *image);
	void Reload(unsigned char *data, int sizeX, int sizeY);
	void Bind(unsigned int programId) const;
	void UnloadImage();
	void LoadToGPU();
	void UnBind() const;
	virtual C3DObjectTexture* Clone();
	void SetDim(int dim);
	void SetFormatsAndType(GLint internalFormat, GLenum pdformat, GLenum type);
};
