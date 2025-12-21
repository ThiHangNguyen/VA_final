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
extern const char* const SOLID_VS;
extern const char* const SOLID_FS;

static const char* WALL_VS = R"(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;

uniform mat4 uMVP;
out vec2 vUV;

void main() {
    vUV = aUV;
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)";

static const char* WALL_FS = R"(#version 330 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uWallTex;

void main() {
    FragColor = texture(uWallTex, vUV);
}
)";

} // namespace glx