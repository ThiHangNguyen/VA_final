#include "app/game.hpp"
#include <map>
#include <vector>
#include <GLFW/glfw3.h>
#include <cmath>
#include <GL/gl.h>
#include <algorithm>

// ============================================================
// SCALING UI AUTOMATIQUE
// ============================================================

float getUIScale(int windowWidth, int windowHeight) {
    // Résolution de référence: 720×540 (Full HD)
    const float REF_WIDTH = 720.0f;
    const float REF_HEIGHT = 540.0f;

    // Calcul du scale par rapport à la hauteur (plus stable que largeur)
    float scaleH = static_cast<float>(windowHeight) / REF_HEIGHT;

    // Clamp entre 0.5 et 2.0 pour éviter les extrêmes
    return std::max(0.5f, std::min(2.0f, scaleH));
}

// ============================================================
// MINI POLICE BITMAP MAISON
// ============================================================
static std::map<char, std::vector<int>> FONT = {
    {'S',{0,1,1,1,0, 1,0,0,0,0, 0,1,1,1,0, 0,0,0,0,1, 1,1,1,1,0}},
    {'T',{1,1,1,1,1, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0}},
    {'A',{0,1,1,1,0, 1,0,0,0,1, 1,1,1,1,1, 1,0,0,0,1, 1,0,0,0,1}},
    {'R',{1,1,1,1,0, 1,0,0,0,1, 1,1,1,1,0, 1,0,1,0,0, 1,0,0,1,0}},
    {'Q',{0,1,1,1,0, 1,0,0,0,1, 1,0,0,0,1, 1,0,1,0,1, 0,1,1,1,1}},
    {'U',{1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 0,1,1,1,0}},
    {'I',{1,1,1, 0,1,0, 0,1,0, 0,1,0, 1,1,1}},
    {'E',{1,1,1,1,1, 1,0,0,0,0, 1,1,1,1,0, 1,0,0,0,0, 1,1,1,1,1}},
    {'N',{1,0,0,0,1, 1,1,0,0,1, 1,0,1,0,1, 1,0,0,1,1, 1,0,0,0,1}},
    {'G',{0,1,1,1,0, 1,0,0,0,0, 1,0,1,1,1, 1,0,0,0,1, 0,1,1,1,0}},
    {'L',{1,0,0,0,0, 1,0,0,0,0, 1,0,0,0,0, 1,0,0,0,0, 1,1,1,1,1}},
    {'V',{1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 0,1,0,1,0, 0,0,1,0,0}},
    {'O',{0,1,1,1,0, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 0,1,1,1,0}},
    {'P',{1,1,1,1,0, 1,0,0,0,1, 1,1,1,1,0, 1,0,0,0,0, 1,0,0,0,0}},
    {'H',{1,0,0,0,1, 1,0,0,0,1, 1,1,1,1,1, 1,0,0,0,1, 1,0,0,0,1}},
    {'C',{0,1,1,1,0, 1,0,0,0,0, 1,0,0,0,0, 1,0,0,0,0, 0,1,1,1,0}},
    {'B',{1,1,1,1,0, 1,0,0,0,1, 1,1,1,1,0, 1,0,0,0,1, 1,1,1,1,0}},
    {'D',{1,1,1,0,0, 1,0,0,1,0, 1,0,0,0,1, 1,0,0,1,0, 1,1,1,0,0}},
    {'Y',{1,0,0,0,1, 0,1,0,1,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0}},
    {'K',{1,0,0,0,1, 1,0,0,1,0, 1,1,1,0,0, 1,0,0,1,0, 1,0,0,0,1}},
    {'M',{1,0,0,0,1, 1,1,0,1,1, 1,0,1,0,1, 1,0,0,0,1, 1,0,0,0,1}},
    {'Z',{1,1,1,1,1, 0,0,0,1,0, 0,0,1,0,0, 0,1,0,0,0, 1,1,1,1,1}},
    {'W',{1,0,0,0,1, 1,0,0,0,1, 1,0,1,0,1, 1,0,1,0,1, 0,1,0,1,0}},
    {'F',{1,1,1,1,1, 1,0,0,0,0, 1,1,1,1,0, 1,0,0,0,0, 1,0,0,0,0}},
    {'J',{0,0,0,0,1, 0,0,0,0,1, 0,0,0,0,1, 1,0,0,0,1, 0,1,1,1,0}},
    {'X',{1,0,0,0,1, 0,1,0,1,0, 0,0,1,0,0, 0,1,0,1,0, 1,0,0,0,1}},
    {'0',{0,1,1,1,0, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 0,1,1,1,0}},
    {'1',{0,0,1,0,0, 0,1,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 1,1,1,1,1}},
    {'2',{0,1,1,1,0, 1,0,0,0,1, 0,0,1,1,0, 0,1,0,0,0, 1,1,1,1,1}},
    {'3',{1,1,1,1,0, 0,0,0,0,1, 0,1,1,1,0, 0,0,0,0,1, 1,1,1,1,0}},
    {'4',{1,0,0,1,0, 1,0,0,1,0, 1,1,1,1,1, 0,0,0,1,0, 0,0,0,1,0}},
    {'5',{1,1,1,1,1, 1,0,0,0,0, 1,1,1,1,0, 0,0,0,0,1, 1,1,1,1,0}},
    {'6',{0,1,1,1,0, 1,0,0,0,0, 1,1,1,1,0, 1,0,0,0,1, 0,1,1,1,0}},
    {'7',{1,1,1,1,1, 0,0,0,0,1, 0,0,0,1,0, 0,0,1,0,0, 0,1,0,0,0}},
    {'8',{0,1,1,1,0, 1,0,0,0,1, 0,1,1,1,0, 1,0,0,0,1, 0,1,1,1,0}},
    {'9',{0,1,1,1,0, 1,0,0,0,1, 0,1,1,1,1, 0,0,0,0,1, 0,1,1,1,0}},
    {'-',{0,0,0,0,0, 0,0,0,0,0, 1,1,1,1,1, 0,0,0,0,0, 0,0,0,0,0}},
    {':',{0,0,0, 0,1,0, 0,0,0, 0,1,0, 0,0,0}},
    {'.',{0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,1,0}},
    {'s',{0,1,1,1,0, 1,0,0,0,0, 0,1,1,1,0, 0,0,0,0,1, 1,1,1,1,0}},
    {'p',{1,1,1,1,0, 1,0,0,0,1, 1,1,1,1,0, 1,0,0,0,0, 1,0,0,0,0}},
    {'m',{1,0,0,0,1, 1,1,0,1,1, 1,0,1,0,1, 1,0,0,0,1, 1,0,0,0,1}},
    {'e',{1,1,1,1,1, 1,0,0,0,0, 1,1,1,1,0, 1,0,0,0,0, 1,1,1,1,1}},
};


