#pragma once

#include <opencv2/opencv.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

/**
 * @struct GLContext
 * @brief Contexte OpenGL principal de l'application.
 *
 * Cette structure regroupe les informations essentielles
 * pour gérer une fenêtre OpenGL :
 * - pointeur vers la fenêtre GLFW
 * - largeur et hauteur de la zone de rendu
 *
 * Elle est utilisée comme point d'entrée pour le rendu AR
 * et l'affichage temps réel.
 */
struct GLContext {
    GLFWwindow* window; ///< Fenêtre GLFW associée au contexte OpenGL
    int width;          ///< Largeur de la fenêtre (en pixels)
    int height;         ///< Hauteur de la fenêtre (en pixels)
};

/**
 * @brief Initialise le contexte OpenGL et crée une fenêtre GLFW.
 *
 * Cette fonction :
 * - initialise GLFW
 * - crée une fenêtre OpenGL
 * - initialise GLEW
 * - configure le viewport OpenGL
 *
 * @param title Titre de la fenêtre
 * @param w Largeur de la fenêtre (pixels)
 * @param h Hauteur de la fenêtre (pixels)
 * @return GLContext Structure contenant le contexte OpenGL initialisé
 *
 * @warning La caméra OpenGL est supposée prête après cet appel.
 * @note Cette fonction doit être appelée avant tout rendu OpenGL.
 */
GLContext initOpenGL(const char* title, int w, int h);
