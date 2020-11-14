#include "context.h"

// System libraries
#include <iostream>   // cout
#include <cstdio>     // printf
#include <cassert>

// Project libraries
#include "glew/GL/glew.h"   // Must come before glfw
#include "glfw/glfw3.h"

void APIENTRY errorCallBack(GLenum source, GLenum type, GLuint id, 
    GLenum serverity, GLsizei length, const GLchar* message, const void* userParam)
{
    fprintf(stderr, message);
}

bool context::init(int width, int height, const char *title)
{
    // Initialize glfw
    int glfwStatus = glfwInit();
    // Test for errors w/ if statement
    if (glfwStatus == GLFW_FALSE)
    {
        printf("GLFW ERROR");
        return false;
    }
    // Or use assert based error checking (will crash program)
    assert(glfwStatus != GLFW_FALSE && "GLFW ERROR");

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Initialize glew
    int glewStatus = glewInit();
    if (glewStatus != GLEW_OK)
    {
        printf("GLEW ERROR");
        return false;
    }

    // Print diagnostics
    // GL_VERSION
    // GL_RENDER
    // GL_VENDOR
    // GL_SHADING_LANGUAGE_VERSION
    printf("OpenGL Version: %s\n", (const char*)glGetString(GL_VERSION));
    printf("Renderer: %s\n", (const char*)glGetString(GL_RENDERER));
    printf("Vendor: %s\n", (const char*)glGetString(GL_VENDOR));
    printf("GLSL: %s\n", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Add error logging logic
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    glDebugMessageCallback(errorCallBack, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, true);

    glEnable(GL_CULL_FACE);   // ogl defaults to off
    glCullFace(GL_BACK);      // cull back faces
    glFrontFace(GL_CCW);      // ccw faces are front faces

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Set up defaults
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);

    // Return true if no errors
    return true;
}

void context::tick()
{
    // Update GLFW
    glfwPollEvents();           // update events, input, etc.
    glfwSwapBuffers(window);    // update window (the one in Windows)
}

void context::term()
{
    // Clean up window
    glfwDestroyWindow(window);
    window = nullptr;

    // Clean up GLFW (also terminates GLEW)
    glfwTerminate();
}

void context::clear()
{
    // Clears screen and replaces it w/ color 
    // specified in glClearColor() function call
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

bool context::shouldClose() const
{
    return glfwWindowShouldClose(window);
}

float context::time() const
{
    return (float)glfwGetTime();
}

void context::resetTime(float resetValue = 0.0)
{
    glfwSetTime(resetValue);
}