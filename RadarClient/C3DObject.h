#pragma once

#include <vector>
#include "Util.h"
#include "ShaderUtils.h"
#include "FreeImage.h"
#include <unordered_map>

class C3DObject;

typedef void (C3DObject::*PtrToMethod)();

#define CALL_MEMBER_FN(object, ptrToMember)  ((object).*(ptrToMember))



typedef std::unordered_map<int, PtrToMethod> PointersMap;


class CVert;
typedef CVert CVec;
class C3DObject
{
protected:
	std::vector<VBOData> VBOBuffer;
	int VBOBufferSize;
	unsigned int VBOName, VAOName;
	GLuint ProgramID;
	bool VBOReady;

	GLuint MVPUniformLoc;
	GLuint NormUniformLoc;


	std::vector<VBOData> MiniMapVBOBuffer;
	int MiniMapVBOBufferSize;
	unsigned int MiniMapVBOName, MiniMapVAOName;
	GLuint MiniMapProgramID;
	bool MiniMapVBOReady;
	unsigned int	MiniMapTextureId;

	FIBITMAP *MiniMapImage;

	char *ImgFile, *VShaderFile, *FShaderFile;

	PointersMap map;
public:
	C3DObject();
	C3DObject(bool initMap);
	~C3DObject();
	virtual CVec * GetBounds();

	bool PrepareAndBuildMinimapVBO(const char * vShaderFile, const char * fShaderFile, const char * imgFile);

	void BuildMinimapVBO();
	void PrepareMinimapVBO();
	void MiniMapAttribBind();
	void BindTextureImage();
	void UnbindAll();
	void CreateProgram();


	void DrawMiniMap();
};

