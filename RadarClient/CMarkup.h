#pragma once
#include "C3DObjectModel.h"
class CMarkup :
	public C3DObjectModel
{
	bool show_labels {true};
public:
	CMarkup(glm::vec4 origin);
	void ShowLabels(bool show_labels);
	void BindUniforms(CViewPortControl* vpControl) override;
	void Draw(CViewPortControl* vpControl, GLenum mode) override;
};

