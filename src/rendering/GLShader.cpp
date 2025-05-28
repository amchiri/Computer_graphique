//#include "stdafx.h"
#include "GLShader.h"
//#define GLEW_STATIC
#include "GL/glew.h"

#include <fstream>
#include <iostream>
#include <vector>

bool ValidateShader(GLuint shader)
{
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

	if (!compiled)
	{
		GLint infoLen = 0;

		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

		if (infoLen > 1)
		{
			char* infoLog = new char[1 + infoLen];

			glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
			std::cout << "Error compiling shader:" << infoLog << std::endl;

			delete[] infoLog;
		}

		// on supprime le shader object car il est inutilisable
		glDeleteShader(shader);

		return false;
	}

	return true;
}

bool GLShader::LoadVertexShader(const char* filename)
{
	// 1. Charger le fichier en memoire
	std::ifstream fin(filename, std::ios::in | std::ios::binary);
	fin.seekg(0, std::ios::end);
	uint32_t length = (uint32_t)fin.tellg();
	fin.seekg(0, std::ios::beg);
	char* buffer = nullptr;
	buffer = new char[length + 1];
	buffer[length] = '\0';
	fin.read(buffer, length);

	// 2. Creer le shader object
	m_VertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(m_VertexShader, 1, &buffer, nullptr);
	// 3. Le compiler
	glCompileShader(m_VertexShader);
	// 4. Nettoyer
	delete[] buffer;
	fin.close();	// non obligatoire ici

	// 5. 
	// verifie le status de la compilation
	return ValidateShader(m_VertexShader);
}

bool GLShader::LoadGeometryShader(const char* filename)
{
	// 1. Charger le fichier en memoire
	std::ifstream fin(filename, std::ios::in | std::ios::binary);
	fin.seekg(0, std::ios::end);
	uint32_t length = (uint32_t)fin.tellg();
	fin.seekg(0, std::ios::beg);
	char* buffer = nullptr;
	buffer = new char[length + 1];
	buffer[length] = '\0';
	fin.read(buffer, length);

	// 2. Creer le shader object
	m_GeometryShader = glCreateShader(GL_GEOMETRY_SHADER);
	glShaderSource(m_GeometryShader, 1, &buffer, nullptr);
	// 3. Le compiler
	glCompileShader(m_GeometryShader);
	// 4. Nettoyer
	delete[] buffer;
	fin.close();	// non obligatoire ici

	// 5. 
	// verifie le status de la compilation
	return ValidateShader(m_GeometryShader);
}

bool GLShader::LoadFragmentShader(const char* filename)
{
	std::ifstream fin(filename, std::ios::in | std::ios::binary);
	fin.seekg(0, std::ios::end);
	uint32_t length = (uint32_t)fin.tellg();
	fin.seekg(0, std::ios::beg);
	char* buffer = nullptr;
	buffer = new char[length + 1];
	buffer[length] = '\0';
	fin.read(buffer, length);

	// 2. Creer le shader object
	m_FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(m_FragmentShader, 1, &buffer, nullptr);
	// 3. Le compiler
	glCompileShader(m_FragmentShader);
	// 4. Nettoyer
	delete[] buffer;
	fin.close();	// non obligatoire ici

	// 5. 
	// verifie le status de la compilation
	return ValidateShader(m_FragmentShader);
}

bool GLShader::Create()
{
	m_Program = glCreateProgram();
	glAttachShader(m_Program, m_VertexShader);
	if (m_GeometryShader)
		glAttachShader(m_Program, m_GeometryShader);
	glAttachShader(m_Program, m_FragmentShader);
	glLinkProgram(m_Program);

	int32_t linked = 0;
	int32_t infoLen = 0;
	// verification du statut du linkage
	glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);

	if (!linked)
	{
		glGetProgramiv(m_Program, GL_INFO_LOG_LENGTH, &infoLen);

		if (infoLen > 1)
		{
			std::vector<char> infoLog(infoLen + 1);
			glGetProgramInfoLog(m_Program, infoLen, nullptr, infoLog.data());
			std::cerr << "Program linking failed:\n" << infoLog.data() << std::endl;
		}

		glDeleteProgram(m_Program);

		return false;
	}

	return true;
}

void GLShader::Destroy()
{
	glDetachShader(m_Program, m_VertexShader);
	glDetachShader(m_Program, m_FragmentShader);
	glDetachShader(m_Program, m_GeometryShader);
	glDeleteShader(m_GeometryShader);
	glDeleteShader(m_VertexShader);
	glDeleteShader(m_FragmentShader);
	glDeleteProgram(m_Program);
}

void GLShader::SetBool(const char* name, bool value) {
    GLint location = GetUniformLocation(name);
    if (location >= 0) {
        glUniform1i(location, (int)value);
    }
}

void GLShader::SetInt(const char* name, int value) {
    GLint location = GetUniformLocation(name);
    if (location >= 0) {
        glUniform1i(location, value);
    }
}

void GLShader::SetFloat(const char* name, float value) {
    GLint location = GetUniformLocation(name);
    if (location >= 0) {
        glUniform1f(location, value);
    }
}

void GLShader::SetVec3(const char* name, const float* value) {
    GLint location = GetUniformLocation(name);
    if (location >= 0) {
        glUniform3fv(location, 1, value);
    }
}

void GLShader::SetVec4(const char* name, const float* value) {
    GLint location = GetUniformLocation(name);
    if (location >= 0) {
        glUniform4fv(location, 1, value);
    }
}

void GLShader::SetMat4(const char* name, const float* value) {
    GLint location = GetUniformLocation(name);
    if (location >= 0) {
        glUniformMatrix4fv(location, 1, GL_FALSE, value);
    }
}

GLint GLShader::GetUniformLocation(const char* name) const {
    return glGetUniformLocation(m_Program, name);
}

