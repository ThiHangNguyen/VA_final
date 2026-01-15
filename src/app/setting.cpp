#include "app/game.hpp"
#include <GLFW/glfw3.h>

// ============================================================
// DRAW SETTINGS PAGE (ONGLETS DYNAMIQUES + TEXTE AUTO-FIT)
// ============================================================
void drawSettingPage(
    bool& inConfig,
    SettingTab& currentTab,
    AppConfig& config,
    int w, int h,
    double mx, double my,
    bool click
) {
    // ====================================================
    // BACKGROUND
    // ====================================================
    glClearColor(0.85f, 0.93f, 0.85f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    // ====================================================
    // TITLE (pas dans un bouton → taille simple)
    // ====================================================
    std::string fullTitle = "SETTING";

    // vitesse : 0.04 s ≈ 40 ms par caractère (rapide et clean)
    static double enterTime = -1.0;
    if (enterTime < 0.0) {
        enterTime = glfwGetTime();
    }

    std::string animatedTitle = getTypewriterText(
        fullTitle,
        enterTime,
        0.04
    );

    float titleSize = computeTitleTextSize(w, h, fullTitle)/2;

    glColor3f(0.1f, 0.1f, 0.1f);
    drawText(
        0.04f * w,
        h - 0.06f * h,
        titleSize,
        animatedTitle
    );


    // =====================
    // BACK BUTTON
    // =====================
    float backW = 0.16f * w;
    float backH = 0.075f * h;
    float backX = 0.04f * w;
    float backY = 0.05f * h;

    bool hoverBack = isInside(mx, my, backX, backY, backW, backH);

    // --- Couleur du bouton
    float backCol = hoverBack ? 0.35f : 0.15f;
    glColor3f(backCol, backCol, backCol);

    // --- Bouton (QUAD)
    glBegin(GL_QUADS);
    glVertex2f(backX, backY);
    glVertex2f(backX + backW, backY);
    glVertex2f(backX + backW, backY + backH);
    glVertex2f(backX, backY + backH);
    glEnd();

    // --- Couleur du TEXTE (OBLIGATOIRE)
    glColor3f(0.9f, 0.9f, 0.9f);   // texte clair

    float backTextSize = computeTextSizeToFit(backW, backH, "BACK");
    drawCenteredText(backX, backY, backW, backH, backTextSize / 2, "BACK");

    // --- Click
    if (hoverBack && click) {
        enterTime = -1.0;
        inConfig = false;
        return;
    }


    // ====================================================
    // TAB BAR (ONGLETS ÉGAUX + AUTO-FIT)
    // ====================================================
    const char* tabLabels[] = { "LEVEL", "SPEED", "BOUNCE", "PHYSIC" };
    const int tabCount = 4;

    float tabAreaW = 0.88f * w;
    float tabW = tabAreaW / tabCount;
    float tabH = 0.075f * h;

    float tabX0 = (w - tabAreaW) * 0.5f;
    float tabY  = h - 0.18f * h;

    for (int i = 0; i < tabCount; ++i) {
        float tx = tabX0 + i * tabW;

        bool hover  = isInside(mx, my, tx, tabY, tabW, tabH);
        bool active = (int)currentTab == i;

        glColor3f(
            active ? 0.30f : (hover ? 0.40f : 0.55f),
            active ? 0.60f : (hover ? 0.60f : 0.70f),
            active ? 0.30f : (hover ? 0.40f : 0.55f)
        );

        glBegin(GL_QUADS);
        glVertex2f(tx, tabY);
        glVertex2f(tx + tabW, tabY);
        glVertex2f(tx + tabW, tabY + tabH);
        glVertex2f(tx, tabY + tabH);
        glEnd();

        glColor3f(0.1f, 0.1f, 0.1f);
        float tabTextSize = computeTextSizeToFit(tabW, tabH, tabLabels[i])/2;
        drawCenteredText(tx, tabY, tabW, tabH, tabTextSize, tabLabels[i]);

        if (hover && click) {
            currentTab = static_cast<SettingTab>(i);
        }
    }

    // ====================================================
    // CONTENT AREA (STUB – PRÊT POUR SLIDERS / OPTIONS)
    // ====================================================
    float contentX = 0.08f * w;
    float contentY = tabY - 0.12f * h;
    float contentTextSize = h * 0.015f;

    glColor3f(0.15f, 0.15f, 0.15f);

    switch (currentTab) {
        case SettingTab::LEVEL:
            drawText(contentX, contentY, contentTextSize, "LEVEL SETTINGS");
            break;
        case SettingTab::SPEED:
            drawText(contentX, contentY, contentTextSize, "SPEED SETTINGS");
            break;
        case SettingTab::BOUNCE:
            drawText(contentX, contentY, contentTextSize, "BOUNCE SETTINGS");
            break;
        case SettingTab::PHYSIC:
            drawText(contentX, contentY, contentTextSize, "PHYSIC SETTINGS");
            break;
    }
}
