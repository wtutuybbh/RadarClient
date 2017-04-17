#include "stdafx.h"
#include "CMarkup.h"
#include "Util.h"
#include "CSettings.h"
#include "CViewPortControl.h"
#include "C3DObjectVBO.h"
#include "C3DObjectProgram.h"


CMarkup::CMarkup(glm::vec4 origin)
{
	c3DObjectModel_TypeName = "CMarkup";

	vbo.insert_or_assign(Main, new C3DObjectVBO(false));
	tex.insert_or_assign(Main, nullptr);
	prog.insert_or_assign(Main, new C3DObjectProgram("CMarkup.v.glsl", "CMarkup.f.glsl", "vertex", nullptr, "norm", "color"));
	translateMatrix.insert_or_assign(Main, glm::mat4(1.0f));
	scaleMatrix.insert_or_assign(Main, glm::mat4(1.0f));
	rotateMatrix.insert_or_assign(Main, glm::mat4(1.0f));

	vbo.insert_or_assign(MiniMap, new C3DObjectVBO(false));
	tex.insert_or_assign(MiniMap, nullptr);
	prog.insert_or_assign(MiniMap, new C3DObjectProgram("CMarkup.v.glsl", "CMarkup.f.glsl", "vertex", nullptr, "norm", "color"));
	translateMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
	scaleMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
	rotateMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));

	int markDistance = CSettings::GetInt(IntMarkupMarkDistance);
	float mpph = CSettings::GetFloat(FloatMPPh);
	float mppv = CSettings::GetFloat(FloatMPPv);
	int numCircles = CSettings::GetInt(IntMarkupNumCircles);
	int marksPerCircle = CSettings::GetInt(IntMarkupMarksPerCircle);
	int segmentsPerCircle = CSettings::GetInt(IntMarkupSegmentsPerCircle);
	int vertexCount_Axis = CSettings::GetInt(IntMarkupHorizontalAxisCount)*2 + 2;

	int markCount = numCircles * marksPerCircle;
	int markSize = CSettings::GetInt(IntMarkupMarkSize);

	float YAxisLength = markCount * markDistance / mppv;
	float maxDist = CSettings::GetFloat(FloatMaxDistance);

	markCount = markCount * 5;

	int vertexCount = vertexCount_Axis + markCount * 2 + (numCircles + 2) * segmentsPerCircle;

	vertices = std::make_shared<C3DObjectVertices>(vertexCount);

	Color = CSettings::GetColor(ColorAxis);

	glm::vec4 colorBlankZones = CSettings::GetColor(ColorBlankZones);
	
	int i0 = 0;
	//vertical axis

	glm::vec3 n(0, 0, 0);
	glm::vec3 n1(0, 0, 1);
	glm::vec2 t(0, 0);

	vertices.get()->SetValues(i0, origin, n, Color, t);

	vertices.get()->SetValues(i0+1, origin + glm::vec4(0, YAxisLength, 0, 0), n, Color, t);

	i0 += 2;
	//horizontal axis
	for (int a = 0; a < 180; a += 360 / CSettings::GetInt(IntMarkupHorizontalAxisCount) / 2) {
		vertices.get()->SetValues(i0, origin + glm::vec4(maxDist * sin(cnvrt::dg2rad(a)) / mpph, 0, maxDist * cos(cnvrt::dg2rad(a)) / mpph, 0), n, Color, t);
		vertices.get()->SetValues(i0 + 1, origin + glm::vec4(-maxDist * sin(cnvrt::dg2rad(a)) / mpph, 0, -maxDist * cos(cnvrt::dg2rad(a)) / mpph, 0), n, Color, t);
		i0 += 2;
	}

	//marks:
	for (int i = 0; i < markCount / 5; i += 1) {
		vertices.get()->SetValues(i0, origin + glm::vec4((i + 1) * markDistance / mpph, 0, -markSize / 2.0, 0), n, Color, t);
		vertices.get()->SetValues(i0 + 1, origin + glm::vec4((i + 1) * markDistance / mpph, 0, markSize / 2.0, 0), n, Color, t);
		i0 += 2;
	}
	for (int i = 0; i < markCount / 5; i += 1) {
		vertices.get()->SetValues(i0, origin + glm::vec4(-(i + 1) * markDistance / mpph, 0, -markSize / 2.0, 0), n, Color, t);
		vertices.get()->SetValues(i0 + 1, origin + glm::vec4(-(i + 1) * markDistance / mpph, 0, markSize / 2.0, 0), n, Color, t);
		i0 += 2;
	}
	for (int i = 0; i < markCount / 5; i += 1) {
		vertices.get()->SetValues(i0, origin + glm::vec4(-markSize / 2.0, 0, (i + 1) * markDistance / mpph, 0), n, Color, t);
		vertices.get()->SetValues(i0 + 1, origin + glm::vec4(markSize / 2.0, 0, (i + 1) * markDistance / mpph, 0), n, Color, t);
		i0 += 2;
	}
	for (int i = 0; i < markCount / 5; i += 1) {
		vertices.get()->SetValues(i0, origin + glm::vec4(-markSize / 2.0, 0, -(i + 1) * markDistance / mpph, 0), n, Color, t);
		vertices.get()->SetValues(i0 + 1, origin + glm::vec4(markSize / 2.0, 0, -(i + 1) * markDistance / mpph, 0), n, Color, t);
		i0 += 2;
	}
	for (int i = 0; i < markCount / 5; i += 1) {
		vertices.get()->SetValues(i0, origin + glm::vec4(-markSize / 2.0, (i + 1) * markDistance / mppv, 0, 0), n, Color, t);
		vertices.get()->SetValues(i0 + 1, origin + glm::vec4(markSize / 2.0, (i + 1) * markDistance / mppv, 0, 0), n, Color, t);
		i0 += 2;
	}
	//circles:
	float R = 0;
	for (int c = 0; c < numCircles; c++) {
		R += markDistance * marksPerCircle;
		for (int i = 0; i < segmentsPerCircle; i++) {
			vertices.get()->SetValues(i0, origin + glm::vec4(R * cos(2 * M_PI * i / segmentsPerCircle) / mpph, 0, R * sin(2 * M_PI * i / segmentsPerCircle) / mpph, 0), n, Color, t);
			i0++;
		}
	}
	//blank zone R1:
	R = CSettings::GetFloat(FloatBlankR1);
	for (int i = 0; i < segmentsPerCircle; i++) {
		vertices.get()->SetValues(i0, origin + glm::vec4(R * cos(2 * M_PI * i / segmentsPerCircle) / mpph, 0, R * sin(2 * M_PI * i / segmentsPerCircle) / mpph, 0), n1, colorBlankZones, t);
		i0++;
	}
	//blank zone R2:
	R = CSettings::GetFloat(FloatBlankR2);
	for (int i = 0; i < segmentsPerCircle; i++) {
		vertices.get()->SetValues(i0, origin + glm::vec4(R * cos(2 * M_PI * i / segmentsPerCircle) / mpph, 0, R * sin(2 * M_PI * i / segmentsPerCircle) / mpph, 0), n1, colorBlankZones, t);
		i0++;
	}


	

	int lineMarkupCount = vertexCount_Axis + markCount * 2;
	unsigned short * lineMarkup = vertices.get()->AddIndexArray(lineMarkupCount, GL_LINES);
	for (int i = 0; i < lineMarkupCount; i++) {
		lineMarkup[i] = i;
	}

	unsigned short *circle;
	for (int c = 0; c < numCircles+2; c++) {
		circle = vertices.get()->AddIndexArray(segmentsPerCircle, GL_LINE_LOOP);
		for (int i = 0; i < segmentsPerCircle; i++) {
			circle[i] = lineMarkupCount + segmentsPerCircle*c + i;
		}
	}
	
	if (!vbo.at(Main)->vertices)
		vbo.at(Main)->vertices = vertices;
	if (!vbo.at(MiniMap)->vertices)
		vbo.at(MiniMap)->vertices = vertices;
	vertices.get()->usesCount = 2;

	/*std::ofstream outfile("new.txt", std::ofstream::binary);
	for (int i = 0; i < buffer->size(); i++) {
		outfile << (*buffer)[i].vert.x << ";" << (*buffer)[i].vert.y << ";" << (*buffer)[i].vert.z << "\r\n";
	}
	outfile.close();*/

}

