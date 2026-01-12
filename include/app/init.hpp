#pragma once
#include <opencv2/opencv.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
struct GLContext {
    GLFWwindow* window;
    int width;
    int height;
};

GLContext initOpenGL(const char* title, int w, int h);
