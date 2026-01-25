/**
 * @file score.cpp
 * @brief Affichage de l'écran de victoire et du score final.
 *
 * Ce fichier implémente une fenêtre OpenGL dédiée à l'écran
 * de fin de partie :
 * - animation de célébration (confettis, étoiles)
 * - affichage du temps et de la difficulté
 * - boutons de navigation (rejouer / menu)
 *
 * Le rendu est volontairement festif afin de renforcer
 * le retour utilisateur après une victoire.
 *
 * @author Thi Hang NGUYEN
 * @author Bichoy DAOUD
 */

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

// ============================================================
// EFFETS VISUELS : CONFETTIS
// ============================================================

/**
 * @brief Dessine des confettis animés tombant depuis le haut de l'écran.
 *
 * L'animation repose sur des paramètres pseudo-aléatoires
 * déterministes afin d'obtenir un rendu fluide et reproductible.
 *
 * @param w Largeur de la fenêtre.
 * @param h Hauteur de la fenêtre.
 * @param time Temps d'animation (en secondes).
 * @param count Nombre de confettis à afficher.
 */
static void drawConfetti(float w, float h, float time, int count) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Palette de couleurs festives
    float colors[][3] = {
        {1.0f, 0.2f, 0.3f},  // Rouge
        {0.2f, 0.8f, 0.3f},  // Vert
        {0.2f, 0.5f, 1.0f},  // Bleu
        {1.0f, 0.9f, 0.2f},  // Jaune
        {1.0f, 0.5f, 0.0f},  // Orange
        {0.8f, 0.2f, 1.0f},  // Violet
    };
    int numColors = 6;

    for (int i = 0; i < count; i++) {

        // Génération déterministe des paramètres du confetti
        float seed = (float)i * 1.618f;
        float x = fmod(seed * 137.5f, w);

        float fallSpeed = 80.0f + fmod(seed * 50.0f, 60.0f);
        float y = h - fmod(time * fallSpeed + seed * 100.0f, h + 50.0f);

        // Oscillation latérale pour un effet naturel
        float wobble = sin(time * 3.0f + seed * 2.0f) * 20.0f;
        x += wobble;

        // Rotation progressive
        float rotation = time * (2.0f + fmod(seed, 3.0f));

        int colorIdx = (int)(seed * 10.0f) % numColors;
        glColor4f(
            colors[colorIdx][0],
            colors[colorIdx][1],
            colors[colorIdx][2],
            0.9f
        );

        float size = 6.0f + fmod(seed * 3.0f, 4.0f);
        float halfW = size * 0.5f;
        float halfH = size * 0.3f;

        // Dessin du confetti (quad 2D)
        glPushMatrix();
        glTranslatef(x, y, 0.0f);
        glRotatef(rotation * 57.3f, 0.0f, 0.0f, 1.0f);

        glBegin(GL_QUADS);
        glVertex2f(-halfW, -halfH);
        glVertex2f( halfW, -halfH);
        glVertex2f( halfW,  halfH);
        glVertex2f(-halfW,  halfH);
        glEnd();

        glPopMatrix();
    }

    glDisable(GL_BLEND);
}

// ============================================================
// EFFETS VISUELS : ÉTOILES
// ============================================================

/**
 * @brief Dessine des particules de type "étoiles" autour du titre.
 *
 * Les étoiles orbitent autour d'un centre et pulsent
 * afin de renforcer l'effet de célébration.
 *
 * @param cx Position X du centre.
 * @param cy Position Y du centre.
 * @param radius Rayon maximal d'orbite.
 * @param time Temps d'animation.
 * @param count Nombre d'étoiles.
 */
static void drawStars(float cx, float cy, float radius, float time, int count) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (int i = 0; i < count; i++) {
        float angle = (2.0f * 3.14159f * i / count) + time * 0.5f;
        float dist = radius * (0.6f + 0.4f * sin(time * 2.0f + i));

        float px = cx + cos(angle) * dist;
        float py = cy + sin(angle) * dist;

        float alpha = 0.5f + 0.5f * sin(time * 3.0f + i * 0.7f);
        float size  = 3.0f + 2.0f * sin(time * 2.5f + i);

        // Couleur dorée
        glColor4f(1.0f, 0.85f, 0.2f, alpha);
        glPointSize(size);

        glBegin(GL_POINTS);
        glVertex2f(px, py);
        glEnd();
    }

    glDisable(GL_BLEND);
}

// ============================================================
// UTILITAIRE : DIFFICULTÉ → TEXTE
// ============================================================

/**
 * @brief Convertit un niveau de difficulté en chaîne lisible.
 *
 * @param diff Difficulté du jeu.
 * @return Chaîne correspondante ("EASY", "MEDIUM", "HARD").
 */
