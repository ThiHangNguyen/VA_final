// glx/cleanup.hpp
#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glx/mesh.hpp"

namespace glx {

// Fonction inline pour libérer toutes les ressources OpenGL et GLFW
inline void cleanup(GLuint bgProgram, GLuint lineProgram,
                    GLuint bgTex,
                    const glx::Mesh& bg,
                    const glx::Mesh& cube,
                    const glx::Axes& axes,
                    GLFWwindow* window) {
    // --- Suppression des programmes shaders ---
    glDeleteProgram(bgProgram);   // Supprime le programme du fond
    glDeleteProgram(lineProgram); // Supprime le programme des lignes (axes, etc.)

    // --- Suppression de la texture de fond ---
    glDeleteTextures(1, &bgTex);

    // --- Libération des ressources du mesh de fond ---
    glDeleteVertexArrays(1, &bg.vao); // Supprime le Vertex Array Object
    glDeleteBuffers(1, &bg.vbo);      // Supprime le Vertex Buffer Object

    // --- Libération des ressources du cube ---
    glDeleteVertexArrays(1, &cube.vao);
    glDeleteBuffers(1, &cube.vbo);
    glDeleteBuffers(1, &cube.ebo);   // Supprime le Element Buffer Object (indices)

    // --- Libération des ressources des axes (x, y, z) ---
    glDeleteVertexArrays(1, &axes.x.vao);
    glDeleteBuffers(1, &axes.x.vbo);
    glDeleteVertexArrays(1, &axes.y.vao);
    glDeleteBuffers(1, &axes.y.vbo);
    glDeleteVertexArrays(1, &axes.z.vao);
    glDeleteBuffers(1, &axes.z.vbo);

    // --- Fermeture de la fenêtre et terminaison de GLFW ---
    glfwDestroyWindow(window); // Ferme la fenêtre
    glfwTerminate();           // Termine proprement la bibliothèque GLFW
}

} // namespace glx