float computeTextSizeToFit(
    float bw, float bh,
    const std::string& text,
    float maxRatio
) {
    // Limite par la hauteur
    float sizeByHeight = bh * maxRatio;

    // Largeur du texte si on prend cette taille
    float textWidth = getTextWidth(text, sizeByHeight);

    // Si ça dépasse en largeur → on réduit
    if (textWidth > bw * 0.9f) {
        sizeByHeight *= (bw * 0.9f) / textWidth;
    }

    return sizeByHeight;
}
float computeTitleTextSize(
    int windowW,
    int windowH,
    const std::string& text
) {
    // Limite par la hauteur (titre discret)
    float sizeByHeight = windowH * 0.012f;

    // Largeur du texte avec cette taille
    float textWidth = getTextWidth(text, sizeByHeight);

    // Limite par la largeur
    float maxWidth = windowW * 0.90f;

    if (textWidth > maxWidth && textWidth > 0.f) {
        sizeByHeight *= maxWidth / textWidth;
    }

    return sizeByHeight;
}


std::string getTypewriterText(
    const std::string& fullText,
    double startTime,
    double speedSecPerChar
) {
    double now = glfwGetTime();
    int count = static_cast<int>((now - startTime) / speedSecPerChar);

    if (count < 0) count = 0;
    if (count > (int)fullText.size())
        count = fullText.size();

    return fullText.substr(0, count);
}


bool isInside(float x, float y, float bx, float by, float bw, float bh) {
    return x >= bx && x <= bx + bw &&
           y >= by && y <= by + bh;
}

