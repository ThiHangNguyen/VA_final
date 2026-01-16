#include "app/game.hpp"
#include <map>
#include <vector>
#include <GLFW/glfw3.h>

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
        int cols = (c == 'I') ? 3 : 5;
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
    int cols = (c == 'I') ? 3 : 5;

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
