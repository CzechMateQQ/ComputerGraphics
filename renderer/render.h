#pragma once

#include <vector>			// vector

#include "glew/GL/glew.h"	// glew (GLuint, etc.)
#include "glm/glm.hpp"		// glm math types (vec4)

// Define vertext structure
struct vertex
{
	glm::vec4 pos;
	glm::vec4 color;
	glm::vec2 uv;
	glm::vec4 normal; 
};

// An object to represent mesh
struct geometry
{
	GLuint vao, vbo, ibo;	// buffers
	GLuint size;			// index count
};

// An object to represent shader
struct shader
{
	GLuint program;
};

struct texture
{
	GLuint handle;
	unsigned width, height, channels;	// unsigned = unsigned int
};

struct light
{
	glm::vec3 direction;	// should be normalized
	glm::vec3 color;
};

// Functions to make and unmake above types
geometry loadGeometry(const char* filePath);
geometry makeGeometry(vertex* verts, size_t vertCount,
	unsigned int* indices, size_t indxCount);
void freeGeometry(geometry& geo);

texture loadTexture(const char* filePath);
texture makeTexture(unsigned width, unsigned height, unsigned channels, const unsigned char* pixels);
void freeTexture(texture& tex);

shader makeShader(const char* vertSource, const char* fragSource);
shader loadShader(const char* vertPath, const char* fragPath);
void freeShader(shader& shad);
bool checkShader(GLuint target, const char* humanReadableName = "");

void draw(const shader& shad, const geometry& geo);

void setUniform(const shader& shad, GLuint location, const glm::mat4& value);
void setUniform(const shader& shad, GLuint location, const texture& tex, int textureSlot);
void setUniform(const shader& shad, GLuint location, float value);
void setUniform(const shader& shad, GLuint location, const glm::vec3& value);