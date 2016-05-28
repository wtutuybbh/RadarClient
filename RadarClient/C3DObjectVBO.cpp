#include "C3DObjectVBO.h"

C3DObjectVBO::~C3DObjectVBO()
{
	if (buffer) {
		buffer->clear();
		delete buffer;
	}
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
}

C3DObjectVBO* C3DObjectVBO::InitStructure()
{
	buffer = new std::vector<VBOData>;
	//default structure
	buffer->push_back({ glm::vec4(-1, -1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	buffer->push_back({ glm::vec4(-1, 1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 1) });
	buffer->push_back({ glm::vec4(1, 1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(1, 1) });

	buffer->push_back({ glm::vec4(1, 1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(1, 1) });
	buffer->push_back({ glm::vec4(1, -1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(1, 0) });
	buffer->push_back({ glm::vec4(-1, -1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });

	return this;
}

void C3DObjectVBO::SetBuffer(std::vector<VBOData>* buffer)
{
	this->buffer = buffer;
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
		glBufferData(GL_ARRAY_BUFFER, buffer->size() * sizeof(VBOData), &(*buffer)[0], GL_STATIC_DRAW);
		bufferSize = buffer->size();

		if (clearAfter) {
			//glFinish();			
			buffer->clear(); //destroy all vbo buffer objects
			std::vector<VBOData>().swap(*buffer); //free memory used by vector itself
		}

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
	if (buffer == NULL || buffer->size() == 0)
	{
		return;
	}
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, buffer->size() * sizeof(VBOData), &(*buffer)[0], GL_STATIC_DRAW);
	bufferSize = buffer->size();
	if (clearAfter) {
		buffer->clear(); //destroy all vbo buffer objects
		std::vector<VBOData>().swap(*buffer); //free memory used by vector itself
	}
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
	return buffer != NULL && buffer->size() > 0;
}

void C3DObjectVBO::UnBind()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void C3DObjectVBO::Draw(GLenum mode) const
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

std::vector<VBOData>* C3DObjectVBO::GetBuffer() const
{
	return buffer;
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

C3DObjectVBO* C3DObjectVBO::Clone()
{
	C3DObjectVBO* vbo_ = new C3DObjectVBO(clearAfter);
	vbo_->SetBuffer(buffer);
	if (idxArrays)
	{
		for (int i = 0; i < idxArrays->size(); i++)
		{
			vbo_->AddIndexArray(idxArrays->at(i), idxLengths->at(i), idxModes->at(i));
		}
	}
	return vbo_;
}
