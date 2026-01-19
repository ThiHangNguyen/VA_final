/**
 * @file maze.hpp
 * @brief Generation procedurale de labyrinthe (DFS backtracking).
 *
 * On genere un labyrinthe "parfait" (un seul chemin entre 2 cases)
 * avec l'algorithme DFS randomise. La difficulte controle la taille
 * de la grille (plus c'est dur, plus il y a de cases).
 *
 * @author Thi Hang NGUYEN & Bichoy DAOUD
 */

#pragma once
#include <GL/glew.h>    
#include <GLFW/glfw3.h>
#include <vector>
#include <utility> // Pour std::pair
#include "glx/mesh.hpp" // Pour glx::Wall

namespace game {

/**
 * @enum Difficulty
 * @brief Niveaux de difficulté pour la génération du labyrinthe.
 */
enum class Difficulty {
    EASY,   ///< Peu de murs, grandes cases (ex: 3x4)
    MEDIUM, ///< Plus de murs, cases moyennes (ex: 5x7)
    HARD    ///< Beaucoup de murs, petites cases (ex: 7x10) - Optionnel
};

/**
 * @struct Cell
 * @brief Représente une cellule de la grille du labyrinthe.
 */
struct Cell {
    int x, y;
    bool visited = false;
    bool top = true;
    bool right = true;
    bool bottom = true;
    bool left = true;
};

/**
 * @class MazeGenerator
 * @brief Classe utilitaire statique pour générer le layout du labyrinthe.
 */
class MazeGenerator {
public:
    /**
     * @brief Génère un labyrinthe procédural selon la difficulté choisie.
     * Calcule automatiquement les dimensions de la grille pour remplir la zone de jeu A4.
     * @param diff Niveau de difficulté (EASY, MEDIUM, HARD).
     * @param wallThickness Épaisseur des murs pour le rendu et les collisions.
     * @return std::vector<glx::Wall> La liste des murs générés.
     */
    static std::vector<glx::Wall> generate(Difficulty diff, float wallThickness);
    
    /**
     * @brief Récupère les dimensions de la grille (colonnes, lignes) pour une difficulté donnée.
     * Utile pour placer le point de départ et d'arrivée.
     * @param diff Niveau de difficulté.
     * @return std::pair<int, int> {colonnes, lignes}.
     */
    static std::pair<int, int> getDimensions(Difficulty diff);
};

} // namespace game