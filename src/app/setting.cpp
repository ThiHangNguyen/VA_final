/**
 * @file settings.cpp
 * @brief Implémentation de la page de paramètres (Settings).
 *
 * Ce fichier gère l'interface de configuration du jeu :
 * - sélection de la difficulté
 * - réglage de la physique (vitesse / rebond)
 * - choix du thème graphique
 *
 * L'interface est rendue en OpenGL 2D, avec un style
 * cohérent avec l'écran de menu et de victoire.
 *
 * @author Thi Hang NGUYEN
 * @author Bichoy DAOUD
 */

#include "app/game.hpp"
#include <GLFW/glfw3.h>
#include "game/maze.hpp"
#include <iostream>
#include <cmath>

// ============================================================
// PAGE SETTINGS (STYLE "VICTOIRE")
// ============================================================

/**
 * @brief Affiche la page de paramètres et gère les interactions utilisateur.
 *
 * Cette fonction :
 * - affiche une interface à onglets (LEVEL, SPEED, BOUNCE, DESIGN)
 * - permet de modifier dynamiquement la configuration de l'application
 * - gère la navigation vers le menu principal
 *
 * @param[in,out] inConfig Indique si l'on se trouve dans la page Settings.
 * @param[in,out] currentTab Onglet actuellement sélectionné.
 * @param[in,out] config Configuration globale de l'application.
 * @param w Largeur de la fenêtre.
 * @param h Hauteur de la fenêtre.
 * @param mx Position X de la souris.
 * @param my Position Y de la souris.
 * @param click État du clic souris (bouton gauche).
 */
