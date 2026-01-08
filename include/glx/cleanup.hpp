#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glx/mesh.hpp"

namespace glx {

// ====== Cleanup pour main_cube.cpp ======

inline void cleanup(
    GLuint bgProgram,
    GLuint lineProgram,
    GLuint bgTex,
    const glx::Mesh& bg,
    const glx::Mesh& walls,
    const glx::Axes& axes,
    GLFWwindow* window
) {
    // --- Shaders ---
    glDeleteProgram(bgProgram);
    glDeleteProgram(lineProgram);

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

// ====== Cleanup pour main.cpp avec balle et mur texturée ======
inline void cleanup(
    GLuint bgProgram,
    GLuint lineProgram,
    GLuint solidProgram,
    GLuint phongProgram,
    GLuint shadowProgram,
    GLuint bgTex,
    GLuint ballTex,
    const glx::Mesh& bg,
    const glx::Mesh& walls,
    const glx::Mesh& ball,
    const glx::Axes& axes,
    GLFWwindow* window
) {
    // Shaders
    glDeleteProgram(bgProgram);
    glDeleteProgram(lineProgram);
    glDeleteProgram(solidProgram);
    glDeleteProgram(phongProgram);
    glDeleteProgram(shadowProgram);

    // Textures
    glDeleteTextures(1, &bgTex);
    glDeleteTextures(1, &ballTex);

    // Meshes
    glDeleteVertexArrays(1, &bg.vao);    glDeleteBuffers(1, &bg.vbo);
    glDeleteVertexArrays(1, &walls.vao); glDeleteBuffers(1, &walls.vbo);
    if (walls.ebo) glDeleteBuffers(1, &walls.ebo);
    
    glDeleteVertexArrays(1, &ball.vao);  glDeleteBuffers(1, &ball.vbo);
    if (ball.ebo) glDeleteBuffers(1, &ball.ebo);

    // Axes
    glDeleteVertexArrays(1, &axes.x.vao); glDeleteBuffers(1, &axes.x.vbo);
    glDeleteVertexArrays(1, &axes.y.vao); glDeleteBuffers(1, &axes.y.vbo);
    glDeleteVertexArrays(1, &axes.z.vao); glDeleteBuffers(1, &axes.z.vbo);

    // Context
    glfwDestroyWindow(window);
    glfwTerminate();
}

} // namespace glx