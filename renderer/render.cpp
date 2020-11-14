#include "render.h"

#include <vector>   // std::vector
#include <cassert>  // assert
#include <cstddef>  // c-style function calls like fprintf
#include <string>   // std::string, std::getline
#include <fstream>  // fstream

#include "glm/ext.hpp"
#include "tinyobjloader/tiny_obj_loader.h"
#include "stb/stb_image.h"

geometry loadGeometry(const char* filePath)
{
    // load up all of the data from the file
    tinyobj::attrib_t vertexAttributes;

    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string error;

    // double-check that everything's OK
    // bool success = tinyobj::LoadObj(&vertexAttributes, &shapes, &materials, &error, filePath);

    //if (!success)
    //{
    //    fprintf(stderr, error.c_str());
    //    return {};  // return empty geo -- indicating failure
    //}

    // get mesh data
    std::vector<vertex> vertices;
    std::vector<unsigned int> indices;

    // form geometry data out of the mesh data provided by tinyobj
    size_t offset = 0;
    for (size_t i = 0; i < shapes[0].mesh.num_face_vertices.size(); ++i)
    {
        unsigned char faceVertices = shapes[0].mesh.num_face_vertices[i];

        assert(faceVertices == 3 && "unsupported number of face vertices");

        // Iterate over vertex data
        for (unsigned char j = 0; j < faceVertices; ++j)
        {
            tinyobj::index_t idx = shapes[0].mesh.indices[offset + j];

            // position
            tinyobj::real_t posX = vertexAttributes.vertices[3 * idx.vertex_index + 0];
            tinyobj::real_t posY = vertexAttributes.vertices[3 * idx.vertex_index + 1];
            tinyobj::real_t posZ = vertexAttributes.vertices[3 * idx.vertex_index + 2];

            // vertex colors
            tinyobj::real_t colR = 1.0f;
            tinyobj::real_t colG = 1.0f;
            tinyobj::real_t colB = 1.0f;

            // UVs
            tinyobj::real_t texU = vertexAttributes.texcoords[2 * idx.texcoord_index + 0];
            tinyobj::real_t texV = vertexAttributes.texcoords[2 * idx.texcoord_index + 1];

            // normal
            tinyobj::real_t norX = vertexAttributes.normals[3 * idx.normal_index + 0];
            tinyobj::real_t norY = vertexAttributes.normals[3 * idx.normal_index + 1];
            tinyobj::real_t norZ = vertexAttributes.normals[3 * idx.normal_index + 2];

            vertices.push_back(
                vertex
                {
                    {posX, posY, posZ, 1.0f},   // vertex position
                    {colR, colG, colB, 1.0f},   // vertex color
                    {texU, texV},               // texture coordinates
                    {norX, norY, norZ, 0.0f}    // vertex normal
                }
            );
            indices.push_back(faceVertices * i + j);
        }
        offset += faceVertices;
    }

    // Return makeGeometry using the data from tinyobj
    return makeGeometry(vertices.data(), vertices.size(), indices.data(), indices.size());
}

