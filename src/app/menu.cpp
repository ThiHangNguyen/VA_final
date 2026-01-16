#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include "app/game.hpp"
#include <algorithm> 

// ============================================================
// MENU
// ============================================================
bool showMenuWindow(AppConfig& config) {
    if (!glfwInit()) return false;

    GLFWwindow* window = glfwCreateWindow(800, 600, "Reality Mix - Menu", nullptr, nullptr);
    if (!window) return false;
    glfwSetWindowSizeLimits(window, 800, 600, GLFW_DONT_CARE, GLFW_DONT_CARE);

    glfwMakeContextCurrent(window);
    glDisable(GL_DEPTH_TEST);
    bool inConfig = false; 
    while (!glfwWindowShouldClose(window)) {
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);

        // Projection pixel-perfect
        glViewport(0, 0, w, h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, w, 0, h, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glClearColor(0.75f, 0.90f, 0.75f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        float bw = 0.35f * w;
        float bh = 0.12f * h;
        float spacing = 0.06f * h;

        double mx, my;
        glfwGetCursorPos(window, &mx, &my);
        my = h - my;
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
        // PAGE MENU  ORIGINAL + CONFIG)
        // ====================================================

        // ---- Bouton CONFIG (haut droite) ----
        // Bouton SETTING responsive
        float cfgW = 0.18f * w;   // 18% largeur fenêtre
        float cfgH = 0.07f * h;   // 7% hauteur fenêtre

        // Sécurité UX (évite trop petit / trop gros)
        cfgW = std::clamp(cfgW, 120.0f, 260.0f);
        cfgH = std::clamp(cfgH, 36.0f, 70.0f);

        float margin = 0.03f * h; // marge proportionnelle
        float cfgX = w - cfgW - margin;
        float cfgY = h - cfgH - margin;


        bool hoverCfg = mx >= cfgX && mx <= cfgX + cfgW &&
                        my >= cfgY && my <= cfgY + cfgH;

        glColor3f(hoverCfg ? 0.30f : 0.35f,
                  hoverCfg ? 0.55f : 0.60f,
                  hoverCfg ? 0.30f : 0.35f);

        glBegin(GL_QUADS);
        glVertex2f(cfgX, cfgY);
        glVertex2f(cfgX + cfgW, cfgY);
        glVertex2f(cfgX + cfgW, cfgY + cfgH);
        glVertex2f(cfgX, cfgY + cfgH);
        glEnd();

        glColor3f(0.1f, 0.1f, 0.1f);
        drawCenteredText(cfgX, cfgY, cfgW, cfgH, bh * 0.037f, "SETTING");
        if (hoverCfg && click) {
            inConfig = true;
        }

        // =====================
        // LAYOUT DYNAMIQUE
        // =====================
        

        float cx = (w - bw) * 0.5f;
        float cy = h * 0.5f;

        float startY = cy + spacing * 0.5f;
        float quitY  = cy - bh - spacing * 0.5f;

        

        // START
        bool hoverStart = mx >= cx && mx <= cx + bw && my >= startY && my <= startY + bh;
        glColor3f(hoverStart ? 0.20f : 0.30f, hoverStart ? 0.55f : 0.65f, hoverStart ? 0.20f : 0.30f);
        glBegin(GL_QUADS);
        glVertex2f(cx, startY);
        glVertex2f(cx + bw, startY);
        glVertex2f(cx + bw, startY + bh);
        glVertex2f(cx, startY + bh);
        glEnd();

        glColor3f(0.1f, 0.1f, 0.1f);
        drawCenteredText(cx, startY, bw, bh, bh * 0.09f, "START");

        if (hoverStart && click) {
            glfwDestroyWindow(window);
            glfwTerminate();
            return true;
        }

        // QUIT
        bool hoverQuit = mx >= cx && mx <= cx + bw && my >= quitY && my <= quitY + bh;
        glColor3f(hoverQuit ? 0.20f : 0.30f, hoverQuit ? 0.55f : 0.65f, hoverQuit ? 0.20f : 0.30f);
        glBegin(GL_QUADS);
        glVertex2f(cx, quitY);
        glVertex2f(cx + bw, quitY);
        glVertex2f(cx + bw, quitY + bh);
        glVertex2f(cx, quitY + bh);
        glEnd();

        glColor3f(0.1f, 0.1f, 0.1f);
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
    glfwTerminate();
    return false;
}
