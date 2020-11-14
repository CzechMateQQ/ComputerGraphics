#include "context.h"
#include "render.h"

#include "glm/ext.hpp"

#include <limits>

int main()
{
	context game;
	game.init(512, 512, "Jesse Engine");

	// Vertices
	vertex triVerts[] =
	{
		{ { -.5f, -.5f, 0, 1 }, { 0.0f, 0.0f, 1.0f, 1.0f }, {0,   0}, { 0.0f, 0.0f, -1.0f, 0.0f } }, // bottom-left
		{ {  .5f, -.5f, 0, 1 }, { 0.0f, 1.0f, 0.0f, 1.0f }, {1,   0}, { 0.0f, 0.0f, -1.0f, 0.0f } }, // bottom-right
		{ {   0.f, .5f, 0, 1 }, { 1.0f, 0.0f, 0.0f, 1.0f }, {0.5, 1}, { 0.0f, 0.0f, -1.0f, 0.0f } }  // top-center
	};

	vertex quadVerts[] =
	{
		{ { -1.f, -1.f, 0, 1 }, { 1.0f, 1.0f, 1.0f, 1.0f }, {0, 0}, { 0.0f, 0.0f, -1.0f, 0.0f } }, // bottom-left
		{ {   1.f,-1.f, 0, 1 }, { 1.0f, 1.0f, 1.0f, 1.0f }, {1, 0}, { 0.0f, 0.0f, -1.0f, 0.0f } }, // bottom-right
		{ {  -1.f, 1.f, 0, 1 }, { 1.0f, 1.0f, 1.0f, 1.0f }, {0, 1}, { 0.0f, 0.0f, -1.0f, 0.0f } }, // top-left
		{ {   1.f, 1.f, 0, 1 }, { 1.0f, 1.0f, 1.0f, 1.0f }, {1, 1}, { 0.0f, 0.0f, -1.0f, 0.0f } }  // top-right
	};

	// Indices
	unsigned int triIndices[] = { 2, 0, 1 };
	unsigned int quadIndices[] = { 3, 2, 0, 0, 1, 3 };

	// Make the geometry
	geometry triangle = makeGeometry(triVerts, 3, triIndices, 3);
	geometry quad = makeGeometry(quadVerts, 4, quadIndices, 6);
	geometry spearObj = loadGeometry("res\\soulspear.obj");

	// load up textures
	texture terry = loadTexture("res\\terry.png");

	// Source for vertex shader
	const char* basicVertShader =
		"#version 410\n"
		"layout (location = 0) in vec4 position;\n"
		"layout (location = 1) in vec4 color;\n"
		"layout (location = 2) in vec2 uv;\n"
		"out vec4 vColor;\n"
		"out vec2 vUV;\n"
		"void main() { gl_Position = position;\nvColor = color};\nvUV = uv;";

	// Source for fragment shader - mvp edition
	const char* basicFragShader =
		"#version 430\n"
		"in vec4 vColor;\n"
		"in vec2 vUV;\n"
		"out vec4 outputColor;\n"
		"layout (location = 3) uniform sampler2D mainTexture;\n"
		"layout (location = 4) uniform float time;\n"

		"void main() { outputColor = texture(mainTexture, vUV); }";

	// Source for vertex shader
	const char* mvpVertShader =
		"#version 430\n"
		"layout (location = 0) in vec4 position;\n"		// in from vertex data
		"layout (location = 1) in vec4 color\n"
		"layout (location = 2) in vec2 uv;\n"

		"layout (location = 0) uniform mat4 proj;\n"	// proj
		"layout (location = 1) uniform mat4 view;\n"	// view
		"layout (location = 2) uniform mat4 model;\n"	// model

		"layout (location = 4) uniform float time;\n"

		"out vec4 vertColor;\n"							// output to later stages
		"out vec2 vUV;\n"

		"void main() { gl_Position = proj * view * model * position;\n"
					   "vColor = color\nvUV = uv};";

	// Source for light vertex shader
	const char* lightVertShader =
		"#version 430\n"
		"layout (location = 0) in vec4 position;\n"		// in from vertex data
		"layout (location = 1) in vec4 color\n"
		"layout (location = 2) in vec2 uv;\n"
		"layout (location = 3) in vec4 normal;\n"

		"layout (location = 0) uniform mat4 proj;\n"	// proj
		"layout (location = 1) uniform mat4 view;\n"	// view
		"layout (location = 2) uniform mat4 model;\n"	// model

		"layout (location = 4) uniform float time;\n"

		"out vec2 vUV;\n"
		"out vec3 vNormal;\n"

		"void main() { gl_Position = proj * view * model * position;\n"
		"vUV = uv\nvNormal = normal.xyz};";

	// Source for light fragment shader
	const char* lightFragShader =
		"#version 430\n"
		"in vec2 vUV;\n"
		"in vec3 vNormal;\n"
		"out vec4 outputColor;\n"

		"layout (location = 3) uniform sampler2D mainTexture;\n"  // material diffuse
		"layout (location = 5) uniform vec3 ambient;\n"	// environmental ambient
		"layout (location = 6) uniform vec3 lightDiffuse;\n"	// light diffuse
		"layout (location = 7) uniform vec3 lightDirection;\n"	// light direction

		"void main() { vec3 ambientColor = ambient\n"
		"float lambert = max (0.0f, dot(vNormal, -lightDirection))\n"
		"vec3 diffuseColor = texture(mainTexture, vUV).xyz * lightDiffuse * lambert\n"
		"outputColor = vec4(ambientColor + diffuseColor, 1.0f); }";

	// Make the shader
	shader basicShader = makeShader(basicVertShader, basicFragShader);
	shader mvpShader = makeShader(mvpVertShader, basicFragShader);
	shader lightShader = loadShader(lightVertShader, lightFragShader);

	light sun = { {-1, 0, 0}, {1,1,1} };

	// Set up matrices
	glm::mat4 camProj = glm::perspective(glm::radians(80.0f), 512.f / 512.f, 0.1f, 100.0f);
	glm::mat4 camView = glm::lookAt(glm::vec3(1, 1, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	glm::mat4 triModel = glm::identity<glm::mat4>();

	//setUniform(mvpShader, 0, camProj);
	//setUniform(mvpShader, 1, camView);
	//setUniform(mvpShader, 2, triModel);

	setUniform(lightShader, 0, camProj);
	setUniform(lightShader, 1, camView);

	setUniform(lightShader, 2, triModel);

	setUniform(lightShader, 3, terry, 0);

	setUniform(lightShader, 5, { 0.1f, 0.1f, 0.1f });	// ambient light level
	setUniform(lightShader, 6, sun.color);	
	setUniform(lightShader, 7, sun.direction);


	while (!game.shouldClose())
	{
		game.tick();

		// Implement game logic here
		//triModel = glm::rotate(triModel, glm::radians(1.0f), glm::vec3(0, 1, 0));

		game.clear();

		// Implement render logic here
		setUniform(lightShader, 2, triModel);
		setUniform(lightShader, 4, game.time());

		draw(lightShader, spearObj);
	}

	freeGeometry(triangle);
	freeGeometry(quad);
	//freeGeometry(triObj);
	//freeGeometry(cubeObj);

	freeShader(basicShader);
	freeShader(mvpShader);

	game.term();

	return 0;
}