geometry makeGeometry(vertex* verts, size_t vertCount, unsigned int* indices, size_t indxCount)
{
    // Make an instance of geometry
    geometry newGeo = {};
    newGeo.size = indxCount;

    // Generate buffers and VAO
    glGenBuffers(1, &newGeo.vbo);   // 1 specifies number of buffer objects
    glGenBuffers(1, &newGeo.ibo);
    glGenVertexArrays(1, &newGeo.vao);

    // Bind the VAO and buffers (NOTE: order matters)
    glBindVertexArray(newGeo.vao);                      // bind VAO
    glBindBuffer(GL_ARRAY_BUFFER, newGeo.vbo);          // bind VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newGeo.ibo);  // bind IBO

    // Populate the buffers
    glBufferData(GL_ARRAY_BUFFER, vertCount * sizeof(vertex), verts, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indxCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    auto off = offsetof(vertex, pos);

    // Describe the data contained within the buffers
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,                // position
                          4,                // number of things
                          GL_FLOAT,         // type of things in the thing
                          GL_FALSE,         // normalize this or not
                          sizeof(vertex),   // byte offset between vertices
                          (void*)0);        // byte offset within a vertex to get this data

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,                 // vertex color
                          4,                 // number of things
                          GL_FLOAT,          // type of things in the thing
                          GL_FALSE,          // normalize this or not
                          sizeof(vertex),    // byte offset between vertices
                          (void*)16);        // byte offset within a vertex to get this data

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2,                  // uv
                          2,                  // number of things
                          GL_FLOAT,           // type of things in the thing
                          GL_FALSE,           // normalize this or not
                          sizeof(vertex),     // byte offset between vertices
                          (void*)32);         // byte offset within a vertex to get this data

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3,                                    // normals
                          4,                                    // how many things? (xyzw)
                          GL_FLOAT,                             // what types of things? floats
                          GL_FALSE,                             // normalize this? no
                          sizeof(vertex),                       // how much data to skip to get to the next set of UV data?
                          (void*)offsetof(vertex, normal));     // byte offset within a vertex to get to this data - 40

    // Unbind the buffers ( VAO, then the buffers)
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Return the geo

    return newGeo;
}

void freeGeometry(geometry& geo)
{
    glDeleteBuffers(1, &geo.vbo);
    glDeleteBuffers(1, &geo.ibo);
    glDeleteVertexArrays(1, &geo.vao);

    // Deleted
    geo = {};
}

texture loadTexture(const char* filePath)
{
    assert(filePath != nullptr && "File path was invalid.");

    // Variables to store data
    int imageWidth, imageHeight, imageFormat;
    unsigned char* rawPixelData = nullptr;

    // Use stb to load image
    stbi_set_flip_vertically_on_load(true);
    rawPixelData = stbi_load(filePath, &imageWidth, &imageHeight, &imageFormat, STBI_default);
    assert(rawPixelData != nullptr && "Image failed to load.");

    // Pass image data to makeTexture
    texture newTexture = makeTexture(imageWidth, imageHeight, imageFormat, rawPixelData);

    // Free the image data
    stbi_image_free(rawPixelData);

    // Return texture
    return newTexture;
}

texture makeTexture(unsigned width, unsigned height, unsigned channels, const unsigned char* pixels)
{
    // Figure out OGL texture format to use
    assert(channels > 0 && channels < 5);
    GLenum oglFormat = GL_RED;
    switch (channels)
    {
    case 1:
        oglFormat = GL_RED;
        break;
    case 2:
        oglFormat = GL_RG;
        break;
    case 3:
        oglFormat = GL_RGB;
        break;
    case 4:
        oglFormat = GL_RGBA;
        break;
    }

    // Generate a texture
    texture retVal = { 0, width, height, channels };

    glGenTextures(1, &retVal.handle);

    // Bind and buffer data to it
    glBindTexture(GL_TEXTURE_2D, retVal.handle);

    glTexImage2D(GL_TEXTURE_2D,     // texture to buffer data to
                 0,                 // level
                 oglFormat,         // format specifier
                 width, height,     // width and height in pixels
                 0, oglFormat,      // border value
                 GL_UNSIGNED_BYTE,  // final format specifier
                 pixels);           // pointer to actual data

    // Set filtering settings
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // filtering applied when texel density is greater than display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  // filtering applied when texel density is less than display

    // Return the texture
    glBindTexture(GL_TEXTURE_2D, 0);

    return retVal;
}

void freeTexture(texture& tex)
{
    glDeleteTextures(1, &tex.handle);
    tex = {};  // default initialize
}

