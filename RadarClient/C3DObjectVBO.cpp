#include "stdafx.h"
#include "C3DObjectVBO.h"

C3DObjectVBO::~C3DObjectVBO()
{
	if (idxArrays) {		
		idxArrays->clear();
		delete idxArrays;	
	}
	if (idxIds) {
		idxIds->clear();
		delete idxIds;
	}
	if (idxLengths) {
		idxLengths->clear();
		delete idxLengths;
	}
	if (idxModes) {
		idxModes->clear();
		delete idxModes;
	}
}

C3DObjectVBO::C3DObjectVBO(bool clearAfter)
{
	this->buffer = NULL;
	this->clearAfter = clearAfter;
	this->ready = false;
	this->bufferSize = 0;
	this->elementSize = sizeof(VBOData);
}

C3DObjectVBO* C3DObjectVBO::InitStructure()
{
	auto b = new std::vector<VBOData>;
	//default structure
	b->push_back({ glm::vec4(-1, -1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	b->push_back({ glm::vec4(-1, 1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 1) });
	b->push_back({ glm::vec4(1, 1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(1, 1) });

	b->push_back({ glm::vec4(1, 1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(1, 1) });
	b->push_back({ glm::vec4(1, -1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(1, 0) });
	b->push_back({ glm::vec4(-1, -1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });

	buffer = b;

	return this;
}

void C3DObjectVBO::SetBuffer(void *vbuffer, void* buffer, int size)
{
	this->vbuffer = vbuffer;
	this->buffer = buffer;
	this->bufferSize = size;
}

void C3DObjectVBO::Bind() const
{
	glBindVertexArray(vaoId);
}

void C3DObjectVBO::LoadToGPU()
{
	if (!this->ready) {
		glGenVertexArrays(1, &vaoId);
		glBindVertexArray(vaoId);

		glGenBuffers(1, &vboId);
		glBindBuffer(GL_ARRAY_BUFFER, vboId);
		glBufferData(GL_ARRAY_BUFFER, bufferSize * sizeof(VBOData), buffer, GL_STATIC_DRAW);
		//bufferSize = buffer->size();


		if (idxArrays)
		{
			for (int i = 0; i < idxArrays->size(); i++)
			{
				unsigned int id;
				glGenBuffers(1, &id);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxLengths->at(i) * sizeof(unsigned short), idxArrays->at(i), GL_STATIC_DRAW);
				(*idxIds)[i] =id;
			}
		}
		this->ready = true;
	}	
}

void C3DObjectVBO::Reload()
{
	if (buffer == NULL || bufferSize == 0)
	{
		return;
	}
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, bufferSize * elementSize, buffer, GL_STATIC_DRAW);
	
	if (idxArrays)
	{
		for (int i = 0; i < idxArrays->size(); i++)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*idxIds)[i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxLengths->at(i) * sizeof(unsigned short), idxArrays->at(i), GL_STATIC_DRAW);
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
	return buffer != NULL && bufferSize > 0;
}

void C3DObjectVBO::UnBind()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void C3DObjectVBO::Draw(GLenum mode) const
{
	
	if (idxArrays)
	{
		GLenum mode;
		GLsizei count;
		//const void * indices;
		int id;
		for (int i = 0; i < idxArrays->size(); i++)
		{
			mode = idxModes->at(i);
			count = idxLengths->at(i);
			//indices = idxArrays->at(i);
			id = idxIds->at(i);
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
			glDrawElements(mode, count, GL_UNSIGNED_SHORT, (void*)0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			//glDrawElements(mode, count, GL_UNSIGNED_SHORT, indices);
		}
	}
	else
	{		
		glDrawArrays(mode, 0, bufferSize);
	}
}

void* C3DObjectVBO::GetBuffer() const
{
	return vbuffer;
}

void C3DObjectVBO::AddIndexArray(unsigned short* idxArray, int length, GLenum mode)
{
	if (!idxArrays)
	{
		idxArrays = new std::vector<unsigned short *>;
		idxLengths = new std::vector<int>;
		idxModes = new std::vector<GLenum>;
		idxIds = new std::vector<unsigned int>;
	}

	idxArrays->push_back(idxArray);
	idxLengths->push_back(length);
	idxModes->push_back(mode);
	idxIds->push_back(0);
}

void C3DObjectVBO::ClearIndexArray()
{
	if (idxArrays) {
		for (auto it = begin(*idxArrays); it != end(*idxArrays); ++it)
			delete *it;
	}
}

C3DObjectVBO* C3DObjectVBO::Clone()
{
	C3DObjectVBO* vbo_ = new C3DObjectVBO(clearAfter);
	vbo_->SetBuffer(vbuffer, buffer, bufferSize);
	if (idxArrays)
	{
		for (int i = 0; i < idxArrays->size(); i++)
		{
			vbo_->AddIndexArray(idxArrays->at(i), idxLengths->at(i), idxModes->at(i));
		}
	}
	return vbo_;
}
