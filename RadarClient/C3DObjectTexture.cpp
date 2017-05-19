#include "stdafx.h"
#include "C3DObjectTexture.h"
#include "Util.h"
#include "CRCLogger.h"

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
	switch (dim)
	{
	case 1:
		if (!ready && floatData)
		{
			if (textureId > 0)
			{
				glDeleteTextures(1, &textureId);
			}
			glGenTextures(1, &textureId);
			glBindTexture(GL_TEXTURE_1D, textureId);

			glTexImage1D(GL_TEXTURE_1D, 0, internalFormat, sizeX, 0, pdformat, type, floatData);

			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			if (clearAfter)
			{
				
			}
		}
		break;
	case 2:
		if (!ready && (useBits && bits || !useBits && image)) {
			if (textureId > 0)
			{
				glDeleteTextures(1, &textureId);
			}
			glGenTextures(1, &textureId);
			glBindTexture(GL_TEXTURE_2D, textureId);

			if (!useBits && image && FreeImage_GetBPP(image) != 32)
			{
				FIBITMAP* tempImage = image;
				image = FreeImage_ConvertTo32Bits(tempImage);
			}

			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, sizeX, sizeY, 0, pdformat, type, useBits ? bits : (void*)FreeImage_GetBits(image));

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			ready = true;
			if (useBits && bits)
			{
				delete[] bits;
				bits = nullptr;
			}
			if (clearAfter && image) {
				FreeImage_Unload(image);
				image = nullptr;
			}
		}
		break;
		default:
			break;
	}
}

void C3DObjectTexture::UnBind() const
{
	switch (dim)
	{
	case 1:
		glBindTexture(GL_TEXTURE_1D, 0);
		break;
	case 2:
		glBindTexture(GL_TEXTURE_2D, 0);
		break;
	default:
		break;
	}
	
}

C3DObjectTexture* C3DObjectTexture::Clone()
{
	if (imgFile.length()>0)
		return new C3DObjectTexture(imgFile.c_str(), textureUniformName);

	return new C3DObjectTexture(image, textureUniformName, clearAfter, useBits);
}

void C3DObjectTexture::SetDim(int dim)
{
	this->dim = dim;
}

void C3DObjectTexture::SetFormatsAndType(GLint internalFormat, GLenum pdformat, GLenum type)
{
	this->internalFormat = internalFormat;
	this->pdformat = pdformat;
	this->type = type;
}

C3DObjectTexture::~C3DObjectTexture()
{
	if (bits)
		delete[] bits;
	if (image)
		FreeImage_Unload(image);
}

C3DObjectTexture::C3DObjectTexture()
{
}

C3DObjectTexture::C3DObjectTexture(const char* textureUniformName)
{
	this->textureUniformName = (char *)textureUniformName;
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
	useBits = true;
	clearAfter = false;
	InitBits();

	FreeImage_Unload(image);
	image = nullptr;

	this->textureUniformName = (char *)textureUniformName;

	ready = false;
}

C3DObjectTexture::C3DObjectTexture(FIBITMAP* image, const char* textureUniformName, bool clearAfter, bool useBits)
{
	this->clearAfter = clearAfter;
	this->useBits = useBits;	
	this->textureUniformName = (char *)textureUniformName;

	Reload(image);
}

void C3DObjectTexture::Reload(FIBITMAP* image)
{
	this->image = image;
	sizeX = FreeImage_GetWidth((FIBITMAP*)image);
	sizeY = FreeImage_GetHeight((FIBITMAP*)image);
	if (useBits) {
		InitBits();
	}
	ready = false;
}

void C3DObjectTexture::Reload(float* data, int sizeX, int sizeY)
{
	floatData = data;
	sizeX = sizeX;
	sizeY = sizeY;
	clearAfter = false;
	ready = false;
}

void C3DObjectTexture::Bind(unsigned int programId) const
{
	int iSamplerLoc;
	switch (dim)
	{
	case 1:
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_1D, textureId);
		iSamplerLoc = glGetUniformLocation(programId, textureUniformName);
		glUniform1i(iSamplerLoc, 0);
		break;
	case 2:
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureId);
		iSamplerLoc = glGetUniformLocation(programId, textureUniformName);
		glUniform1i(iSamplerLoc, 0);
		break;
	default:
		break;
	}
}

void C3DObjectTexture::UnloadImage()
{
	FreeImage_Unload(image);
	image = nullptr;
}
