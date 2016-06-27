#include "C3DObjectTexture.h"
#include <FreeImage.h>
#include <GL/glew.h>

void C3DObjectTexture::InitBits()
{
	bits = new BYTE[sizeX * sizeY * 4];

	BYTE *pixels = (BYTE*)FreeImage_GetBits(image);

	for (int pix = 0; pix < sizeX * sizeY; pix++)
	{
		bits[pix * 4 + 0] = pixels[pix * 4 + 2];
		bits[pix * 4 + 1] = pixels[pix * 4 + 1];
		bits[pix * 4 + 2] = pixels[pix * 4 + 0];
		bits[pix * 4 + 3] = pixels[pix * 4 + 3];
	}
}

void C3DObjectTexture::LoadToGPU()
{
	if (!ready && (useBits && bits || !useBits && image)) {
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sizeX, sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, useBits ? bits : (void*)FreeImage_GetBits(image));

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		ready = true;
		if (useBits && bits)
		{
			delete[] bits;
			bits = NULL;
		}
		if (clearAfter) {
			FreeImage_Unload(image);
			image = NULL;
		}
	}
}

void C3DObjectTexture::UnBind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

C3DObjectTexture* C3DObjectTexture::Clone()
{
	if (imgFile.length()>0)
		return new C3DObjectTexture(imgFile.c_str(), textureUniformName);

	return new C3DObjectTexture(image, textureUniformName, clearAfter, useBits);
}

C3DObjectTexture::~C3DObjectTexture()
{
	if (bits)
		delete[] bits;
	if (image)
		FreeImage_Unload(image);
}

C3DObjectTexture::C3DObjectTexture(const char* imgFile, const char *textureUniformName)
{	
	this->imgFile = imgFile?string(imgFile):string();
	try {
		image = FreeImage_Load(FreeImage_GetFileType(imgFile, 0), imgFile);
	}
	catch (...) {
		return;
	}
	if (FreeImage_GetBPP(image) != 32)
	{
		FIBITMAP* tempImage = image;
		image = FreeImage_ConvertTo32Bits(tempImage);
	}
	sizeX = FreeImage_GetWidth((FIBITMAP*)image);
	sizeY = FreeImage_GetHeight((FIBITMAP*)image);
	useBits = true;
	clearAfter = false;
	InitBits();

	FreeImage_Unload(image);
	image = NULL;

	this->textureUniformName = (char *)textureUniformName;

	ready = false;
}

C3DObjectTexture::C3DObjectTexture(FIBITMAP* image, const char* textureUniformName, bool clearAfter, bool useBits)
{
	this->useBits = useBits;
	this->image = image;
	sizeX = FreeImage_GetWidth((FIBITMAP*)image);
	sizeY = FreeImage_GetHeight((FIBITMAP*)image);
	if (useBits) {
		InitBits();
	}	

	this->textureUniformName = (char *)textureUniformName;

	ready = false;
}

void C3DObjectTexture::Bind(unsigned int programId) const
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureId);

	int iSamplerLoc = glGetUniformLocation(programId, textureUniformName);
	glUniform1i(iSamplerLoc, 0);
}

void C3DObjectTexture::UnloadImage()
{
	FreeImage_Unload(image);
	image = NULL;
}