shader makeShader(const char* vertSource, const char* fragSource)
{
    // Make a shader program
    shader newShad = {};
    newShad.program = glCreateProgram(); // no parameters

    // Create the shaders
    GLuint vert = glCreateShader(GL_VERTEX_SHADER);     // this shader is a vertex shader
    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);   // this shader is a fragment

    // Compile the shaders
    glShaderSource(vert, 1, &vertSource, 0);
    glShaderSource(frag, 1, &fragSource, 0);
    glCompileShader(vert);
    glCompileShader(frag);

    // Validate the shaders
    bool shaderOK = checkShader(vert, "Vertex");
    assert(shaderOK && "Vertex shader failed to compile.");
    shaderOK = checkShader(frag, "Fragment");
    assert(shaderOK && "Fragment shader failed to compile.");

    // Attach the shaders
    glAttachShader(newShad.program, vert);  // associate the vertex shader w/ the shader program
    glAttachShader(newShad.program, frag);  // associate the fragment shader w/ the shader program

    // Link the shader program
    glLinkProgram(newShad.program);

    // Delete the shaders
    glDeleteShader(vert);
    glDeleteShader(frag);

    // Return the shader
    return newShad;
}

shader loadShader(const char* vertPath, const char* fragPath)
{
    // extract the text for vertex shader from the file
    std::fstream vertFile(vertPath,         // path to the file
        std::ios::in);    // the mode of operation (in == reading)

    assert(vertFile.is_open() && "Failed to open vertex shader file.");
    std::string vertSource;
    for (std::string line; std::getline(vertFile, line);)
    {
        vertSource += line + "\n";
    }

    vertFile.close();

    // extract the text for vertex shader from the file
    std::fstream fragFile(fragPath,         // path to the file
        std::ios::in);    // the mode of operation (in == reading)

    assert(fragFile.is_open() && "Failed to open fragment shader file.");
    std::string fragSource;
    for (std::string line; std::getline(fragFile, line);)
    {
        fragSource += line + "\n";
    }

    fragFile.close();

    return makeShader(vertSource.c_str(), fragSource.c_str());

    // extract the text for fragment shader from the file
    // pass it along to makeShader and return the final return value
}

void freeShader(shader& shad)
{
    glDeleteProgram(shad.program);
    shad = {};
}

bool checkShader(GLuint target, const char* humanReadableName)
{
    assert(glIsShader(target) && "target is not a shader -- cannot check");

    GLint shaderCompileStatus = 0;
    glGetShaderiv(target, GL_COMPILE_STATUS, &shaderCompileStatus);
    if (shaderCompileStatus != GL_TRUE)
    {
        GLsizei logLength = 0;
        GLchar message[1024];
        glGetShaderInfoLog(target, 1024, &logLength, message);
        fprintf(stderr, "\n[ERROR] %s Shader \n %s", humanReadableName, message);

        // return an empty shader if it fails
        return false;
    }

    return true;
}

void draw(const shader& shad, const geometry& geo)
{
    // Specify which shader
    glUseProgram(shad.program);

    // Specify which VAO
    glBindVertexArray(geo.vao);

    // Draw
    glDrawElements(GL_TRIANGLES,    // primitive type
                   geo.size,        // indices
                   GL_UNSIGNED_INT, // index type
                   0);
}

void setUniform(const shader& shad, GLuint location, const glm::mat4& value)
{
    // glUniform is usable, but would only affect last bound shader program
    // glProgramUniform allows specification
    glProgramUniformMatrix4fv(shad.program, location, 1, GL_FALSE, glm::value_ptr(value));
}

void setUniform(const shader& shad, GLuint location, const texture& tex, int textureSlot)
{
    // Set up texture in a slot
    glActiveTexture(GL_TEXTURE0 + textureSlot);
    glBindTexture(GL_TEXTURE_2D, tex.handle);

    // Pass the slot number over to the uniform
    glProgramUniform1i(shad.program, location, textureSlot);
}

void setUniform(const shader& shad, GLuint location, float value)
{
    glProgramUniform1f(shad.program, location, value);
}

void setUniform(const shader& shad, GLuint location, const glm::vec3& value)
{
    glProgramUniform3fv(shad.program, location, 1, glm::value_ptr(value));
}
