#include "game/maze.hpp"
#include <stack>
#include <algorithm>
#include <random>
#include <iostream>

namespace game {

// Dimensions physiques de la zone de jeu (intérieur du cadre A4 moins les marges)
// A4 = 210 x 297 mm. On enlève une marge de sécurité pour les murs extérieurs.
const float PLAY_W = 190.0f; 
const float PLAY_H = 270.0f;

std::pair<int, int> MazeGenerator::getDimensions(Difficulty diff) {
    switch(diff) {
        case Difficulty::EASY:   return {3, 4}; // Grosses cases, facile
        case Difficulty::MEDIUM: return {5, 7}; // Cases moyennes, moyen
        case Difficulty::HARD:   return {7, 10}; // Petites cases, difficile
    }
    return {3, 4}; // Par défaut
}

std::vector<glx::Wall> MazeGenerator::generate(Difficulty diff, float wallThickness) {
    auto dims = getDimensions(diff);
    int cols = dims.first;
    int rows = dims.second;
    
    float cellW = PLAY_W / cols;
    float cellH = PLAY_H / rows;

    // --- 1. Initialisation de la grille ---
    std::vector<Cell> grid(cols * rows);
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            grid[y * cols + x].x = x;
            grid[y * cols + x].y = y;
        }
    }

    // --- 2. Algorithme DFS (Recursive Backtracker) ---
    std::stack<Cell*> stack;
    Cell* current = &grid[0]; // Départ en bas à gauche (0,0)
    current->visited = true;
    stack.push(current);

    std::random_device rd;
    std::mt19937 g(rd());

    while (!stack.empty()) {
        current = stack.top();
        std::vector<Cell*> neighbors;

        // Vérification des voisins non visités
        // Haut
        if (current->y < rows - 1) {
            Cell* n = &grid[(current->y + 1) * cols + current->x];
            if (!n->visited) neighbors.push_back(n);
        }
        // Droite
        if (current->x < cols - 1) {
            Cell* n = &grid[current->y * cols + (current->x + 1)];
            if (!n->visited) neighbors.push_back(n);
        }
        // Bas
        if (current->y > 0) {
            Cell* n = &grid[(current->y - 1) * cols + current->x];
            if (!n->visited) neighbors.push_back(n);
        }
        // Gauche
        if (current->x > 0) {
            Cell* n = &grid[current->y * cols + (current->x - 1)];
            if (!n->visited) neighbors.push_back(n);
        }

        if (!neighbors.empty()) {
            std::uniform_int_distribution<> distr(0, neighbors.size() - 1);
            Cell* next = neighbors[distr(g)];

            // Casser le mur entre current et next
            if (next->x > current->x) { // Next est à droite
                current->right = false;
                next->left = false;
            } else if (next->x < current->x) { // Next est à gauche
                current->left = false;
                next->right = false;
            } else if (next->y > current->y) { // Next est en haut
                current->top = false;
                next->bottom = false;
            } else if (next->y < current->y) { // Next est en bas
                current->bottom = false;
                next->top = false;
            }

            next->visited = true;
            stack.push(next);
        } else {
            stack.pop();
        }
    }

    // --- 3. Conversion en murs physiques (glx::Wall) ---
    // Centrage sur (0,0)
    float startX = -PLAY_W / 2.0f;
    float startY = -PLAY_H / 2.0f;
    float hT = wallThickness / 2.0f; // Demi-épaisseur pour l'ajustement

    std::vector<glx::Wall> walls;

    // --- CADRE EXTÉRIEUR (Toujours présent) ---
    // Vertical Gauche (-105)
    walls.push_back({{-105.f, -148.5f - hT}, {-105.f, 148.5f + hT}});
    // Vertical Droite (+105)
    walls.push_back({{105.f, -148.5f - hT}, {105.f, 148.5f + hT}});
    // Horizontal Bas (-148.5) - Encastré
    walls.push_back({{-105.f + hT, -148.5f}, {105.f - hT, -148.5f}});
    // Horizontal Haut (+148.5) - Encastré
    walls.push_back({{-105.f + hT, 148.5f}, {105.f - hT, 148.5f}});

    // --- MURS INTÉRIEURS DU LABYRINTHE ---
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            Cell& c = grid[y * cols + x];
            
            float cx = startX + x * cellW; // Coin bas-gauche de la cellule
            float cy = startY + y * cellH;

            // Mur DROIT
            if (c.right && x < cols - 1) {
                float wx = cx + cellW;
                // Mur vertical : on l'allonge un peu (+hT) pour que les horizontaux s'y collent
                walls.push_back({{wx, cy - hT}, {wx, cy + cellH + hT}});
            }

            // Mur HAUT
            if (c.top && y < rows - 1) {
                float wy = cy + cellH;
                // Mur horizontal : on le raccourcit (+hT ... -hT) pour l'encastrer
                walls.push_back({{cx + hT, wy}, {cx + cellW - hT, wy}});
            }
        }
    }

    return walls;
}

} // namespace game