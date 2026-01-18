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
    if (!window) return false;

    glfwMakeContextCurrent(window);
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

        glClearColor(0.05f, 0.05f, 0.08f, 1.f); // Fond sombre de bar
        glClear(GL_COLOR_BUFFER_BIT);
        animTime += 0.016f;

        // À l'intérieur du while (!glfwWindowShouldClose(window))
        glfwGetFramebufferSize(window, &w, &h); // Récupère la taille RÉELLE actuelle

        // Repositionnement dynamique dans les coins
        drawDiscoBall(60.f, h - 60.f, 25.f, animTime, true, w, h);  // Haut-Gauche
        //drawDiscoBall(w - 60.f, h - 60.f, 25.f, animTime, false, w, h); // Haut-Droite

        // Guirlande LED qui s'adapte aussi à la largeur
        int numLEDs = w / 50; // Une LED tous les 50 pixels
        for (int i = 0; i < numLEDs; i++) {
            float xPos = (w / (float)(numLEDs - 1)) * i;
            float pulse = 0.5f + 0.5f * sin(animTime * 5.0f + i * 0.5f);
            drawBarLED(xPos, h - 15.f, 7.f, 1.0f, 0.2f, 0.2f, pulse);
            drawBarLED(xPos, 15.f, 7.f, 0.2f, 0.2f, 1.0f, pulse);
        }
        // 3. TEXTE ET BOUTONS
        glColor3f(1, 1, 1);
        float titleX = 0.0f;
        float titleY = h * 0.80f;
        float titleW = w;
        float titleH = h * 0.12f;

        // taille adaptative (≈ 8–10 % de la box)
        float titleSize = titleH * 0.085f;
        drawCenteredText(
            titleX,
            titleY,
            titleW,
            titleH,
            titleSize,
            "WELCOME TO AR MAZE"
        );

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
        // LAYOUT DYNAMIQUE - STYLE SQUID GAME
        // ====================================================

        // On garde tes variables de taille d'origine
        float bw = 0.45f * w;
        float bh = 0.12f * h;
        float spacing = 0.06f * h;

        // Calcul des positions pour empiler les 3 boutons au centre
        float cx = (w - bw) * 0.5f;
        float cy = h * 0.4f;

        // Positionnement vertical (Start en haut, Setting au milieu, Quit en bas)
        float startY   = cy + bh + spacing; 
        float settingY = cy;
        float quitY    = cy - bh - spacing;

        // Couleurs Squid Game (Rose Magenta)
        float pinkR = 0.97f, pinkG = 0.11f, pinkB = 0.46f;

        // --- 1. BOUTON START (Triangle) ---
        bool hoverStart = mx >= cx && mx <= cx + bw && my >= startY && my <= startY + bh;
        glColor3f(hoverStart ? pinkR + 0.1f : pinkR, hoverStart ? pinkG + 0.1f : pinkG, hoverStart ? pinkB + 0.1f : pinkB);
        glBegin(GL_QUADS);
        glVertex2f(cx, startY); glVertex2f(cx + bw, startY);
        glVertex2f(cx + bw, startY + bh); glVertex2f(cx, startY + bh);
        glEnd();
        
        // Forme Triangle
        glColor3f(1, 1, 1);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(cx + 20, startY + bh/2 + 10);
        glVertex2f(cx + 10, startY + bh/2 - 10);
        glVertex2f(cx + 30, startY + bh/2 - 10);
        glEnd();

        // On garde EXACTEMENT ta fonction de texte et sa taille bh * 0.09f
        glColor3f(1.0f, 1.0f, 1.0f); // Texte en blanc pour le contraste
        drawCenteredText(cx, startY, bw, bh, bh * 0.09f, "START");

        if (hoverStart && click) {
            glfwDestroyWindow(window);
            glfwTerminate();
            return true;
        }

        // --- 2. BOUTON SETTING (Carré) ---
        bool hoverCfg = mx >= cx && mx <= cx + bw && my >= settingY && my <= settingY + bh;
        glColor3f(hoverCfg ? pinkR + 0.1f : pinkR, hoverCfg ? pinkG + 0.1f : pinkG, hoverCfg ? pinkB + 0.1f : pinkB);
        glBegin(GL_QUADS);
        glVertex2f(cx, settingY); glVertex2f(cx + bw, settingY);
        glVertex2f(cx + bw, settingY + bh); glVertex2f(cx, settingY + bh);
        glEnd();

        // Forme Carré
        glColor3f(1, 1, 1);
        glBegin(GL_LINE_LOOP);
        glVertex2f(cx + 12, settingY + bh/2 - 10); glVertex2f(cx + 30, settingY + bh/2 - 10);
        glVertex2f(cx + 30, settingY + bh/2 + 10); glVertex2f(cx + 12, settingY + bh/2 + 10);
        glEnd();

        glColor3f(1.0f, 1.0f, 1.0f);
        drawCenteredText(cx, settingY, bw, bh, bh * 0.09f, "SETTING");

        if (hoverCfg && click) {
            inConfig = true;
        }

        // --- 3. BOUTON QUIT (Cercle) ---
        bool hoverQuit = mx >= cx && mx <= cx + bw && my >= quitY && my <= quitY + bh;
        glColor3f(hoverQuit ? pinkR + 0.1f : pinkR, hoverQuit ? pinkG + 0.1f : pinkG, hoverQuit ? pinkB + 0.1f : pinkB);
        glBegin(GL_QUADS);
        glVertex2f(cx, quitY); glVertex2f(cx + bw, quitY);
        glVertex2f(cx + bw, quitY + bh); glVertex2f(cx, quitY + bh);
        glEnd();

        // Forme Cercle
        glColor3f(1, 1, 1);
        glBegin(GL_LINE_LOOP);
        for(int i=0; i<360; i+=30) {
            float rad = i * 3.14159f / 180.0f;
            glVertex2f(cx + 21 + cos(rad)*10, quitY + bh/2 + sin(rad)*10);
        }
        glEnd();

        glColor3f(1.0f, 1.0f, 1.0f);
        drawCenteredText(cx, quitY, bw, bh, bh * 0.09f, "QUIT");

        if (hoverQuit && click) {
            glfwDestroyWindow(window);
            glfwTerminate();
            return false;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    return false;
}