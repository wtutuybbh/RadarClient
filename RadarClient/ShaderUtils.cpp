#include "stdafx.h"
#include "CRCLogger.h"
#include "Util.h"
/**
* Store all the file's contents in memory, useful to pass shaders
* source code to OpenGL
*/
char* file_read(const char* filename)
{
	FILE* in = fopen(filename, "rb");
	if (in == nullptr) return nullptr;

	int res_size = BUFSIZ;
	char* res = (char*)malloc(res_size);
	int nb_read_total = 0;

	while (!feof(in) && !ferror(in)) {
		if (nb_read_total + BUFSIZ > res_size) {
			if (res_size > 10 * 1024 * 1024) break;
			res_size = res_size * 2;
			res = (char*)realloc(res, res_size);
		}
		char* p_res = res + nb_read_total;
		nb_read_total += fread(p_res, 1, BUFSIZ, in);
	}

	fclose(in);
	res = (char*)realloc(res, nb_read_total + 1);
	res[nb_read_total] = '\0';
	return res;
}

/**
* Display compilation errors from the OpenGL shader compiler
*/
void print_log(GLuint object)
{
	GLint log_length = 0;
	if (glIsShader(object))
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
	else if (glIsProgram(object))
		glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
	else {
		LOG_ERROR("ShaderUtils", "printlog", "Not a shader or a program");
		return;
	}

	char* log = (char*)malloc(log_length);

	if (glIsShader(object))
		glGetShaderInfoLog(object, log_length, nullptr, log);
	else if (glIsProgram(object))
		glGetProgramInfoLog(object, log_length, nullptr, log);

	LOG_ERROR("ShaderUtils", "printlog", "%s", log);
	free(log);
}

/**
* Compile the shader from file 'filename', with error handling
*/
GLuint create_shader(const char* filename, GLenum type)
{
	const GLchar* source = file_read(filename);
	if (source == nullptr) {
		LOG_ERROR("ShaderUtils", "create_shader", "Error opening %s: ", filename);
		return 0;
	}
	GLuint res = glCreateShader(type);
	const GLchar* sources[] = {
		// Define GLSL version
#ifdef GL_ES_VERSION_2_0
		"#version 100\n"  // OpenGL ES 2.0
#else
		"#version 120\n"  // OpenGL 2.1
#endif
		,
		// GLES2 precision specifiers
#ifdef GL_ES_VERSION_2_0
		// Define default float precision for fragment shaders:
		(type == GL_FRAGMENT_SHADER) ?
		"#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
		"precision highp float;           \n"
		"#else                            \n"
		"precision mediump float;         \n"
		"#endif                           \n"
		: ""
		// Note: OpenGL ES automatically defines this:
		// #define GL_ES
#else
		// Ignore GLES 2 precision specifiers:
		"#define lowp   \n"
		"#define mediump\n"
		"#define highp  \n"
#endif
		,
		source };
	glShaderSource(res, 3, sources, nullptr);
	free((void*)source);

	glCompileShader(res);
	GLint compile_ok = GL_FALSE;
	glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);
	if (compile_ok == GL_FALSE) {
		LOG_ERROR("ShaderUtils", "create_shader", "%s:", filename);
		print_log(res);
		glDeleteShader(res);
		return 0;
	}

	return res;
}

GLuint create_program(const char *vertexfile, const char *fragmentfile) {
	GLuint program = glCreateProgram();
	GLuint shader;

	if (vertexfile) {
		shader = create_shader(vertexfile, GL_VERTEX_SHADER);
		if (!shader)
			return 0;
		glAttachShader(program, shader);
	}

	if (fragmentfile) {
		shader = create_shader(fragmentfile, GL_FRAGMENT_SHADER);
		if (!shader)
			return 0;
		glAttachShader(program, shader);
	}

	glLinkProgram(program);
	GLint link_ok = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	if (!link_ok) {
		LOG_ERROR("ShaderUtils", "create_program", "(vertexfile=%s, fragmentfile=%s) glLinkProgram:", vertexfile, fragmentfile);
		print_log(program);
		glDeleteProgram(program);
		return 0;
	}

	return program;
}

#ifdef GL_GEOMETRY_SHADER
GLuint create_gs_program(const char *vertexfile, const char *geometryfile, const char *fragmentfile, GLint input, GLint output, GLint vertices) {
	GLuint program = glCreateProgram();
	GLuint shader;

	if (vertexfile) {
		shader = create_shader(vertexfile, GL_VERTEX_SHADER);
		if (!shader)
			return 0;
		glAttachShader(program, shader);
	}

	if (geometryfile) {
		shader = create_shader(geometryfile, GL_GEOMETRY_SHADER);
		if (!shader)
			return 0;
		glAttachShader(program, shader);

		glProgramParameteriEXT(program, GL_GEOMETRY_INPUT_TYPE_EXT, input);
		glProgramParameteriEXT(program, GL_GEOMETRY_OUTPUT_TYPE_EXT, output);
		glProgramParameteriEXT(program, GL_GEOMETRY_VERTICES_OUT_EXT, vertices);
	}

	if (fragmentfile) {
		shader = create_shader(fragmentfile, GL_FRAGMENT_SHADER);
		if (!shader)
			return 0;
		glAttachShader(program, shader);
	}

	glLinkProgram(program);
	GLint link_ok = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	if (!link_ok) {
		LOG_ERROR("ShaderUtils", "create_gs_program", "(vertexfile=%s, geometryfile=%s, fragmentfile=%s) glLinkProgram:", vertexfile, geometryfile, fragmentfile);
		print_log(program);
		glDeleteProgram(program);
		return 0;
	}

	return program;
}
#else
GLuint create_gs_program(const char *vertexfile, const char *geometryfile, const char *fragmentfile, GLint input, GLint output, GLint vertices) {
	LOG_ERROR("ShaderUtils", "create_gs_program", "Missing support for geometry shaders");
	return 0;
}
#endif

GLint get_attrib(GLuint program, const char *name) {
	GLint attribute = glGetAttribLocation(program, name);
	if (attribute == -1)
		LOG_ERROR("ShaderUtils", "get_attrib", "program=%d, Could not bind attribute %s", program, name);
	return attribute;
}

GLint get_uniform(GLuint program, const char *name) {
	GLint uniform = glGetUniformLocation(program, name);
	if (uniform == -1)
		LOG_ERROR("ShaderUtils", "get_uniform", "program=%d, Could not bind uniform %s", program, name);
	return uniform;
}
