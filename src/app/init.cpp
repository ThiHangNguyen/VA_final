/**
 * @file init.cpp
 * @brief Initialisation du contexte OpenGL et de la fenêtre GLFW.
 *
 * Ce fichier contient la fonction d'initialisation principale
 * du rendu OpenGL :
 * - création de la fenêtre
 * - initialisation de GLFW et GLEW
 * - configuration du contexte OpenGL
 *
 * Cette étape est indispensable avant tout appel OpenGL.
 */

#include "app/init.hpp"
#include <GL/glew.h>
#include <iostream>
#include <stdexcept>

/**
 * @brief Initialise GLFW, GLEW et crée une fenêtre OpenGL.
 *
 * Cette fonction :
 * - initialise la bibliothèque GLFW
 * - configure le contexte OpenGL (version 3.3 core)
 * - crée une fenêtre GLFW
 * - initialise GLEW pour charger les extensions OpenGL
 *
 * @param title Titre de la fenêtre.
 * @param w Largeur de la fenêtre (pixels).
 * @param h Hauteur de la fenêtre (pixels).
 * @return GLContext Structure contenant la fenêtre et ses dimensions.
 *
 * @throw std::runtime_error En cas d'échec d'initialisation GLFW,
 *        de création de fenêtre ou d'initialisation GLEW.
 */
GLContext initOpenGL(const char* title, int w, int h)
{
    // =====================================================
    // Initialisation de GLFW
    // =====================================================
    if (!glfwInit()) {
        throw std::runtime_error("GLFW init failed");
    }

    // Configuration du contexte OpenGL (version 3.3 Core)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    // Nécessaire sur macOS pour OpenGL Core Profile
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // =====================================================
    // Création de la fenêtre
    // =====================================================
    GLFWwindow* window = glfwCreateWindow(w, h, title, nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Window creation failed");
    }

    // Activation du contexte OpenGL pour la fenêtre courante
    glfwMakeContextCurrent(window);

    // Synchronisation verticale (V-Sync)
    glfwSwapInterval(1);

    // =====================================================
    // Initialisation de GLEW
    // =====================================================
    glewExperimental = GL_TRUE; // Autorise l'utilisation des extensions modernes
    if (glewInit() != GLEW_OK) {
        throw std::runtime_error("GLEW init failed");
    }

    // Nettoyage de l'erreur générée par GLEW lors de l'initialisation
    glGetError();

    // Retourne le contexte OpenGL initialisé
    return { window, w, h };
}
