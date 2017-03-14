#pragma once
#include "stdafx.h"

class C3DObjectVertices;


struct VBOData {
	glm::vec4 vert;
	glm::vec3 norm;
	glm::vec4 color;
	glm::vec2 texcoord;
};

class C3DObjectVBO
{
	static const std::string requestID;

	std::vector<VBOData> *vbuffer{ nullptr };
	unsigned int vboId { 0 }, vaoId { 0 }, vbufferSize {0};
	bool ready{ false }, clearAfter;
	int elementSize{ sizeof(VBOData) };

	std::vector<unsigned short *> *idxArrays { nullptr };
	std::vector<int> *idxLengths { nullptr };
	std::vector<GLenum> *idxModes { nullptr };
	std::vector<unsigned int> *idxIds{ nullptr };

	

public:
	std::shared_ptr<C3DObjectVertices> vertices{ std::shared_ptr<C3DObjectVertices>(nullptr) };

	~C3DObjectVBO();
	bool NeedsReload { false };

	explicit C3DObjectVBO(bool clearAfter);
	virtual C3DObjectVBO* InitStructure();
	virtual void SetVBuffer(std::vector<VBOData> *vbuffer);
	void Bind() const;
	void LoadToGPU();
	void Reload();
	bool Ready() const;
	bool HasBuffer() const;
	static void UnBind();
	void Draw(GLenum mode) const;

	std::vector<VBOData>* GetVBuffer() const;

	virtual void AddIndexArray(unsigned short *idxArray, int length, GLenum mode);
	virtual void ClearIndexArray();

	virtual C3DObjectVBO* Clone();
};
