#pragma once
#include <GL/glew.h>
#include <vector>

/**
 * @file shaders.hpp
 * @brief Utilitaires de compilation/édition de programmes GLSL.
 */
namespace glx {

GLuint compile(GLenum type, const char* src);
GLuint link(const std::vector<GLuint>& shaders);

// Shaders source (BG, LINES + GS) – fournis comme littéraux
extern const char* const BG_VS;
extern const char* const BG_FS;
extern const char* const LINE_VS;
extern const char* const LINE_GS;
extern const char* const LINE_FS;

} // namespace glx
