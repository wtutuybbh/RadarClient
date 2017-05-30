#include "stdafx.h"

class C3DObjectVertices
{
	void Clear();
	void ClearIdxArrays();
	static void TestVertexCount(unsigned int vertexCount);
public:
	float *vbuffer {nullptr};
	std::vector<unsigned short *> idxArrays;
	std::vector<unsigned short> idxLengths;
	std::vector<GLenum> idxModes;
	std::vector<unsigned int> idxIds;
	unsigned short vertexSize{ 13 };	
	unsigned int vertexCount{ 0 };
	int needsReload{ 0 };
	int usesCount{ 0 };

	explicit C3DObjectVertices(int vertexCount);

	C3DObjectVertices(int vertexCount, int vertexSize);

	~C3DObjectVertices();
	void SetValues(int offset, float vx, float vy, float vz, float vt, float nx, float ny, float nz, float cr, float cg, float cb, float ca, float tu, float tv) const;
	void SetValues(int offset, glm::vec4 v, glm::vec3 n, glm::vec4 c, glm::vec2 t) const;
	void SetValues(int offset, glm::vec4 v, glm::vec3 n, glm::vec4 c, glm::vec4 c2, glm::vec2 t) const;

	unsigned short * AddIndexArray(int length, GLenum mode);
	unsigned short * GetIndexArray(int i, int &length);
	void SetArrayLength(int i, int length);
	int GetIndexArrayCount() const;

	void SetIndex(int a, int i, int v);

	float * GetBuffer() const;
	glm::vec4 *getv(int offset);	
	void Translate(glm::vec3 vshift, int place);
	void ReCreate(int vertexCount);

	void ReCreate(int vertexCount, bool preserve);
	boost::property_tree::ptree GetPropertyTree(int i_from, int i_to) const;
};


