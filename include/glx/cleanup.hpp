#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glx/mesh.hpp"

/**
 * @file cleanup.hpp
 * @brief Libération centralisée des ressources OpenGL et GLFW.
 *
 * Ce fichier fournit une fonction utilitaire permettant de
 * libérer proprement l'ensemble des ressources GPU et du
 * contexte GLFW en fin d'application.
 *
 * Il évite les fuites mémoire GPU et garantit une fermeture
 * propre de l'application AR.
 */

namespace glx {

/**
 * @brief Libère toutes les ressources OpenGL et détruit la fenêtre GLFW.
 *
 * Cette fonction supprime :
 * - les programmes shaders
 * - les textures OpenGL
 * - les buffers et VAO des meshes
 * - les axes de repère
 * - la fenêtre GLFW et le contexte associé
 *
 * Elle doit être appelée une seule fois à la fin du programme,
 * après la boucle principale de rendu.
 *
 * @param bgProgram Programme shader du fond
 * @param lineProgram Programme shader pour les lignes (axes, debug)
 * @param solidProgram Programme shader pour les surfaces simples
 * @param phongProgram Programme shader avec éclairage Phong
 * @param shadowProgram Programme shader pour les ombres
 * @param bgTex Texture du fond
 * @param ballTex Texture de la balle
 * @param bg Mesh du fond
 * @param walls Mesh des murs du labyrinthe
 * @param ball Mesh de la balle
 * @param axes Axes 3D de repère (X, Y, Z)
 * @param window Fenêtre GLFW à détruire
 *
 * @warning Tous les identifiants OpenGL doivent être valides
 *          au moment de l'appel.
 * @note Après cet appel, aucun appel OpenGL ne doit être effectué.
 */
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
    // =====================
    // Shaders
    // =====================
    glDeleteProgram(bgProgram);
    glDeleteProgram(lineProgram);
    glDeleteProgram(solidProgram);
    glDeleteProgram(phongProgram);
    glDeleteProgram(shadowProgram);

    // =====================
    // Textures
    // =====================
    glDeleteTextures(1, &bgTex);
    glDeleteTextures(1, &ballTex);

    // =====================
    // Meshes
    // =====================
    glDeleteVertexArrays(1, &bg.vao);
    glDeleteBuffers(1, &bg.vbo);

    glDeleteVertexArrays(1, &walls.vao);
    glDeleteBuffers(1, &walls.vbo);
    if (walls.ebo) glDeleteBuffers(1, &walls.ebo);

    glDeleteVertexArrays(1, &ball.vao);
    glDeleteBuffers(1, &ball.vbo);
    if (ball.ebo) glDeleteBuffers(1, &ball.ebo);

    // =====================
    // Axes
    // =====================
    glDeleteVertexArrays(1, &axes.x.vao);
    glDeleteBuffers(1, &axes.x.vbo);

    glDeleteVertexArrays(1, &axes.y.vao);
    glDeleteBuffers(1, &axes.y.vbo);

    glDeleteVertexArrays(1, &axes.z.vao);
    glDeleteBuffers(1, &axes.z.vbo);

    // =====================
    // GLFW
    // =====================
    glfwDestroyWindow(window);
    glfwTerminate();
}

} // namespace glx
