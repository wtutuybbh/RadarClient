#include "CMarkup.h"
#include "CSettings.h"


CMarkup::CMarkup(glm::vec4 origin) : C3DObjectModel (Main,
	new C3DObjectVBO(false),
	NULL,
	new C3DObjectProgram("CMarkup.v.glsl", "CMarkup.f.glsl", "vertex", NULL, NULL, "color"))
{

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

	vector<VBOData> *buffer = new vector<VBOData>(vertexCount);

	Color = CSettings::GetColor(ColorAxis);

	glm::vec4 colorBlankZones = CSettings::GetColor(ColorBlankZones);
	
	for (int i = 0; i < vertexCount; i++)
	{
		(*buffer)[i].color = Color;
	}

	int i0 = 0;
	//vertical axis

	(*buffer)[i0].vert = origin;
	(*buffer)[i0 + 1].vert = origin + glm::vec4(0, YAxisLength, 0, 0);

	i0 += 2;
	//horizontal axis
	for (int a = 0; a < 180; a += 360 / CSettings::GetInt(IntMarkupHorizontalAxisCount) / 2) {
		(*buffer)[i0].vert = origin + glm::vec4(maxDist * sin(cnvrt::dg2rad(a)) / mpph, 0, maxDist * cos(cnvrt::dg2rad(a)) / mpph, 0);
		(*buffer)[i0 + 1].vert = origin + glm::vec4(-maxDist * sin(cnvrt::dg2rad(a)) / mpph, 0, -maxDist * cos(cnvrt::dg2rad(a)) / mpph, 0);
		i0 += 2;
	}

	//marks:
	for (int i = 0; i < markCount / 5; i += 1) {
		(*buffer)[i0].vert = origin + glm::vec4((i + 1) * markDistance / mpph, 0, -markSize / 2.0, 0);
		(*buffer)[i0 + 1].vert = origin + glm::vec4((i + 1) * markDistance / mpph, 0, markSize / 2.0, 0);
		i0 += 2;
	}
	for (int i = 0; i < markCount / 5; i += 1) {
		(*buffer)[i0].vert = origin + glm::vec4(-(i + 1) * markDistance / mpph, 0, -markSize / 2.0, 0);
		(*buffer)[i0 + 1].vert = origin + glm::vec4(-(i + 1) * markDistance / mpph, 0, markSize / 2.0, 0);
		i0 += 2;
	}
	for (int i = 0; i < markCount / 5; i += 1) {
		(*buffer)[i0].vert = origin + glm::vec4(-markSize / 2.0, 0, (i + 1) * markDistance / mpph, 0);
		(*buffer)[i0 + 1].vert = origin + glm::vec4(markSize / 2.0, 0, (i + 1) * markDistance / mpph, 0);
		i0 += 2;
	}
	for (int i = 0; i < markCount / 5; i += 1) {
		(*buffer)[i0].vert = origin + glm::vec4(-markSize / 2.0, 0, -(i + 1) * markDistance / mpph, 0);
		(*buffer)[i0 + 1].vert = origin + glm::vec4(markSize / 2.0, 0, -(i + 1) * markDistance / mpph, 0);
		i0 += 2;
	}
	for (int i = 0; i < markCount / 5; i += 1) {
		(*buffer)[i0].vert = origin + glm::vec4(-markSize / 2.0, (i + 1) * markDistance / mppv, 0, 0);
		(*buffer)[i0 + 1].vert = origin + glm::vec4(markSize / 2.0, (i + 1) * markDistance / mppv, 0, 0);
		i0 += 2;
	}
	//circles:
	float R = 0;
	for (int c = 0; c < numCircles; c++) {
		R += markDistance * marksPerCircle;
		for (int i = 0; i < segmentsPerCircle; i++) {
			(*buffer)[i0].vert = origin + glm::vec4(R * cos(2 * M_PI * i / segmentsPerCircle) / mpph, 0, R * sin(2 * M_PI * i / segmentsPerCircle) / mpph, 0);
			i0++;
		}
	}
	//blank zone R1:
	R = CSettings::GetFloat(FloatBlankR1);
	for (int i = 0; i < segmentsPerCircle; i++) {
		(*buffer)[i0].vert = origin + glm::vec4(R * cos(2 * M_PI * i / segmentsPerCircle) / mpph, 0, R * sin(2 * M_PI * i / segmentsPerCircle) / mpph, 0);
		(*buffer)[i0].color = colorBlankZones;
		i0++;
	}
	//blank zone R2:
	R = CSettings::GetFloat(FloatBlankR2);
	for (int i = 0; i < segmentsPerCircle; i++) {
		(*buffer)[i0].vert = origin + glm::vec4(R * cos(2 * M_PI * i / segmentsPerCircle) / mpph, 0, R * sin(2 * M_PI * i / segmentsPerCircle) / mpph, 0);
		(*buffer)[i0].color = colorBlankZones;
		i0++;
	}


	C3DObjectVBO *mmvbo = new C3DObjectVBO(false);

	vbo.at(Main)->SetBuffer(buffer, &(*buffer)[0], buffer->size());
	mmvbo->SetBuffer(buffer, &(*buffer)[0], buffer->size());

	int lineMarkupCount = vertexCount_Axis + markCount * 2;
	unsigned short * lineMarkup = new unsigned short[lineMarkupCount];
	for (int i = 0; i < lineMarkupCount; i++) {
		lineMarkup[i] = i;
	}
	vbo.at(Main)->AddIndexArray(lineMarkup, lineMarkupCount, GL_LINES);
	mmvbo->AddIndexArray(lineMarkup, lineMarkupCount, GL_LINES);

	unsigned short *circle;
	for (int c = 0; c < numCircles+2; c++) {
		circle = new unsigned short[segmentsPerCircle];
		for (int i = 0; i < segmentsPerCircle; i++) {
			circle[i] = lineMarkupCount + segmentsPerCircle*c + i;
		}
		vbo.at(Main)->AddIndexArray(circle, segmentsPerCircle, GL_LINE_LOOP);
		mmvbo->AddIndexArray(circle, segmentsPerCircle, GL_LINE_LOOP);
	}

	vbo.insert_or_assign(MiniMap, mmvbo);

	prog.insert_or_assign(MiniMap, new C3DObjectProgram("CMarkup.v.glsl", "CMarkup.f.glsl", "vertex", NULL, NULL, "color"));

	tex.insert_or_assign(MiniMap, nullptr);


	scaleMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
	rotateMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));
	translateMatrix.insert_or_assign(MiniMap, glm::mat4(1.0f));

	

	/*std::ofstream outfile("new.txt", std::ofstream::binary);
	for (int i = 0; i < buffer->size(); i++) {
		outfile << (*buffer)[i].vert.x << ";" << (*buffer)[i].vert.y << ";" << (*buffer)[i].vert.z << "\r\n";
	}
	outfile.close();*/

}


CMarkup::~CMarkup()
{
	if (id == _testid)
	{
		int dummy;
		dummy = 1 + 1;
	}
	auto buffer = (vector<VBOData>*)vbo.at(Main)->GetBuffer();
	if (buffer)
		delete buffer;
	vbo.at(Main)->ClearIndexArray();
}

void CMarkup::BindUniforms(CViewPortControl* vpControl)
{
	glm::mat4 m = GetModelMatrix(vpControl);
	glm::mat4 v = vpControl->GetViewMatrix();
	glm::mat4 p = vpControl->GetProjMatrix();
	glm::mat4 mvp = p*v*m;
	int mvp_loc = prog.at(vpControl->Id)->GetUniformLocation("mvp");
	glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, glm::value_ptr(mvp));

	int color_loc = prog.at(vpControl->Id)->GetUniformLocation("color");
	glUniform4fv(color_loc, 1, glm::value_ptr(Color));
}
