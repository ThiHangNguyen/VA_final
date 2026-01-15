#pragma once
#include <GL/glew.h>
#include <vector>
#include <array>        // OBLIGATOIRE pour std::array
#include <glm/glm.hpp>  // OBLIGATOIRE pour glm::vec3

namespace game { enum class Difficulty; }
/**
 * @file mesh.hpp
 * @brief Petites fonctions utilitaires pour créer des géométries simples (quad, cube, axes).
 */
namespace glx {

// Représente une géométrie OpenGL basique (VAO/VBO/EBO)
struct Mesh {
  GLuint vao = 0;   ///< Vertex Array Object
  GLuint vbo = 0;   ///< Vertex Buffer Object
  GLuint ebo = 0;   ///< Element Buffer Object  
  GLsizei count = 0;///< Nombre de sommets / indices
};

struct Wall {
    glm::vec2 start;
    glm::vec2 end;
};
// Regroupe trois Mesh pour les axes x/y/z
struct Axes { Mesh x, y, z; };

// Quad plein-écran (pour fond ou post-process)
Mesh createBackgroundQuad();

// Cube en mode fil de fer
Mesh createCubeWireframe(float size);

// Axes X/Y/Z centrés à l'origine
Axes createAxes(float L);
// Déclarations des fonctions avec le nouveau type Wall
std::vector<Wall> createMazeLayout(float thickness, game::Difficulty difficulty);
Mesh createWalls(const std::vector<Wall>& segments, float height, float thickness);
Mesh createWallsWireframe(const std::vector<Wall>& segments, float height, float thickness);
Mesh createSphere(float radius, int slices, int stacks);
} // namespace glx
