#pragma once
#include "stdafx.h"


struct VBOData {
	glm::vec4 vert;
	glm::vec3 norm;
	glm::vec4 color;
	glm::vec2 texcoord;
};

class C3DObjectVBO
{
	//std::vector<VBOData>* buffer {NULL};
	void * buffer{ nullptr }, *vbuffer{ nullptr };
	unsigned int vboId { 0 }, vaoId { 0 };
	bool ready, clearAfter;
	int bufferSize, elementSize{ 0 };

	std::vector<unsigned short *> *idxArrays { nullptr };
	std::vector<int> *idxLengths { nullptr };
	std::vector<GLenum> *idxModes { nullptr };
	std::vector<unsigned int> *idxIds{ nullptr };
public:
	virtual ~C3DObjectVBO();
	bool NeedsReload {false};

	C3DObjectVBO(bool clearAfter);
	virtual C3DObjectVBO* InitStructure();
	//virtual void SetBuffer(std::vector<VBOData>* buffer);
	virtual void SetBuffer(void *vbuffer, void* buffer, int size);
	void Bind() const;
	void LoadToGPU();
	void Reload();
	bool Ready() const;
	bool HasBuffer() const;
	static void UnBind();
	void Draw(GLenum mode) const;

	//std::vector<VBOData>* GetBuffer() const;
	void* GetBuffer() const;

	virtual void AddIndexArray(unsigned short *idxArray, int length, GLenum mode);
	virtual void ClearIndexArray();

	virtual C3DObjectVBO* Clone();
};
