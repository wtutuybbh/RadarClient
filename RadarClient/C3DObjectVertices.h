#include "stdafx.h"

class C3DObjectVertices
{
public:
	float *vbuffer {nullptr};
	std::vector<unsigned short *> idxArrays;
	std::vector<unsigned short> idxLengths;
	std::vector<GLenum> idxModes;
	std::vector<unsigned int> idxIds;
	unsigned short vertexSize{ 13 };	
	unsigned int vertexCount{ 0 };
	bool needsReload{ false };

	explicit C3DObjectVertices(int vertexCount)
	{
		if (vertexCount <= 0)
			throw std::exception("vertexCount must be greater than 0");

		this->vertexCount = vertexCount;
		vbuffer = new float[vertexCount * this->vertexSize];		
	}
	C3DObjectVertices(int vertexCount, int vertexSize)
	{
		if (vertexCount <= 0)
			throw std::exception("vertexCount must be greater than 0");
		if (vertexSize <= 0)
			throw std::exception("vertexSize must be greater than 0");

		this->vertexCount = vertexCount;
		this->vertexSize = vertexSize;
		vbuffer = new float[vertexCount * vertexSize];
	}
	~C3DObjectVertices()
	{
		if (vbuffer)
			delete[] vbuffer;
		if (idxArrays.size()>0)
		{
			for (auto it=idxArrays.begin(); it != idxArrays.end(); ++it)
			{
				if (*it)
					delete[] *it;
			}
		}
	}
	void SetValues(int offset, float vx, float vy, float vz, float vt, float nx, float ny, float nz, float cr, float cg, float cb, float ca, float tu, float tv) const;
	void SetValues(int offset, glm::vec4 v, glm::vec3 n, glm::vec4 c, glm::vec2 t) const;

	void AddIndexArray(int length, GLenum mode);
	void SetIndex(int a, int i, int v);
};