float getTextWidth(const std::string& text, float size) {
    float w = 0.f;
    for (char c : text) {
        int cols = (c == 'I' || c == ':' || c == '.') ? 3 : 5;
        w += cols * size + size;
    }
    return w - size;
}

float getTextHeight(float size) {
    return 5 * size;
}

void drawChar(float x, float y, float s, char c) {
    if (!FONT.count(c)) return;
    const auto& g = FONT[c];
    int cols = (c == 'I' || c == ':' || c == '.') ? 3 : 5;

    glBegin(GL_QUADS);
    for (int r = 0; r < 5; ++r) {
        for (int c2 = 0; c2 < cols; ++c2) {
            if (g[r * cols + c2]) {
                float px = x + c2 * s;
                float py = y - r * s;
                glVertex2f(px, py);
                glVertex2f(px + s, py);
                glVertex2f(px + s, py - s);
                glVertex2f(px, py - s);
            }
        }
    }
    glEnd();
}

void drawText(float x, float y, float s, const std::string& txt) {
    float cx = x;
    for (char c : txt) {
        if (c == ' ') {
            cx += s * 6; // espace
            continue;
        }
        drawChar(cx, y, s, c);
        cx += s * 6;
    }

}

void drawCenteredText(float bx, float by, float bw, float bh, float s, const std::string& txt) {
    float tw = getTextWidth(txt, s);
    float th = getTextHeight(s);
    drawText(
        bx + (bw - tw) * 0.5f,
        by + (bh + th) * 0.5f,
        s,
        txt
    );
}

void drawLight2D(float cx, float cy, float r,
                 float cr, float cg, float cb,
                 float alpha)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(cr, cg, cb, alpha);   // centre lumineux
    glVertex2f(cx, cy);

    glColor4f(cr, cg, cb, 0.0f);    // bord transparent
    for (int i = 0; i <= 40; ++i) {
        float a = (float)i / 40.f * 2.f * M_PI;
        glVertex2f(cx + cos(a) * r, cy + sin(a) * r);
    }
    glEnd();

    glDisable(GL_BLEND);
}
#include <GL/gl.h>
#include <cmath>

// Rectangle plein
void drawRect(float x, float y, float w, float h) {
    glBegin(GL_QUADS);
        glVertex2f(x,     y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x,     y + h);
    glEnd();
}

// Cercle plein (approximation par triangles)
void drawFilledCircle(float cx, float cy, float r, int segments) {
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy); // centre
        for (int i = 0; i <= segments; ++i) {
            float a = 2.0f * M_PI * i / segments;
            glVertex2f(
                cx + std::cos(a) * r,
                cy + std::sin(a) * r
            );
        }
    glEnd();
}

void drawLED(float cx, float cy, float r, float cr, float cg, float cb, float alpha) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Halo
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(cr, cg, cb, alpha * 0.5f);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 20; i++) {
        float ang = i * 2.0f * 3.14159f / 20.0f;
        glVertex2f(cx + cos(ang) * r * 2.5f, cy + sin(ang) * r * 2.5f);
    }
    glEnd();

    // Cœur de la LED
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(cr, cg, cb, 1.0f);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 20; i++) {
        float ang = i * 2.0f * 3.14159f / 20.0f;
        glVertex2f(cx + cos(ang) * r, cy + sin(ang) * r);
    }
    glEnd();
    glDisable(GL_BLEND);
}

void drawBarLED(float cx, float cy, float r, float cr, float cg, float cb, float alpha) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(cr, cg, cb, alpha * 0.4f);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 16; i++) {
        float a = i * 2.0f * M_PI / 16.0f;
        glVertex2f(cx + cos(a) * r * 2.2f, cy + sin(a) * r * 2.2f);
    }
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    glVertex2f(cx, cy);
    glColor4f(cr, cg, cb, alpha);
    for (int i = 0; i <= 16; i++) {
        float a = i * 2.0f * M_PI / 16.0f;
        glVertex2f(cx + cos(a) * r, cy + sin(a) * r);
    }
    glEnd();
}