void CMarkup::ShowLabels(bool show_labels)
{
	this->show_labels = show_labels;
}

void CMarkup::BindUniforms(CViewPortControl* vpControl)
{
	glm::mat4 m = GetModelMatrix(vpControl->Id);
	glm::mat4 v = vpControl->GetViewMatrix();
	glm::mat4 p = vpControl->GetProjMatrix();
	glm::mat4 mvp = p*v*m;
	int mvp_loc = prog.at(vpControl->Id)->GetUniformLocation("mvp");
	glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, glm::value_ptr(mvp));

	int unicolor2_loc = prog.at(vpControl->Id)->GetUniformLocation("unicolor2");
	glUniform4fv(unicolor2_loc, 1, glm::value_ptr(CSettings::GetColor(ColorBlankZones)));
	
	int unicolor_loc = prog.at(vpControl->Id)->GetUniformLocation("unicolor");
	glUniform4fv(unicolor_loc, 1, glm::value_ptr(CSettings::GetColor(ColorAxis)));

	int uniweight_loc = prog.at(vpControl->Id)->GetUniformLocation("uniweight");
	glUniform1f(uniweight_loc, uniweight);
}

void CMarkup::Draw(CViewPortControl* vpControl, GLenum mode)
{
	C3DObjectModel::Draw(vpControl, mode);
	if (show_labels)
	{
		DrawBitmaps();
	}
}

