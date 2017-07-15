#pragma once
#include "stdafx.h"

using namespace std;

class C3DObjectTexture
{
	string imgFile;
	unsigned int textureId {0};
	char *textureUniformName;
	FIBITMAP *image { nullptr };
	int sizeX;
	int sizeY;
	unsigned char *bytes { nullptr };
	void InitBytes();
	
	bool ready {false};
	bool useBytes;
	bool clearAfter;
	int dim{ 2 };
	float *floatData{ nullptr };
	void *data;
	
public:
	int needsReload{ 1 };
	virtual ~C3DObjectTexture();

	C3DObjectTexture(const char *textureUniformName, float *data, int size);
	C3DObjectTexture(const char *imgFile, const char *textureUniformName);
	C3DObjectTexture(FIBITMAP *image, const char *textureUniformName, bool clearAfter, bool useBits);
	void SetImage(FIBITMAP *image);
	void SetImage(float *data, int size);
	void Bind(unsigned int programId) const;
	void UnloadImage();
	void LoadToGPU();
	void UnloadFromGPU() const;
	void Reload();
	void Clear();
	static void UnBind();
	virtual C3DObjectTexture* Clone();
	void SetBytes(unsigned char *bytes, int sizeX, int sizeY);
	unsigned char * GetBytes() const;
};
