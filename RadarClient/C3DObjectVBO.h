#pragma once
#include "stdafx.h"

class C3DObjectVertices;


class C3DObjectVBO
{
	static const std::string requestID;

	unsigned int vboId { 0 }, vaoId { 0 }, vbufferSize {0};
	bool ready{ false }, clearAfter;

	std::vector<unsigned int> idxIds;

	

public:
	std::shared_ptr<C3DObjectVertices> vertices{ std::shared_ptr<C3DObjectVertices>(nullptr) };

	~C3DObjectVBO();


	explicit C3DObjectVBO(bool clearAfter);
	void Bind() const;
	void LoadToGPU();
	void Reload();
	bool Ready() const;
	bool HasBuffer() const;
	static void UnBind();
	void Draw(GLenum mode) const;
};