void CMarkup::DrawBitmaps() const
{
	float y_0 = 0;// Mesh->GetCenterHeight() / MPPv;
	glm::vec2 position = glm::vec2(CSettings::GetFloat(FloatPositionLon), CSettings::GetFloat(FloatPositionLat));
	float MPPh = CSettings::GetFloat(FloatMPPh);
	float MPPv = CSettings::GetFloat(FloatMPPv);
	float markDistance = CSettings::GetFloat(FloatMarkDistance);
	float maxDist = CSettings::GetFloat(FloatMaxDist);

	BitmapString(0, y_0, 0, "(" + cnvrt::float2str(position.x) + "; " + cnvrt::float2str(position.y) + ")");
	auto color = CSettings::GetColor(ColorNumbers);
	glColor4f(color.r, color.g, color.b, color.a);
	BitmapString(-10 * markDistance / MPPh, y_0 + 1, 0, "1km");
	BitmapString(10 * markDistance / MPPh, y_0 + 1, 0, "1km");
	BitmapString(0, y_0 + 1, -10 * markDistance / MPPh, "1km");
	BitmapString(0, y_0 + 1, 10 * markDistance / MPPh, "1km");
	BitmapString(0, y_0 + 10 * markDistance / MPPv, 0, "1km");

	BitmapString(-20 * markDistance / MPPh, y_0 + 1, 0, "2km");
	BitmapString(20 * markDistance / MPPh, y_0 + 1, 0, "2km");
	BitmapString(0, y_0 + 1, -20 * markDistance / MPPh, "2km");
	BitmapString(0, y_0 + 1, 20 * markDistance / MPPh, "2km");
	BitmapString(0, y_0 + 20 * markDistance / MPPv, 0, "2km");

	BitmapString(-30 * markDistance / MPPh, y_0 + 1, 0, "3km");
	BitmapString(30 * markDistance / MPPh, y_0 + 1, 0, "3km");
	BitmapString(0, y_0 + 1, -30 * markDistance / MPPh, "3km");
	BitmapString(0, y_0 + 1, 30 * markDistance / MPPh, "3km");
	BitmapString(0, y_0 + 30 * markDistance / MPPv, 0, "3km");

	BitmapString(-40 * markDistance / MPPh, y_0 + 1, 0, "4km");
	BitmapString(40 * markDistance / MPPh, y_0 + 1, 0, "4km");
	BitmapString(0, y_0 + 1, -40 * markDistance / MPPh, "4km");
	BitmapString(0, y_0 + 1, 40 * markDistance / MPPh, "4km");
	BitmapString(0, y_0 + 40 * markDistance / MPPv, 0, "4km");

	BitmapString(-50 * markDistance / MPPh, y_0 + 1, 0, "5km");
	BitmapString(50 * markDistance / MPPh, y_0 + 1, 0, "5km");
	BitmapString(0, y_0 + 1, -50 * markDistance / MPPh, "5km");
	BitmapString(0, y_0 + 1, 50 * markDistance / MPPh, "5km");
	BitmapString(0, y_0 + 50 * markDistance / MPPv, 0, "5km");

	for (int a = 0; a < 360; a += 10) {
		BitmapString(-(maxDist + 50 * MPPh) * sin(cnvrt::dg2rad(a)) / MPPh, y_0, (maxDist + 50 * MPPh) * cos(cnvrt::dg2rad(a)) / MPPh, cnvrt::float2str(a) + "°");
	}

	glColor3f(0.8f, 0.8f, 1.0f);
	BitmapString(0, y_0 + markDistance, 60 * markDistance / MPPh, "N");
	glColor3f(1.0f, 0.8f, 0.8f);
	BitmapString(0, y_0 + markDistance, -60 * markDistance / MPPh, "S");
	glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
	BitmapString(60 * markDistance / MPPh, y_0 + markDistance, 0, "W");
	BitmapString(-60 * markDistance / MPPh, y_0 + markDistance, 0, "E");
}