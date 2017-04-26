#include "stdafx.h"
#include "C3DObjectVertices.h"

void C3DObjectVertices::Clear()
{
	if (vbuffer)
		delete[] vbuffer;
	ClearIdxArrays();
}

void C3DObjectVertices::ClearIdxArrays()
{
	if (idxArrays.size()>0)
	{
		for (auto it = idxArrays.begin(); it != idxArrays.end(); ++it)
		{
			if (*it)
				delete[] * it;
		}
	}
}

void C3DObjectVertices::TestVertexCount(unsigned vertexCount)
{
	if (vertexCount > USHRT_MAX)
		throw std::exception("vertexCount must be less than USHRT_MAX (65535)");
	if (vertexCount <= 0)
		throw std::exception("vertexCount must be greater than 0");
}

C3DObjectVertices::C3DObjectVertices(int vertexCount)
{
	TestVertexCount(vertexCount);

	this->vertexCount = vertexCount;
	vbuffer = new float[vertexCount * this->vertexSize];		
}

C3DObjectVertices::C3DObjectVertices(int vertexCount, int vertexSize)
{
	TestVertexCount(vertexCount);

	this->vertexCount = vertexCount;
	this->vertexSize = vertexSize;
	vbuffer = new float[vertexCount * vertexSize];
}

C3DObjectVertices::~C3DObjectVertices()
{
	Clear();
}

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
		throw std::exception(("wrong function call of wrong vertexSize=" + std::to_string(vertexSize) + " (must be 13)").c_str());
	int o = offset * vertexSize;
	if (vbuffer && o + 12 < vertexCount * vertexSize)
	{
		vbuffer[o] = v.x;
		vbuffer[o + 1] = v.y;
		vbuffer[o + 2] = v.z;
		vbuffer[o + 3] = v.w;
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

void C3DObjectVertices::SetValues(int offset, glm::vec4 v, glm::vec3 n, glm::vec4 c, glm::vec4 c2, glm::vec2 t) const
{
	if (vertexSize != 17)
		throw std::exception(("wrong function call of wrong vertexSize=" + std::to_string(vertexSize) + " (must be 17)").c_str());
	int o = offset * vertexSize;
	if (vbuffer && o + 16 < vertexCount * vertexSize)
	{
		vbuffer[o] = v.x;
		vbuffer[o + 1] = v.y;
		vbuffer[o + 2] = v.z;
		vbuffer[o + 3] = v.w;
		vbuffer[o + 4] = n.x;
		vbuffer[o + 5] = n.y;
		vbuffer[o + 6] = n.z;
		vbuffer[o + 7] = c.r;
		vbuffer[o + 8] = c.g;
		vbuffer[o + 9] = c.b;
		vbuffer[o + 10] = c.a;
		vbuffer[o + 11] = c2.r;
		vbuffer[o + 12] = c2.g;
		vbuffer[o + 13] = c2.b;
		vbuffer[o + 14] = c2.a;
		vbuffer[o + 15] = t.x;
		vbuffer[o + 16] = t.y;
	}
}

unsigned short * C3DObjectVertices::AddIndexArray(int length, GLenum mode)
{
	auto arr = new unsigned short[length];
	idxArrays.push_back(arr);
	idxLengths.push_back(length);
	idxModes.push_back(mode);
	idxIds.push_back(0);
	return arr;
}

unsigned short* C3DObjectVertices::GetIndexArray(int i, int &length)
{
	if (i<idxArrays.size())
	{
		length = idxLengths[i];
		return idxArrays[i];
	}
	return nullptr;
}

void C3DObjectVertices::SetArrayLength(int i, int length)
{
	idxLengths[i] = length;
}

int C3DObjectVertices::GetIndexArrayCount() const
{
	return idxArrays.size();
}

void C3DObjectVertices::SetIndex(int a, int i, int v)
{
	idxArrays[a][i] = v;
}

float* C3DObjectVertices::GetBuffer() const
{
	return vbuffer;
}

glm::vec4 *C3DObjectVertices::getv(int offset)
{	
	return reinterpret_cast<glm::vec4*>(vbuffer + offset * vertexSize);
}

void C3DObjectVertices::Translate(glm::vec3 vshift, int place)
{
	if (!vbuffer)
		return;
	for (auto i =0;i<vertexCount;i++)
	{
		vbuffer[i * vertexSize + place] += vshift.x;
		vbuffer[i * vertexSize + place + 1] += vshift.y;
		vbuffer[i * vertexSize + place + 2] += vshift.z;
	}
	needsReload = true;
}

void C3DObjectVertices::ReCreate(int vertexCount)
{
	TestVertexCount(vertexCount);

	this->vertexCount = vertexCount;
		
	Clear();

	vbuffer = new float[vertexCount * this->vertexSize];

		
}

void C3DObjectVertices::ReCreate(int vertexCount, bool preserve)
{
	if (!preserve)
	{
		ReCreate(vertexCount);
	}
	else
	{
		TestVertexCount(vertexCount);

		if (vbuffer) {
			auto vbuffer_tmp = vbuffer;
			vbuffer = new float[vertexCount * this->vertexSize];
			memcpy(vbuffer, vbuffer_tmp, this->vertexCount * vertexSize * sizeof(float));			
			delete[] vbuffer_tmp;
		}
		else
		{
			vbuffer = new float[vertexCount * this->vertexSize];
		}

		auto dvc = vertexCount - this->vertexCount;
		for (auto i = 0; i < idxArrays.size(); i++)
		{
			if (idxArrays[i])
			{
				auto array_tmp = idxArrays[i];
				idxArrays[i] = new unsigned short[idxLengths[i] + dvc];
				memcpy(idxArrays[i], array_tmp, sizeof(unsigned short) * (dvc > 0 ? idxLengths[i] : idxLengths[i] + dvc));
				idxLengths[i] += dvc;
				delete[] array_tmp;
			}
		}
		
		this->vertexCount = vertexCount;
	}
}

boost::property_tree::ptree C3DObjectVertices::GetPropertyTree(int i_from, int i_to) const
{
	boost::property_tree::ptree ret;
	ret.put("vbuffer", vbuffer);
	ret.put("vertexCount", vertexCount);
	ret.put("vertexSize", vertexSize);
	if (vbuffer && vertexCount > 0)
	{
		boost::property_tree::ptree vbuffer_ptree;
		for(auto i= i_from; i<vertexCount && i<=i_to; i++)
		{
			boost::property_tree::ptree node;
			for (auto j=0; j<vertexSize; j++)
			{
				node.put(std::to_string(j), vbuffer[i*vertexSize + j]);
			}
			vbuffer_ptree.push_back(std::make_pair(std::to_string(i), node));
		}
		ret.add_child("vbuffer_content", vbuffer_ptree);
	}
	return ret;
}
