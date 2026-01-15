#pragma once

#include <string>

// ===============================
// ÉTATS DE L'APPLICATION
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
    PHYSIC
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
float getTextWidth(const std::string& text, float size);
float getTextHeight(float size);

void drawChar(float x, float y, float size, char c);
void drawText(float x, float y, float size, const std::string& text);
void drawCenteredTextInButton(
    float bx, float by, float bw, float bh,
    float size,
    const std::string& text
);

// ===============================
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