static std::string difficultyToString(game::Difficulty diff) {
    switch (diff) {
        case game::Difficulty::EASY:   return "EASY";
        case game::Difficulty::MEDIUM: return "MEDIUM";
        case game::Difficulty::HARD:   return "HARD";
        default: return "???";
    }
}

// ============================================================
// FENÊTRE DE SCORE / VICTOIRE
// ============================================================

/**
 * @brief Affiche la fenêtre de score final et gère le choix utilisateur.
 *
 * Cette fonction ouvre une fenêtre OpenGL dédiée affichant :
 * - un écran de victoire animé
 * - le temps final
 * - la difficulté de la partie
 * - deux actions possibles : rejouer ou retourner au menu
 *
 * @param result Résultat de la partie (temps, difficulté).
 * @return 0 si l'utilisateur choisit de rejouer.
 * @return 1 si l'utilisateur retourne au menu.
 */
int showScoreWindow(const GameResult& result) {

    // =====================================================
    // INITIALISATION GLFW / OPENGL
    // =====================================================
    if (!glfwInit()) return 1;

    GLFWwindow* window =
        glfwCreateWindow(800, 600, "VICTORY", nullptr, nullptr);

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

    // =====================================================
    // BOUCLE PRINCIPALE
    // =====================================================
    while (!glfwWindowShouldClose(window) && choice == -1) {
        glfwPollEvents();

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);

        // Projection 2D
        glViewport(0, 0, w, h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, w, 0, h, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        float animTime = (float)(glfwGetTime() - startTime);

        // Fond dégradé
        drawGradientRect(
            0, 0, w, h,
            0.02f, 0.02f, 0.08f,
            0.08f, 0.12f, 0.2f
        );

        // Effets de célébration
        drawConfetti(w, h, animTime, 50);
        drawStars(w * 0.5f, h * 0.7f, w * 0.3f, animTime, 15);

        // =================================================
        // TITRE "VICTORY"
        // =================================================
        float pulse = 1.0f + 0.05f * sin(animTime * 3.0f);
        glColor3f(0.0f, 0.9f, 1.0f);

        drawCenteredText(
            0, h * 0.68f, w, h * 0.20f,
            6.0f * pulse,
            "VICTORY !"
        );

        // =================================================
        // AFFICHAGE DU TEMPS
        // =================================================
        std::stringstream ssTime;
        ssTime << "TIME " << (int)result.time << " S";

        glColor3f(1.0f, 1.0f, 1.0f);
        drawCenteredText(0, h * 0.48f, w, h * 0.12f, 3.5f, ssTime.str());

        // =================================================
        // AFFICHAGE DE LA DIFFICULTÉ
        // =================================================
        std::string diffStr =
            "LEVEL " + difficultyToString(result.difficulty);

        glColor3f(0.7f, 0.7f, 0.7f);
        drawCenteredText(0, h * 0.34f, w, h * 0.10f, 2.5f, diffStr);

        // =================================================
        // INTERACTIONS SOURIS
        // =================================================
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);
        my = h - my;

        bool click =
            glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

        bool clickEvent = click && !wasClicked;
        wasClicked = click;

        // =================================================
        // BOUTONS
        // =================================================
        float btnW = w * 0.30f;
        float btnH = h * 0.12f;
        float btnY = h * 0.12f;
        float gap  = w * 0.08f;

        // ----------------------------
        // BOUTON REJOUER
        // ----------------------------
        float restartX = w * 0.5f - btnW - gap * 0.5f;
        bool hoverRestart =
            mx >= restartX && mx <= restartX + btnW &&
            my >= btnY      && my <= btnY + btnH;

        drawStyledButton(
            restartX, btnY, btnW, btnH,
            0.1f, 0.6f, 0.2f,
            hoverRestart
        );

        glColor3f(1, 1, 1);
        drawCenteredText(
            restartX, btnY, btnW, btnH,
            2.5f,
            "PLAY AGAIN"
        );

        if (hoverRestart && clickEvent) choice = 0;

        // ----------------------------
        // BOUTON MENU
        // ----------------------------
        float menuX = w * 0.5f + gap * 0.5f;
        bool hoverMenu =
            mx >= menuX && mx <= menuX + btnW &&
            my >= btnY  && my <= btnY + btnH;

        drawStyledButton(
            menuX, btnY, btnW, btnH,
            0.6f, 0.1f, 0.1f,
            hoverMenu
        );

        glColor3f(1, 1, 1);
        drawCenteredText(
            menuX, btnY, btnW, btnH,
            2.5f,
            "MENU"
        );

        if (hoverMenu && clickEvent) choice = 1;

        glfwSwapBuffers(window);
    }

    // Fermeture propre de la fenêtre
    glfwDestroyWindow(window);
    return choice;
}
