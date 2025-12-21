#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glx/mesh.hpp"

namespace glx {

inline void cleanup(
    GLuint bgProgram,
    GLuint lineProgram,
    GLuint solidProgram,
    GLuint bgTex,
    const glx::Mesh& bg,
    const glx::Mesh& walls,
    const glx::Axes& axes,
    GLFWwindow* window
) {
    // --- Shaders ---
    glDeleteProgram(bgProgram);
    glDeleteProgram(lineProgram);
    glDeleteProgram(solidProgram);

    // --- Texture ---
    glDeleteTextures(1, &bgTex);

    // --- Mesh fond ---
    glDeleteVertexArrays(1, &bg.vao);
    glDeleteBuffers(1, &bg.vbo);

    // --- Mesh murs ---
    glDeleteVertexArrays(1, &walls.vao);
    glDeleteBuffers(1, &walls.vbo);
    if (walls.ebo)
        glDeleteBuffers(1, &walls.ebo);

    // --- Axes ---
    glDeleteVertexArrays(1, &axes.x.vao);
    glDeleteBuffers(1, &axes.x.vbo);
    glDeleteVertexArrays(1, &axes.y.vao);
    glDeleteBuffers(1, &axes.y.vbo);
    glDeleteVertexArrays(1, &axes.z.vao);
    glDeleteBuffers(1, &axes.z.vbo);

    // --- GLFW ---
    glfwDestroyWindow(window);
    glfwTerminate();
}

} // namespace glx
