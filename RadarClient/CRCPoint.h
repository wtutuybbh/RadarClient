#pragma once
#include <fstream>
#include <iostream>
#include <windows.h>

#include <GL/GL.h>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/polar_coordinates.hpp"
#include <vector>
#include "Util.h"
#include "CCamera.h"
#include "C3DObject.h"

#define _USE_MATH_DEFINES 
#include <math.h>

#define CRCPOINT_CONSTRUCTOR_USES_RADIANS

#define POINT_SIZE 5.0f

class CScene;

class CRCPoint : C3DObject
{
private:
	static void CreateSphereR1(int level);
	static std::vector<VBOData> VBOBuffer_s;
	static int VBOBufferSize_s;
	static unsigned int VBOName_s, VAOName_s;
	static GLuint ProgramID_s;
	static GLuint MVPUniformLoc_s;
	static GLuint NormUniformLoc_s;


	glm::mat4 model, mvp;
	glm::mat3 norm;

public:
	glm::vec3 CartesianCoords, SphericalCoords;
	float r;

	CRCPoint(float y0, float mpph, float mppv, float r, float a, float e);
	~CRCPoint();
	static void LoadShaders();
	static void UseProgram();
	static void PrepareVBO();
	static void BuildVBO();
	void Draw(CCamera *cam);

	CVec * GetBounds();
};

