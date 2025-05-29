#pragma once

#include <GL/glew.h>
#include <cstdint>

class GLShader
{
private:
	// un programme fait le liens entre Vertex Shader et Fragment Shader
	uint32_t m_Program;
	// Un Vertex Shader est execute pour chaque sommet (vertex)
	uint32_t m_VertexShader;
	// Un Geometry Shader est execute pour chaque primitive
	uint32_t m_GeometryShader;
	// Un Fragment Shader est execute pour chaque "pixel"
	// lors de la rasterization/remplissage de la primitive
	uint32_t m_FragmentShader;

	bool CompileShader(uint32_t type);
public:
	GLShader() : m_Program(0), m_VertexShader(0),
		m_GeometryShader(0), m_FragmentShader(0) {

	}
	~GLShader() {}

	inline uint32_t GetProgram() { return m_Program; }

	bool LoadVertexShader(const char* filename);
	bool LoadGeometryShader(const char* filename);
	bool LoadFragmentShader(const char* filename);
	bool Create();
	void Destroy();

	void Use() const { glUseProgram(m_Program); }

	// Ajout des méthodes pour gérer les uniformes
	void SetBool(const char* name, bool value);
	void SetInt(const char* name, int value);
	void SetFloat(const char* name, float value);
	void SetVec3(const char* name, const float* value);
	void SetVec4(const char* name, const float* value);
	void SetMat4(const char* name, const float* value);

	// Méthode pour récupérer la location d'un uniform
	GLint GetUniformLocation(const char* name) const;
};