#include "stdafx.h"
#include "C3DObjectTexture.h"
#include "Util.h"
#include "CRCLogger.h"

void C3DObjectTexture::InitBytes()
{
	bytes = new BYTE[sizeX * sizeY * 4];

	BYTE *pixels = (BYTE*)FreeImage_GetBits(image);

	for (int pix = 0; pix < sizeX * sizeY; pix++)
	{
		bytes[pix * 4 + 0] = pixels[pix * 4 + 2];
		bytes[pix * 4 + 1] = pixels[pix * 4 + 1];
		bytes[pix * 4 + 2] = pixels[pix * 4 + 0];
		bytes[pix * 4 + 3] = pixels[pix * 4 + 3];
	}

	
}

void C3DObjectTexture::SetBytes(unsigned char* bytes, int sizeX, int sizeY)
{
	if (this->bytes)
	{
		delete[] this->bytes;
	}
	this->bytes = bytes;
	this->sizeX = sizeX;
	this->sizeY = sizeY;
	needsReload = 1;
}

void C3DObjectTexture::LoadToGPU()
{
	switch (dim)
	{
	case 1:
		break;
	case 2:
		if (!ready && (useBytes && bytes || !useBytes && image)) {
			if (textureId > 0)
			{
				glDeleteTextures(1, &textureId);
			}
			glGenTextures(1, &textureId);
			glBindTexture(GL_TEXTURE_2D, textureId);

			if (!useBytes && image && FreeImage_GetBPP(image) != 32)
			{
				FIBITMAP* tempImage = image;
				image = FreeImage_ConvertTo32Bits(tempImage);
			}

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sizeX, sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, useBytes ? bytes : (void*)FreeImage_GetBits(image));

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			ready = true;
			if (useBytes && bytes && clearAfter)
			{
				delete[] bytes;
				bytes = nullptr;
			}
			if (clearAfter && image) {
				FreeImage_Unload(image);
				image = nullptr;
			}
			needsReload--;
		}
		break;
	}
}

void C3DObjectTexture::UnloadFromGPU() const
{
	if (textureId > 0)
	{
		glDeleteTextures(1, &textureId);
	}
}

void C3DObjectTexture::Reload()
{
	UnloadFromGPU();
	LoadToGPU();
}

void C3DObjectTexture::Clear()
{
	if (bytes)
	{
		delete [] bytes;
		bytes = nullptr;
	}
	if (floatData)
	{
		delete[] floatData;
		floatData = nullptr;
	}
	if (image) 
	{
		FreeImage_Unload(image);
		image = nullptr;
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

	return new C3DObjectTexture(image, textureUniformName, clearAfter, useBytes);
}

unsigned char* C3DObjectTexture::GetBytes() const
{
	return bytes;
}

C3DObjectTexture::~C3DObjectTexture()
{
	if (bytes)
		delete[] bytes;
	if (image)
		FreeImage_Unload(image);
	if (textureId > 0)
	{
		glDeleteTextures(1, &textureId);
	}
	if (floatData)
	{
		delete[] floatData;
	}
}

C3DObjectTexture::C3DObjectTexture(const char *textureUniformName, float* data, int size)
{
	this->textureUniformName = (char *)textureUniformName;
	SetImage(data, size);
}

C3DObjectTexture::C3DObjectTexture(const char* imgFile, const char *textureUniformName)
{		
	this->imgFile = imgFile?string(imgFile):string();
	try {
		image = FreeImage_Load(FreeImage_GetFileType(imgFile, 0), imgFile);
	}
	catch (const std::exception &ex) {
		LOG_WARN("exception", "C3DObjectTexture::C3DObjectTexture", ex.what());
		return;
	}
	if (FreeImage_GetBPP(image) != 32)
	{
		FIBITMAP* tempImage = image;
		image = FreeImage_ConvertTo32Bits(tempImage);
	}
	sizeX = FreeImage_GetWidth((FIBITMAP*)image);
	sizeY = FreeImage_GetHeight((FIBITMAP*)image);
	useBytes = true;
	clearAfter = false;
	InitBytes();

	FreeImage_Unload(image);
	image = nullptr;

	this->textureUniformName = (char *)textureUniformName;

	ready = false;
}

C3DObjectTexture::C3DObjectTexture(FIBITMAP* image, const char* textureUniformName, bool clearAfter, bool useBytes)
{
	this->clearAfter = clearAfter;
	this->useBytes = useBytes;	
	this->textureUniformName = (char *)textureUniformName;

	SetImage(image);
}

void C3DObjectTexture::SetImage(FIBITMAP* image)
{
	Clear();

	this->image = image;
	if (image) {
		sizeX = FreeImage_GetWidth((FIBITMAP*)image);
		sizeY = FreeImage_GetHeight((FIBITMAP*)image);
		if (useBytes) {
			InitBytes();
		}
	}
	ready = false;
	needsReload = 1;
}

void C3DObjectTexture::SetImage(float* data, int size)
{
	dim = 1;
	floatData = data;
	sizeX = size;
	sizeY = 1;
	clearAfter = false;
	ready = false;
}

void C3DObjectTexture::Bind(unsigned int programId) const
{
	switch (dim)
	{
	case 1:
		break;
	case 2:
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureId);

		int iSamplerLoc = glGetUniformLocation(programId, textureUniformName);
		glUniform1i(iSamplerLoc, 0);
		break;
	}
}

void C3DObjectTexture::UnloadImage()
{
	FreeImage_Unload(image);
	image = nullptr;
}
