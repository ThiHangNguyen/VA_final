#pragma once
#include <GL/glew.h>

/**
 * @file mesh.hpp
 * @brief Création de géométries simples (quad plein-écran, cube wireframe, axes).
 */
namespace glx {

struct Mesh {
  GLuint vao = 0;
  GLuint vbo = 0;
  GLuint ebo = 0;
  GLsizei count = 0;
};

struct Axes { Mesh x, y, z; };

Mesh createBackgroundQuad();
Mesh createCubeWireframe(float size);
Axes createAxes(float L);

} // namespace glx
