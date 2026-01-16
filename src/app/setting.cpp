#include "app/game.hpp"
#include <GLFW/glfw3.h>
#include "game/maze.hpp"   
#include <iostream>
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
        case SettingTab::LEVEL: {

        float bw = 0.22f * w;
        float bh = 0.08f * h;
        float spacing = 0.04f * h;

        float startX = 0.1f * w;
        float startY = 0.6f * h;

        struct LevelBtn {
            const char* label;
            game::Difficulty value;
        };

        LevelBtn buttons[] = {
            { "EASY",   game::Difficulty::EASY   },
            { "MEDIUM", game::Difficulty::MEDIUM },
            { "HARD",   game::Difficulty::HARD   }
        };

        for (int i = 0; i < 3; ++i) {

            float bx = startX;
            float by = startY - i * (bh + spacing);

            // ✅ DÉCLARATIONS MANQUANTES
            bool hover = isInside(mx, my, bx, by, bw, bh);
            bool active = (config.difficulty == buttons[i].value);

            // 🎨 Couleur selon état
            if (active)
                glColor3f(0.25f, 0.65f, 0.25f);   // sélectionné
            else if (hover)
                glColor3f(0.55f, 0.75f, 0.55f);   // hover
            else
                glColor3f(0.75f, 0.75f, 0.75f);   // normal

            // Bouton
            glBegin(GL_QUADS);
            glVertex2f(bx, by);
            glVertex2f(bx + bw, by);
            glVertex2f(bx + bw, by + bh);
            glVertex2f(bx, by + bh);
            glEnd();

            // Texte centré
            glColor3f(0.1f, 0.1f, 0.1f);
            float textSize = computeTextSizeToFit(bw, bh, buttons[i].label);
            drawCenteredText(bx, by, bw, bh, textSize/2, buttons[i].label);

            // 🖱️ Click → change la difficulté
            if (hover && click) {
                config.difficulty = buttons[i].value;
                std::cout << "[SETTING] Difficulty set to " << buttons[i].label << "\n";
            }
        }

        break;
    }

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
