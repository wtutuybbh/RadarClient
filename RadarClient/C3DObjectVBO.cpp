#include "stdafx.h"
#include "C3DObjectVBO.h"
#include "CRCLogger.h"
#include "C3DObjectVertices.h"

const std::string C3DObjectVBO::requestID = "C3DObjectVBO";

C3DObjectVBO::~C3DObjectVBO()
{
}

C3DObjectVBO::C3DObjectVBO(bool clearAfter)
{
	this->clearAfter = clearAfter;
}


void C3DObjectVBO::Bind() const
{
	glBindVertexArray(vaoId);
}

void C3DObjectVBO::LoadToGPU()
{
	//if (vbuffer && vbuffer->size()>0 && !this->ready) {
	if (vertices && vertices.get()->vertexCount>0 && !this->ready) {
		glGenVertexArrays(1, &vaoId);
		glBindVertexArray(vaoId);

		glGenBuffers(1, &vboId);
		glBindBuffer(GL_ARRAY_BUFFER, vboId);
		//glBufferData(GL_ARRAY_BUFFER, vbuffer->size() * sizeof(VBOData), &(*vbuffer)[0], GL_STATIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, vertices.get()->vertexCount * vertices.get()->vertexSize * sizeof(float), vertices.get()->vbuffer, GL_STATIC_DRAW);
		vbufferSize = vertices.get()->vertexCount;
		
		if (vertices.get()->idxArrays.size()>0)
		{
			for (int i = 0; i < vertices.get()->idxArrays.size(); i++)
			{
				unsigned int id;
				glGenBuffers(1, &id);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertices.get()->idxLengths.at(i) * sizeof(unsigned short), vertices.get()->idxArrays.at(i), GL_STATIC_DRAW);
				idxIds.push_back(id);
			}
		}
		this->ready = true;
		if (this->clearAfter)
		{
			vertices.get()->usesCount--;
			if (vertices.get()->usesCount == 0)
			{
				delete[] vertices.get()->vbuffer;
				vertices.get()->vbuffer = nullptr;
				vertices.get()->vertexCount = 0;
			}
		}
		
	}	
}

void C3DObjectVBO::Reload()
{
	//if (vbuffer == nullptr || vbuffer->size() == 0)
	if (!vertices || vertices.get()->vertexCount==0)
	{
		return;
	}
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, vertices.get()->vertexCount * vertices.get()->vertexSize * sizeof(float), vertices.get()->vbuffer, GL_STATIC_DRAW);
	vbufferSize = vertices.get()->vertexCount;

	if (vertices.get()->idxArrays.size()>0)
	{
		for (int i = 0; i < idxIds.size(); i++)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxIds[i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertices.get()->idxLengths.at(i) * sizeof(unsigned short), vertices.get()->idxArrays.at(i), GL_STATIC_DRAW);
		}
	}
	NeedsReload = false;
}

bool C3DObjectVBO::Ready() const
{
	return ready;
}

bool C3DObjectVBO::HasBuffer() const
{
	//return vbuffer != nullptr && vbuffer->size() > 0; // TODO! 
	return vertices && vertices.get()->vertexCount > 0 && vertices.get()->vbuffer;
}

void C3DObjectVBO::UnBind()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void C3DObjectVBO::Draw(GLenum mode) const
{
	
	if (vertices.get()->idxArrays.size()>0)
	{
		//GLenum mode;
		GLsizei count;
		int id;
		for (int i = 0; i < vertices.get()->idxArrays.size(); i++)
		{
			mode = vertices.get()->idxModes.at(i);
			count = vertices.get()->idxLengths.at(i);
			id = idxIds.at(i);
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
			glDrawElements(mode, count, GL_UNSIGNED_SHORT, (void*)nullptr);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
	}
	else
	{				
		glDrawArrays(mode, 0, vbufferSize);
	}
}
