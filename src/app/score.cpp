#include "app/score.hpp"
#include "app/init.hpp"
#include "app/game.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <cmath>

// Dessine des particules/étoiles de célébration
static void drawStars(float cx, float cy, float radius, float time, int count) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (int i = 0; i < count; i++) {
        float angle = (2.0f * 3.14159f * i / count) + time * 0.5f;
        float dist = radius * (0.6f + 0.4f * sin(time * 2.0f + i));
        float px = cx + cos(angle) * dist;
        float py = cy + sin(angle) * dist;
        float alpha = 0.5f + 0.5f * sin(time * 3.0f + i * 0.7f);
        float size = 3.0f + 2.0f * sin(time * 2.5f + i);

        // Étoile dorée
        glColor4f(1.0f, 0.85f, 0.2f, alpha);
        glPointSize(size);
        glBegin(GL_POINTS);
        glVertex2f(px, py);
        glEnd();
    }
    glDisable(GL_BLEND);
}

// Convertit la difficulté en texte
static std::string difficultyToString(game::Difficulty diff) {
    switch (diff) {
        case game::Difficulty::EASY:   return "FACILE";
        case game::Difficulty::MEDIUM: return "MOYEN";
        case game::Difficulty::HARD:   return "DIFFICILE";
        default: return "???";
    }
}

int showScoreWindow(const GameResult& result) {
    if (!glfwInit()) return 1;

    GLFWwindow* window = glfwCreateWindow(600, 400, "VICTOIRE !", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwShowWindow(window);
    glfwFocusWindow(window);

    double startTime = glfwGetTime();
    int choice = -1;
    bool wasClicked = false;

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

        float animTime = (float)(glfwGetTime() - startTime);

        // Fond avec dégradé
        drawGradientRect(0, 0, w, h, 0.02f, 0.02f, 0.08f, 0.08f, 0.12f, 0.2f);

        // Particules de célébration
        drawStars(w * 0.5f, h * 0.7f, w * 0.3f, animTime, 15);

        // === TITRE ===
        float pulse = 1.0f + 0.05f * sin(animTime * 3.0f);
        float titleSize = 6.0f * pulse;  // Plus grand

        glColor3f(0.0f, 0.9f, 1.0f);
        drawCenteredText(0, h * 0.68f, w, h * 0.20f, titleSize, "VICTOIRE");

        // === TEMPS ===
        std::stringstream ssTime;
        ssTime << "TEMPS " << (int)result.time << " S";

        glColor3f(1.0f, 1.0f, 1.0f);
        drawCenteredText(0, h * 0.48f, w, h * 0.12f, 3.5f, ssTime.str());

        // === DIFFICULTE ===
        std::string diffStr = "NIVEAU " + difficultyToString(result.difficulty);
        glColor3f(0.7f, 0.7f, 0.7f);
        drawCenteredText(0, h * 0.34f, w, h * 0.10f, 2.5f, diffStr);

        // === BOUTONS ===
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);
        my = h - my;

        bool click = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        bool clickEvent = click && !wasClicked;
        wasClicked = click;

        float btnW = w * 0.30f;
        float btnH = h * 0.12f;
        float btnY = h * 0.12f;
        float gap = w * 0.08f;

        // Bouton REJOUER (vert)
        float restartX = w * 0.5f - btnW - gap * 0.5f;
        bool hoverRestart = (mx >= restartX && mx <= restartX + btnW &&
                            my >= btnY && my <= btnY + btnH);

        drawStyledButton(restartX, btnY, btnW, btnH, 0.1f, 0.6f, 0.2f, hoverRestart);
        glColor3f(1.0f, 1.0f, 1.0f);
        drawCenteredText(restartX, btnY, btnW, btnH, 2.5f, "REJOUER");

        if (hoverRestart && clickEvent) choice = 0;

        // Bouton MENU (rouge)
        float menuX = w * 0.5f + gap * 0.5f;
        bool hoverMenu = (mx >= menuX && mx <= menuX + btnW &&
                         my >= btnY && my <= btnY + btnH);

        drawStyledButton(menuX, btnY, btnW, btnH, 0.6f, 0.1f, 0.1f, hoverMenu);
        glColor3f(1.0f, 1.0f, 1.0f);
        drawCenteredText(menuX, btnY, btnW, btnH, 2.5f, "MENU");

        if (hoverMenu && clickEvent) choice = 1;

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    return choice;
}