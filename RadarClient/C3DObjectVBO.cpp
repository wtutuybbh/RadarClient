#include "stdafx.h"
#include "C3DObjectVBO.h"
#include "CRCLogger.h"

const std::string C3DObjectVBO::requestID = "C3DObjectVBO";

C3DObjectVBO::~C3DObjectVBO()
{
	if (vbuffer) {
		delete vbuffer;
	}
	if (idxArrays) {		
		delete idxArrays;	
	}
	if (idxIds) {
		delete idxIds;
	}
	if (idxLengths) {
		delete idxLengths;
	}
	if (idxModes) {
		delete idxModes;
	}
}

C3DObjectVBO::C3DObjectVBO(bool clearAfter)
{
	this->clearAfter = clearAfter;
}

C3DObjectVBO* C3DObjectVBO::InitStructure()
{
	auto b = new std::vector<VBOData>;

	//default structure (2 triangles)
	b->push_back({ glm::vec4(-1, -1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });
	b->push_back({ glm::vec4(-1, 1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 1) });
	b->push_back({ glm::vec4(1, 1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(1, 1) });

	b->push_back({ glm::vec4(1, 1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(1, 1) });
	b->push_back({ glm::vec4(1, -1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(1, 0) });
	b->push_back({ glm::vec4(-1, -1, 0, 1), glm::vec3(0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec2(0, 0) });

	vbuffer = b;

	return this;
}


void C3DObjectVBO::SetVBuffer(std::vector<VBOData> *vbuffer)
{
	if (vbuffer) {
		this->vbuffer = vbuffer;
	}
}

void C3DObjectVBO::Bind() const
{
	glBindVertexArray(vaoId);
}

void C3DObjectVBO::LoadToGPU()
{
	if (vbuffer && vbuffer->size()>0 && !this->ready) {
		glGenVertexArrays(1, &vaoId);
		glBindVertexArray(vaoId);

		glGenBuffers(1, &vboId);
		glBindBuffer(GL_ARRAY_BUFFER, vboId);
		glBufferData(GL_ARRAY_BUFFER, vbuffer->size() * sizeof(VBOData), &(*vbuffer)[0], GL_STATIC_DRAW);
		vbufferSize = vbuffer->size();
		
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
		if (this->clearAfter)
		{
			vbuffer->clear();
			vbuffer->shrink_to_fit();
		}
	}	
}

void C3DObjectVBO::Reload()
{
	if (vbuffer == nullptr || vbuffer->size() == 0)
	{
		return;
	}
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, vbuffer->size() * elementSize, &(*vbuffer)[0], GL_STATIC_DRAW);
	vbufferSize = vbuffer->size();

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
	return vbuffer != nullptr && vbuffer->size() > 0;
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
		int id;
		for (int i = 0; i < idxArrays->size(); i++)
		{
			mode = idxModes->at(i);
			count = idxLengths->at(i);
			id = idxIds->at(i);
			
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

std::vector<VBOData>* C3DObjectVBO::GetVBuffer() const
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
	
	if (vbuffer)
	{
		std::vector<VBOData> *newvbuffer = new std::vector<VBOData>(vbuffer->size());
		vbo_->SetVBuffer(newvbuffer);
	}

	
	if (idxArrays)
	{
		for (int i = 0; i < idxArrays->size(); i++)
		{
			vbo_->AddIndexArray(idxArrays->at(i), idxLengths->at(i), idxModes->at(i));
		}
	}
	return vbo_;
}