void drawDiscoBall(float cx, float cy, float r, float animTime, bool isLeft, int w, int h) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    int numRays = 12;
    float sweep = sin(animTime * 0.4f) * 0.5f;
    float rotation = animTime * (isLeft ? 1.0f : -1.0f);

    // On calcule la diagonale pour être sûr de couvrir tout l'écran agrandi
    float maxRange = sqrt(w * w + h * h);

    for (int i = 0; i < numRays; i++) {
        float angle = rotation + (i * 2.0f * M_PI / numRays) + sweep;

        glBegin(GL_TRIANGLES);
        // Gamme de couleurs dynamique (Arc-en-ciel)
        float r_c = 0.5f + 0.5f * sin(animTime + i);
        float g_c = 0.5f + 0.5f * sin(animTime + i + 2.0f);
        float b_c = 0.5f + 0.5f * sin(animTime + i + 4.0f);
        glColor4f(r_c, g_c, b_c, 0.15f);

        glVertex2f(cx, cy);
        glVertex2f(cx + cos(angle - 0.18f) * maxRange, cy + sin(angle - 0.18f) * maxRange);
        glVertex2f(cx + cos(angle + 0.18f) * maxRange, cy + sin(angle + 0.18f) * maxRange);
        glEnd();
    }
    glDisable(GL_BLEND);
}

// ============================================================
// STYLE VICTOIRE - DEGRADE ET BOUTONS
// ============================================================

void drawGradientRect(float x, float y, float w, float h,
                      float r1, float g1, float b1,
                      float r2, float g2, float b2) {
    glBegin(GL_QUADS);
    glColor3f(r1, g1, b1);
    glVertex2f(x, y + h);
    glVertex2f(x + w, y + h);
    glColor3f(r2, g2, b2);
    glVertex2f(x + w, y);
    glVertex2f(x, y);
    glEnd();
}

void drawStyledButton(float x, float y, float w, float h,
                      float r, float g, float b, bool hover) {
    float factor = hover ? 1.2f : 1.0f;
    float br = std::min(1.0f, r * factor);
    float bg = std::min(1.0f, g * factor);
    float bb = std::min(1.0f, b * factor);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Ombre portee
    glColor4f(0.0f, 0.0f, 0.0f, 0.4f);
    glBegin(GL_QUADS);
    glVertex2f(x + 4, y - 4);
    glVertex2f(x + w + 4, y - 4);
    glVertex2f(x + w + 4, y + h - 4);
    glVertex2f(x + 4, y + h - 4);
    glEnd();

    // Fond du bouton avec degrade
    drawGradientRect(x, y, w, h, br * 0.7f, bg * 0.7f, bb * 0.7f, br, bg, bb);

    // Bordure lumineuse
    glColor3f(std::min(1.0f, br * 1.4f), std::min(1.0f, bg * 1.4f), std::min(1.0f, bb * 1.4f));
    glLineWidth(hover ? 3.0f : 2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
    glLineWidth(1.0f);

    glDisable(GL_BLEND);
}

// Texte anime style Minecraft (zoom/dezoom + rotation)
void drawAnimatedSignature(float baseX, float baseY, float baseSize, float animTime, int w, int h) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Effet zoom/dezoom sinusoidal (plus subtil)
    float scale = 1.0f + 0.05f * sin(animTime * 2.5f);
    float size = baseSize * scale * 0.3f; // Plus petit

    // Rotation legere
    float angle = -12.0f + 2.0f * sin(animTime * 1.5f); // Negatif pour pencher vers la droite

    std::string text = "BY HANG - BICHOY";
    float tw = getTextWidth(text, size);
    float th = getTextHeight(size);

    // Position adaptative (coin bas-droite avec marge)
    float cx = w - tw * 0.6f - 20.0f;
    float cy = 30.0f + th * 0.5f;

    // Sauvegarde et transformation
    glPushMatrix();
    glTranslatef(cx, cy, 0.0f);
    glRotatef(angle, 0.0f, 0.0f, 1.0f);

    // Ombre du texte
    glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
    drawText(-tw * 0.5f + 1, th * 0.5f - 1, size, text);

    // Texte principal avec couleur jaune/or qui pulse
    float pulse = 0.8f + 0.2f * sin(animTime * 3.0f);
    glColor4f(1.0f, 0.85f * pulse, 0.2f, 0.95f);
    drawText(-tw * 0.5f, th * 0.5f, size, text);

    glPopMatrix();
    glDisable(GL_BLEND);
}