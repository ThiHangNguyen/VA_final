#include "app/score.hpp"
#include "app/init.hpp"
#include "app/game.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iomanip>
#include <sstream>
#include <iostream>

int showScoreWindow(const GameResult& result) {
    // Création d'une fenêtre GLFW pour éviter les conflits Qt/OpenCV
    if (!glfwInit()) return 1;

    GLFWwindow* window = glfwCreateWindow(600, 400, "Resultats du Jeu", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwShowWindow(window);
    glfwFocusWindow(window);

    int choice = -1;

    while (!glfwWindowShouldClose(window) && choice == -1) {
        glfwPollEvents();

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, w, 0, h, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // Fond sombre
        glClearColor(0.07f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Titre "VICTOIRE !"
        glColor3f(0.0f, 0.84f, 1.0f);
        drawText(w * 0.27f, h * 0.75f, h * 0.025f, "VICTOIRE");

        // Temps (formaté correctement avec la police bitmap)
        std::stringstream ss;
        ss << "TEMPS   " << (int)result.time << "s";
        std::string timeStr = ss.str();

        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(w * 0.28f, h * 0.55f, h * 0.018f, timeStr);

        // Récupération position souris
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);
        my = h - my; // Inversion Y

        bool click = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

        // Bouton RESTART
        float restartX = w * 0.15f;
        float restartY = h * 0.20f;
        float restartW = w * 0.30f;
        float restartH = h * 0.12f;

        bool hoverRestart = (mx >= restartX && mx <= restartX + restartW &&
                            my >= restartY && my <= restartY + restartH);

        glColor3f(hoverRestart ? 0.0f : 0.0f, hoverRestart ? 0.9f : 0.78f, hoverRestart ? 0.0f : 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(restartX, restartY);
        glVertex2f(restartX + restartW, restartY);
        glVertex2f(restartX + restartW, restartY + restartH);
        glVertex2f(restartX, restartY + restartH);
        glEnd();

        glColor3f(1.0f, 1.0f, 1.0f);
        drawCenteredText(restartX, restartY, restartW, restartH, restartH * 0.12f, "RESTART");

        if (hoverRestart && click) choice = 0;

        // Bouton MENU
        float menuX = w * 0.55f;
        float menuY = h * 0.20f;
        float menuW = w * 0.30f;
        float menuH = h * 0.12f;

        bool hoverMenu = (mx >= menuX && mx <= menuX + menuW &&
                         my >= menuY && my <= menuY + menuH);

        glColor3f(hoverMenu ? 0.9f : 0.78f, hoverMenu ? 0.0f : 0.0f, hoverMenu ? 0.0f : 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(menuX, menuY);
        glVertex2f(menuX + menuW, menuY);
        glVertex2f(menuX + menuW, menuY + menuH);
        glVertex2f(menuX, menuY + menuH);
        glEnd();

        glColor3f(1.0f, 1.0f, 1.0f);
        drawCenteredText(menuX, menuY, menuW, menuH, menuH * 0.12f, "MENU");

        if (hoverMenu && click) choice = 1;

        glfwSwapBuffers(window);

        // Petit délai pour éviter les clics multiples
        if (click) {
            glfwWaitEventsTimeout(0.2);
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return choice;
}