#pragma once
#include <GL/glew.h>
#include <vector>
#include <array>        // OBLIGATOIRE pour std::array
#include <glm/glm.hpp>  // OBLIGATOIRE pour glm::vec3

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

// Regroupe trois Mesh pour les axes x/y/z
struct Axes { Mesh x, y, z; };

// Quad plein-écran (pour fond ou post-process)
Mesh createBackgroundQuad();

// Cube en mode fil de fer
Mesh createCubeWireframe(float size);

// Axes X/Y/Z centrés à l'origine
Axes createAxes(float L);

// Mur vertical entre (x1,y1) et (x2,y2) avec une certaine hauteur
Mesh createWall(float x1, float y1, float x2, float y2, float height);
Mesh createWalls(const std::vector<std::array<float,4>>& segments, float height);
Mesh createSphere(float radius, int slices, int stacks);
} // namespace glx
