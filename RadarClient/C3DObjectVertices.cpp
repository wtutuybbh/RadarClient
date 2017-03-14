#include "stdafx.h"
#include "C3DObjectVertices.h"

void C3DObjectVertices::SetValues(int offset, float vx, float vy, float vz, float vt, float nx, float ny, float nz, float cr, float cg, float cb, float ca, float tu, float tv) const
{
	if (vertexSize != 13)
		throw std::exception(("wrong function call of wrong vertexSize=" + std::to_string(vertexSize)).c_str());
	int o = offset * vertexSize;
	if (vbuffer && o + 12 < vertexCount * vertexSize)
	{
		vbuffer[o] = vx;
		vbuffer[o + 1] = vy;
		vbuffer[o + 2] = vz;
		vbuffer[o + 3] = vt;
		vbuffer[o + 4] = nx;
		vbuffer[o + 5] = ny;
		vbuffer[o + 6] = nz;
		vbuffer[o + 7] = cr;
		vbuffer[o + 8] = cg;
		vbuffer[o + 9] = cb;
		vbuffer[o + 10] = ca;
		vbuffer[o + 11] = tu;
		vbuffer[o + 12] = tv;
	}
}

void C3DObjectVertices::SetValues(int offset, glm::vec4 v, glm::vec3 n, glm::vec4 c, glm::vec2 t) const
{
	if (vertexSize != 13)
		throw std::exception(("wrong function call of wrong vertexSize=" + std::to_string(vertexSize)).c_str());
	int o = offset * vertexSize;
	if (vbuffer && o + 12 < vertexCount * vertexSize)
	{
		vbuffer[o] = v.x;
		vbuffer[o + 1] = v.y;
		vbuffer[o + 2] = v.z;
		vbuffer[o + 3] = v.t;
		vbuffer[o + 4] = n.x;
		vbuffer[o + 5] = n.y;
		vbuffer[o + 6] = n.z;
		vbuffer[o + 7] = c.r;
		vbuffer[o + 8] = c.g;
		vbuffer[o + 9] = c.b;
		vbuffer[o + 10] = c.a;
		vbuffer[o + 11] = t.x;
		vbuffer[o + 12] = t.y;
	}
}

void C3DObjectVertices::AddIndexArray(int length, GLenum mode)
{
	idxArrays.push_back(new unsigned short[length]);
	idxLengths.push_back(length);
	idxModes.push_back(mode);
}

void C3DObjectVertices::SetIndex(int a, int i, int v)
{
	idxArrays[a][i] = v;
}
