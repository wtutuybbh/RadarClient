#include "stdafx.h"
#include "CPath.h"
#include "CSettings.h"
#include "CMinimap.h"
#include "C3DObjectVBO.h"

CPath::CPath(): CTrack(0, false)
{
}

void CPath::AddPoint(glm::vec4 point)
{
	if (!vertices)
		vertices = std::make_shared<C3DObjectVertices>(1);
	else 
		vertices.get()->ReCreate(vertices.get()->vertexCount + 1, true);

	Color = CSettings::GetColor(ColorMeasureLine);

	vertices.get()->SetValues(vertices.get()->vertexCount - 1, point.x, point.y, point.z, point.w, 0, 0, 0, Color.r, Color.g, Color.b, Color.a, 0, 0);

	unsigned short * arr[2];
	if (vertices.get()->GetIndexArrayCount() == 0)
	{
		if(arr[0] = vertices.get()->AddIndexArray(vertices.get()->vertexCount, GL_POINTS))
			arr[0][vertices.get()->vertexCount - 1] = 0;
		if(arr[1] = vertices.get()->AddIndexArray(vertices.get()->vertexCount, GL_LINE_STRIP))
			arr[1][vertices.get()->vertexCount - 1] = 0;
	}
	else
	{
		int length;
		if(arr[0] = vertices.get()->GetIndexArray(0, length))
			arr[0][vertices.get()->vertexCount - 1] = vertices.get()->vertexCount - 1;


		
		if (arr[1] = vertices.get()->GetIndexArray(1, length))
			arr[1][vertices.get()->vertexCount - 1] = vertices.get()->vertexCount - 1;
	}
	if (!vbo.at(Main)->vertices)
		vbo.at(Main)->vertices = vertices;
	if (!vbo.at(MiniMap)->vertices)
		vbo.at(MiniMap)->vertices = vertices;

	vertices.get()->needsReload = 2;
}

void CPath::BindUniforms(CViewPortControl* vpControl)
{
	uniColor = CSettings::GetColor(ColorMeasureLineInvisible);
	CSector::BindUniforms(vpControl);
}
