#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include "app/game.hpp"
#include <algorithm> 

// ============================================================
// BALLE 2D
// ============================================================
static void drawBall2D(float cx, float cy, float r) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 32; ++i) {
        float a = (float)i / 32.f * 2.f * M_PI;
        glVertex2f(cx + cos(a) * r, cy + sin(a) * r);
    }
    glEnd();
}

// ============================================================
// MENU
// ============================================================
bool showMenuWindow(AppConfig& config) {
    if (!glfwInit()) return false;

    GLFWwindow* window = glfwCreateWindow(800, 600, "Reality Mix - Menu Bar", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);

    // Réinitialiser GLEW pour le nouveau contexte OpenGL
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Erreur: Impossible d'initialiser GLEW dans le menu\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return false;
    }

    glfwShowWindow(window);
    glfwFocusWindow(window); // Empêche la notification "est prêt"

    float animTime = 0.0f;
    bool inConfig = false; 
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        int w, h; glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glMatrixMode(GL_PROJECTION); glLoadIdentity();
        glOrtho(0, w, 0, h, -1, 1);
        glMatrixMode(GL_MODELVIEW); glLoadIdentity();

        // Fond degrade style victoire (sombre en bas, moins sombre en haut)
        glClear(GL_COLOR_BUFFER_BIT);
        drawGradientRect(0, 0, w, h, 0.02f, 0.02f, 0.06f, 0.08f, 0.10f, 0.18f);
        animTime += 0.016f;

        // Repositionnement dynamique dans les coins
        drawDiscoBall(60.f, h - 60.f, 25.f, animTime, true, w, h);  // Haut-Gauche

        // Guirlande LED qui s'adapte aussi à la largeur
        int numLEDs = w / 50;
        for (int i = 0; i < numLEDs; i++) {
            float xPos = (w / (float)(numLEDs - 1)) * i;
            float pulse = 0.5f + 0.5f * sin(animTime * 5.0f + i * 0.5f);
            drawBarLED(xPos, h - 15.f, 7.f, 1.0f, 0.2f, 0.2f, pulse);
            drawBarLED(xPos, 15.f, 7.f, 0.2f, 0.2f, 1.0f, pulse);
        }

        // Titre avec effet pulse
        float titlePulse = 1.0f + 0.03f * sin(animTime * 3.0f);
        float titleX = 0.0f;
        float titleY = h * 0.78f;
        float titleW = w;
        float titleH = h * 0.12f;
        float titleSize = titleH * 0.085f * titlePulse;

        glColor3f(0.0f, 0.9f, 1.0f); // Cyan comme victoire
        drawCenteredText(titleX, titleY, titleW, titleH, titleSize, "AR MAZE");

        double mx, my; glfwGetCursorPos(window, &mx, &my); my = h - my;
        bool click = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

        static SettingTab currentTab = SettingTab::LEVEL;
        if (inConfig) {
            drawSettingPage(
                inConfig,
                currentTab,
                config,
                w, h,
                mx, my,
                click
            );

            glfwSwapBuffers(window);
            glfwPollEvents();
            continue;
        }
        // ====================================================
        // LAYOUT DYNAMIQUE - STYLE VICTOIRE
        // ====================================================

        float bw = 0.40f * w;
        float bh = 0.11f * h;
        float spacing = 0.05f * h;

        float cx = (w - bw) * 0.5f;
        float cy = h * 0.38f;

        float startY   = cy + bh + spacing;
        float settingY = cy;
        float quitY    = cy - bh - spacing;

        // --- 1. BOUTON START (vert) ---
        bool hoverStart = mx >= cx && mx <= cx + bw && my >= startY && my <= startY + bh;
        drawStyledButton(cx, startY, bw, bh, 0.1f, 0.6f, 0.2f, hoverStart);
        glColor3f(1.0f, 1.0f, 1.0f);
        drawCenteredText(cx, startY, bw, bh, bh * 0.08f, "START");

        if (hoverStart && click) {
            glfwDestroyWindow(window);
            return true;
        }

        // --- 2. BOUTON SETTING (bleu) ---
        bool hoverCfg = mx >= cx && mx <= cx + bw && my >= settingY && my <= settingY + bh;
        drawStyledButton(cx, settingY, bw, bh, 0.2f, 0.4f, 0.7f, hoverCfg);
        glColor3f(1.0f, 1.0f, 1.0f);
        drawCenteredText(cx, settingY, bw, bh, bh * 0.08f, "SETTING");

        if (hoverCfg && click) {
            inConfig = true;
        }

        // --- 3. BOUTON QUIT (rouge) ---
        bool hoverQuit = mx >= cx && mx <= cx + bw && my >= quitY && my <= quitY + bh;
        drawStyledButton(cx, quitY, bw, bh, 0.6f, 0.1f, 0.1f, hoverQuit);
        glColor3f(1.0f, 1.0f, 1.0f);
        drawCenteredText(cx, quitY, bw, bh, bh * 0.08f, "QUIT");

        if (hoverQuit && click) {
            glfwDestroyWindow(window);
            glfwTerminate();
            return false;
        }

        // --- SIGNATURE ANIMEE "by Hang & Bichoy" ---
        float sigSize = h * 0.012f;
        drawAnimatedSignature(w - 100.f, 45.f, sigSize, animTime, w, h);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate(); // Fermeture de la fenêtre par X -> on quitte
    return false;
}