#pragma once
#include <GL/glew.h>
#include "Util.h"

class C3DObjectVBO
{
	//std::vector<VBOData>* buffer {NULL};
	void * buffer{ NULL }, *vbuffer{ NULL };
	unsigned int vboId { 0 }, vaoId { 0 };
	bool ready, clearAfter;
	int bufferSize, elementSize{ sizeof(VBOData) };

	std::vector<unsigned short *> *idxArrays {NULL};
	std::vector<int> *idxLengths {NULL};
	std::vector<GLenum> *idxModes {NULL};
	std::vector<unsigned int> *idxIds{ NULL };
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
