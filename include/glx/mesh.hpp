/**
 * @file mesh.hpp
 * @brief Creation de geometries OpenGL (VAO/VBO/EBO).
 *
 * Fonctions utilitaires pour creer des meshes: quad plein ecran,
 * sphere UV, murs 3D du labyrinthe, axes de coordonnees, etc.
 *
 * @author Thi Hang NGUYEN & Bichoy DAOUD
 */

#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <array>
#include <glm/glm.hpp>

namespace game { enum class Difficulty; }

namespace glx {

/**
 * @struct Mesh
 * @brief Geometrie OpenGL basique avec VAO/VBO/EBO.
 */
struct Mesh {
    GLuint vao = 0;    ///< Vertex Array Object
    GLuint vbo = 0;    ///< Vertex Buffer Object
    GLuint ebo = 0;    ///< Element Buffer Object (optionnel)
    GLsizei count = 0; ///< Nombre d'elements a dessiner
};

/**
 * @struct Wall
 * @brief Segment de mur defini par 2 points 2D.
 */
struct Wall {
    glm::vec2 start;  ///< Point de depart
    glm::vec2 end;    ///< Point d'arrivee
};

/// @brief Regroupe les 3 meshes des axes X/Y/Z
struct Axes { Mesh x, y, z; };

/// @brief Cree un quad qui couvre tout l'ecran (pour le fond video)
Mesh createBackgroundQuad();

/// @brief Cree un cube en fil de fer (wireframe) pour debug
Mesh createCubeWireframe(float size);

/// @brief Cree les 3 axes colores (longueur L)
Axes createAxes(float L);

/**
 * @brief Genere le layout 2D du labyrinthe (liste de segments).
 * @param thickness Epaisseur des murs
 * @param difficulty Niveau de difficulte
 * @return Liste des murs sous forme de segments 2D
 */
std::vector<Wall> createMazeLayout(float thickness, game::Difficulty difficulty);

/**
 * @brief Cree le mesh 3D des murs a partir du layout 2D.
 * @param segments Liste des murs 2D
 * @param height Hauteur des murs
 * @param thickness Epaisseur des murs
 */
Mesh createWalls(const std::vector<Wall>& segments, float height, float thickness);

/// @brief Version wireframe des murs (contours noirs)
Mesh createWallsWireframe(const std::vector<Wall>& segments, float height, float thickness);

/**
 * @brief Cree une sphere UV pour la bille.
 * @param radius Rayon de la sphere
 * @param slices Nombre de divisions horizontales
 * @param stacks Nombre de divisions verticales
 */
Mesh createSphere(float radius, int slices, int stacks);

} // namespace glx
