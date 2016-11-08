#pragma once
#include "stdafx.h"

using namespace std;

class C3DObjectTexture
{
	string imgFile;
	unsigned int textureId;
	char *textureUniformName;
	FIBITMAP *image { nullptr };
	int sizeX;
	int sizeY;
	unsigned char *bits { nullptr };
	void InitBits();
	bool ready {false};
	bool useBits;
	bool clearAfter;
public:
	virtual ~C3DObjectTexture();

	C3DObjectTexture(const char *imgFile, const char *textureUniformName);
	C3DObjectTexture(FIBITMAP *image, const char *textureUniformName, bool clearAfter, bool useBits);
	void Bind(unsigned int programId) const;
	void UnloadImage();
	void LoadToGPU();
	static void UnBind();
	virtual C3DObjectTexture* Clone();
};