void drawSettingPage(
    bool& inConfig,
    SettingTab& currentTab,
    AppConfig& config,
    int w, int h,
    double mx, double my,
    bool click
) {
    // Animation interne (pulsation, transitions légères)
    static float settingAnimTime = 0.0f;
    settingAnimTime += 0.016f;

    // ====================================================
    // BACKGROUND : dégradé sombre (cohérent avec l'écran victoire)
    // ====================================================
    glClear(GL_COLOR_BUFFER_BIT);
    drawGradientRect(
        0, 0, w, h,
        0.02f, 0.02f, 0.06f,
        0.06f, 0.08f, 0.14f
    );

    // ====================================================
    // TITRE "SETTINGS" (effet de pulsation discret)
    // ====================================================
    float titlePulse = 1.0f + 0.03f * sin(settingAnimTime * 3.0f);
    float titleSize  = h * 0.004f * titlePulse;

    glColor3f(0.0f, 0.9f, 1.0f); // Cyan
    drawText(0.04f * w, h - 0.05f * h, titleSize, "SETTINGS");

    // ====================================================
    // BOUTON BACK (retour au menu principal)
    // ====================================================
    float backW = 0.12f * w;
    float backH = 0.06f * h;
    float backX = 0.04f * w;
    float backY = 0.04f * h;

    bool hoverBack = isInside(mx, my, backX, backY, backW, backH);
    drawStyledButton(backX, backY, backW, backH,
                     0.5f, 0.15f, 0.15f,
                     hoverBack);

    glColor3f(1.0f, 1.0f, 1.0f);
    float backTextSize =
        computeTextSizeToFit(backW, backH, "BACK") * 0.4f;

    drawCenteredText(backX, backY, backW, backH, backTextSize, "BACK");

    // Retour vers le menu
    if (hoverBack && click) {
        inConfig = false;
        return;
    }

    // ====================================================
    // BARRE D'ONGLETS (LEVEL / SPEED / BOUNCE / DESIGN)
    // ====================================================
    const char* tabLabels[] = { "LEVEL", "SPEED", "BOUNCE", "DESIGN" };
    const int tabCount = 4;

    float tabAreaW = 0.92f * w;
    float tabGap   = 6;
    float tabW     = (tabAreaW - (tabCount - 1) * tabGap) / tabCount;
    float tabH     = 0.06f * h;

    float tabX0 = (w - tabAreaW) * 0.5f;
    float tabY  = h - 0.14f * h;

    for (int i = 0; i < tabCount; ++i) {
        float tx = tabX0 + i * (tabW + tabGap);

        bool hover  = isInside(mx, my, tx, tabY, tabW, tabH);
        bool active = (int)currentTab == i;

        // Onglet actif / inactif
        if (active) {
            drawStyledButton(tx, tabY, tabW, tabH,
                             0.1f, 0.5f, 0.7f,
                             false);
        } else {
            drawStyledButton(tx, tabY, tabW, tabH,
                             0.25f, 0.25f, 0.3f,
                             hover);
        }

        glColor3f(1.0f, 1.0f, 1.0f);
        float tabTextSize =
            computeTextSizeToFit(tabW, tabH, tabLabels[i]) * 0.35f;

        drawCenteredText(tx, tabY, tabW, tabH, tabTextSize, tabLabels[i]);

        // Changement d'onglet
        if (hover && click) {
            currentTab = static_cast<SettingTab>(i);
        }
    }

    // ====================================================
    // ZONE DE CONTENU (boutons contextuels par onglet)
    // ====================================================
    float bw      = 0.28f * w;
    float bh      = 0.07f * h;
    float spacing = 0.025f * h;
    float startX  = 0.08f * w;
    float startY  = 0.62f * h;

    switch (currentTab) {

        // ------------------------------------------------
        // ONGLET LEVEL : difficulté du jeu
        // ------------------------------------------------
        case SettingTab::LEVEL: {
            struct LevelBtn { const char* label; game::Difficulty value; };
            LevelBtn buttons[] = {
                { "EASY",   game::Difficulty::EASY   },
                { "MEDIUM", game::Difficulty::MEDIUM },
                { "HARD",   game::Difficulty::HARD   }
            };

            for (int i = 0; i < 3; ++i) {
                float bx = startX;
                float by = startY - i * (bh + spacing);

                bool hover  = isInside(mx, my, bx, by, bw, bh);
                bool active = (config.difficulty == buttons[i].value);

                if (active) {
                    drawStyledButton(bx, by, bw, bh,
                                     0.1f, 0.6f, 0.2f,
                                     false);
                } else {
                    drawStyledButton(bx, by, bw, bh,
                                     0.3f, 0.3f, 0.35f,
                                     hover);
                }

                glColor3f(1.0f, 1.0f, 1.0f);
                float textSize =
                    computeTextSizeToFit(bw, bh, buttons[i].label) * 0.4f;

                drawCenteredText(bx, by, bw, bh, textSize, buttons[i].label);

                if (hover && click) {
                    config.difficulty = buttons[i].value;
                    std::cout << "[SETTING] Difficulty set to "
                              << buttons[i].label << "\n";
                }
            }
            break;
        }

        // ------------------------------------------------
        // ONGLET SPEED : gravité / vitesse
        // ------------------------------------------------
        case SettingTab::SPEED: {
            struct SpeedBtn { const char* label; PhysicsMode value; };
            SpeedBtn buttons[] = {
                { "EARTH", PhysicsMode::EARTH },
                { "MOON",  PhysicsMode::MOON  }
            };

            for (int i = 0; i < 2; ++i) {
                float bx = startX;
                float by = startY - i * (bh + spacing);

                bool hover  = isInside(mx, my, bx, by, bw, bh);
                bool active = (config.speedMode == buttons[i].value);

                if (active) {
                    drawStyledButton(bx, by, bw, bh,
                                     0.1f, 0.6f, 0.2f,
                                     false);
                } else {
                    drawStyledButton(bx, by, bw, bh,
                                     0.3f, 0.3f, 0.35f,
                                     hover);
                }

                glColor3f(1.0f, 1.0f, 1.0f);
                float textSize =
                    computeTextSizeToFit(bw, bh, buttons[i].label) * 0.4f;

                drawCenteredText(bx, by, bw, bh, textSize, buttons[i].label);

                if (hover && click) {
                    config.speedMode = buttons[i].value;
                    std::cout << "[SETTING] Speed set to "
                              << buttons[i].label << "\n";
                }
            }
            break;
        }

        // ------------------------------------------------
        // ONGLET BOUNCE : comportement des rebonds
        // ------------------------------------------------
        case SettingTab::BOUNCE: {
            struct BounceBtn { const char* label; PhysicsMode value; };
            BounceBtn buttons[] = {
                { "EARTH", PhysicsMode::EARTH },
                { "MOON",  PhysicsMode::MOON  }
            };

            for (int i = 0; i < 2; ++i) {
                float bx = startX;
                float by = startY - i * (bh + spacing);

                bool hover  = isInside(mx, my, bx, by, bw, bh);
                bool active = (config.bounceMode == buttons[i].value);

                if (active) {
                    drawStyledButton(bx, by, bw, bh,
                                     0.1f, 0.6f, 0.2f,
                                     false);
                } else {
                    drawStyledButton(bx, by, bw, bh,
                                     0.3f, 0.3f, 0.35f,
                                     hover);
                }

                glColor3f(1.0f, 1.0f, 1.0f);
                float textSize =
                    computeTextSizeToFit(bw, bh, buttons[i].label) * 0.4f;

                drawCenteredText(bx, by, bw, bh, textSize, buttons[i].label);

                if (hover && click) {
                    config.bounceMode = buttons[i].value;
                    std::cout << "[SETTING] Bounce set to "
                              << buttons[i].label << "\n";
                }
            }
            break;
        }

        // ------------------------------------------------
        // ONGLET DESIGN : thème graphique
        // ------------------------------------------------
        case SettingTab::DESIGN: {
            struct DesignBtn { const char* label; DesignTheme value; };
            DesignBtn buttons[] = {
                { "DEFAULT", DesignTheme::DEFAULT },
                { "SPACE",   DesignTheme::SPACE   },
                { "DESERT",  DesignTheme::DESERT  }
            };

            for (int i = 0; i < 3; ++i) {
                float bx = startX;
                float by = startY - i * (bh + spacing);

                bool hover  = isInside(mx, my, bx, by, bw, bh);
                bool active = (config.designTheme == buttons[i].value);

                if (active) {
                    drawStyledButton(bx, by, bw, bh,
                                     0.1f, 0.6f, 0.2f,
                                     false);
                } else {
                    drawStyledButton(bx, by, bw, bh,
                                     0.3f, 0.3f, 0.35f,
                                     hover);
                }

                glColor3f(1.0f, 1.0f, 1.0f);
                float textSize =
                    computeTextSizeToFit(bw, bh, buttons[i].label) * 0.4f;

                drawCenteredText(bx, by, bw, bh, textSize, buttons[i].label);

                if (hover && click) {
                    config.designTheme = buttons[i].value;
                    std::cout << "[SETTING] Design set to "
                              << buttons[i].label << "\n";
                }
            }
            break;
        }
    }
}
