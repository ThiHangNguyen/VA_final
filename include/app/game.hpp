/**
 * @file game.hpp
 * @brief Structures et fonctions pour le menu et la configuration du jeu.
 *
 * Contient les enums d'etat (AppPage, SettingTab), la config du jeu
 * (AppConfig), et les fonctions de rendu UI (boutons, texte, etc).
 *
 * @author Thi Hang NGUYEN & Bichoy DAOUD
 */

#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "game/maze.hpp"
#include "app/input.hpp"
#include <string>

// ===============================
// ETATS DE L'APPLICATION
// ===============================
enum class AppPage {
    MENU,
    CONFIG,
    START_GAME,
    QUIT
};

// ===============================
// ONGLET CONFIGURATION
// ===============================
enum class SettingTab {
    LEVEL,
    SPEED,
    BOUNCE,
    DESIGN
};

// ===============================
// CONFIGURATION GLOBALE DU JEU
// ===============================
struct AppConfig {
    int   level        = 1;
    float speed        = 1.0f;
    float restitution  = 0.8f;

    // Couleurs (RGB 0..1)
    float wallColor[3]  = {0.6f, 0.6f, 0.6f};
    float floorColor[3] = {0.3f, 0.8f, 0.3f};
    float skyColor[3]   = {0.6f, 0.8f, 1.0f};

    // Paramètres gameplay
    game::Difficulty difficulty = game::Difficulty::EASY;
    PhysicsMode speedMode = PhysicsMode::EARTH;   // Gravité
    PhysicsMode bounceMode = PhysicsMode::EARTH;  // Restitution
    DesignTheme designTheme = DesignTheme::DEFAULT; // Thème visuel
};

enum class EndReason {
    WIN,
    LOSE,
    QUIT,
    ERROR
};

struct GameResult {
    EndReason reason;
    double time = 0.0;
    game::Difficulty difficulty;
};


// ===============================
// UI — FONCTIONS UTILITAIRES
// ===============================
bool isInside(float x, float y, float bx, float by, float bw, float bh);
float computeTextSizeToFit(
    float bw,
    float bh,
    const std::string& text,
    float maxRatio = 0.15f
);
float computeTitleTextSize(
    int windowW,
    int windowH,
    const std::string& text
);
std::string getTypewriterText(
    const std::string& fullText,
    double startTime,
    double speedSecPerChar
);

// ===============================
// TEXTE (POLICE MAISON)
// ===============================

/**
 * @brief Calcule un facteur de scaling UI automatique selon la taille de fenêtre
 * @param windowWidth Largeur actuelle de la fenêtre
 * @param windowHeight Hauteur actuelle de la fenêtre
 * @return Facteur de scale (1.0 = résolution de référence 1920×1080)
 */
float getUIScale(int windowWidth, int windowHeight);

float getTextWidth(const std::string& text, float size);
float getTextHeight(float size);

void drawChar(float x, float y, float size, char c);
void drawText(float x, float y, float size, const std::string& text);
void drawCenteredTextInButton(
    float bx, float by, float bw, float bh,
    float size,
    const std::string& text
);
void drawLED(float cx, float cy, float r, float cr, float cg, float cb, float alpha);
void drawBarLED(float cx, float cy, float r, float cr, float cg, float cb, float alpha);
void drawDiscoBall(float cx, float cy, float r, float animTime, bool isLeft, int w, int h) ;

// PAGES UI
// ===============================
bool showMenuWindow(AppConfig& config);
void drawSettingPage(
    bool& inConfig,
    SettingTab& currentTab,
    AppConfig& config,
    int w, int h,
    double mx, double my,
    bool click
);
void drawCenteredText(float bx, float by, float bw, float bh, float s, const std::string& txt) ;
void drawLight2D(float cx, float cy, float r,
                 float cr, float cg, float cb,
                 float alpha);
void drawRect(float x, float y, float w, float h);
void drawFilledCircle(float cx, float cy, float r, int segments);

// Style victoire - degrade et boutons
void drawGradientRect(float x, float y, float w, float h,
                      float r1, float g1, float b1,
                      float r2, float g2, float b2);
void drawStyledButton(float x, float y, float w, float h,
                      float r, float g, float b, bool hover);
void drawAnimatedSignature(float baseX, float baseY, float baseSize, float animTime, int w, int h);