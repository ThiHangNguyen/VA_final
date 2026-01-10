#include "glx/shaders.hpp"
#include <stdexcept>
#include <iostream>
#include <vector>

namespace glx {

/**
 * @brief Compile un shader OpenGL (vertex, fragment, geometry, ...).
 * @param type type du shader (GL_VERTEX_SHADER, etc.)
 * @param src code source GLSL
 * @return ID du shader compilé
 * @throw std::runtime_error si la compilation échoue
 */
GLuint compile(GLenum type, const char* src) {
  GLuint s = glCreateShader(type);
  glShaderSource(s, 1, &src, nullptr);
  glCompileShader(s);

  GLint ok = GL_FALSE;
  glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    // Récupère et affiche le log d’erreur GLSL
    GLint logLen = 0;
    glGetShaderiv(s, GL_INFO_LOG_LENGTH, &logLen);
    std::vector<GLchar> log(std::max(1, logLen));
    glGetShaderInfoLog(s, logLen, nullptr, log.data());
    std::cerr << "Shader compile error: " << log.data() << std::endl;
    glDeleteShader(s);
    throw std::runtime_error("Shader compile failed");
  }
  return s;
}

/**
 * @brief Lie plusieurs shaders compilés dans un seul programme.
 * @param shaders liste d’IDs de shaders
 * @return ID du programme OpenGL
 * @throw std::runtime_error si le linkage échoue
 */
GLuint link(const std::vector<GLuint>& shaders) {
  GLuint p = glCreateProgram();
  for (auto s : shaders)
    glAttachShader(p, s);

  glLinkProgram(p);

  GLint ok = GL_FALSE;
  glGetProgramiv(p, GL_LINK_STATUS, &ok);
  if (!ok) {
    // Récupère et affiche le log d’erreur du linker
    GLint logLen = 0;
    glGetProgramiv(p, GL_INFO_LOG_LENGTH, &logLen);
    std::vector<GLchar> log(std::max(1, logLen));
    glGetProgramInfoLog(p, logLen, nullptr, log.data());
    std::cerr << "Program link error: " << log.data() << std::endl;
    glDeleteProgram(p);
    throw std::runtime_error("Program link failed");
  }
  return p;
}

// ====================================================================
// === Shaders GLSL intégrés (fond + lignes épaisses 2D/3D) ============
// ====================================================================

// --- Fond plein-écran (texturé) ---
const char* const BG_VS = R"(#version 330 core
layout (location = 0) in vec2 aPos;  // NDC [-1,1]
layout (location = 1) in vec2 aUV;
out vec2 vUV;
void main() {
  vUV = aUV;
  gl_Position = vec4(aPos, 0.0, 1.0);
})";

const char* const BG_FS = R"(#version 330 core
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D uTex;
void main() {
  FragColor = texture(uTex, vUV);
})";

// --- Lignes 3D avec épaisseur en pixels (Geometry Shader) ---
const char* const LINE_VS = R"(#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 uMVP;
void main() {
  gl_Position = uMVP * vec4(aPos, 1.0);
})";

const char* const LINE_GS = R"(#version 330 core
layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;
uniform float uThicknessPx;
uniform vec2 uViewport;

void main() {
  vec4 p0 = gl_in[0].gl_Position;
  vec4 p1 = gl_in[1].gl_Position;

  // conversion en NDC
  vec2 ndc0 = p0.xy / p0.w;
  vec2 ndc1 = p1.xy / p1.w;

  // direction et normale
  vec2 dir = ndc1 - ndc0;
  float len = length(dir);
  vec2 n = (len > 1e-6) ? normalize(vec2(-dir.y, dir.x)) : vec2(0.0, 1.0);

  // conversion px → NDC
  vec2 px2ndc = 2.0 / uViewport;
  vec2 off = n * uThicknessPx * px2ndc;

  // profondeur
  float z0 = p0.z / p0.w;
  float z1 = p1.z / p1.w;

  // construit le quad représentant la ligne
  vec4 v0 = vec4(ndc0 - off, z0, 1.0);
  vec4 v1 = vec4(ndc0 + off, z0, 1.0);
  vec4 v2 = vec4(ndc1 - off, z1, 1.0);
  vec4 v3 = vec4(ndc1 + off, z1, 1.0);

  gl_Position = v0; EmitVertex();
  gl_Position = v1; EmitVertex();
  gl_Position = v2; EmitVertex();
  gl_Position = v3; EmitVertex();
  EndPrimitive();
})";

const char* const LINE_FS = R"(#version 330 core
out vec4 FragColor;
uniform vec3 uColor;
void main() {
  FragColor = vec4(uColor, 1.0);
})";

const char* const SOLID_VS = R"(#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 uMVP;
void main() {
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)";

const char* const SOLID_FS = R"(#version 330 core
out vec4 FragColor;
uniform vec3 uColor;
void main() {
    FragColor = vec4(uColor, 1.0);
}
)";
const char* const TEX_OBJ_VS = R"(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
uniform mat4 uMVP;
out vec2 vUV;
void main() {
    gl_Position = uMVP * vec4(aPos, 1.0);
    vUV = aUV; 
})";

const char* const TEX_OBJ_FS = R"(#version 330 core
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D uTex;
void main() {
    FragColor = texture(uTex, vUV);
})";

// ==========================================
// SHADER PHONG (Ambient + Diffuse + Specular)
// ==========================================
const char* const PHONG_VS = R"(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec3 aNormal;

uniform mat4 uMVP;
uniform mat4 uModel;
uniform mat4 uView; // Necessaire pour la speculaire (position camera)

out vec3 vFragPos;
out vec3 vNormal;
out vec2 vUV;

void main() {
    // Position dans le monde pour l'eclairage
    vFragPos = vec3(uModel * vec4(aPos, 1.0));
    
    // Normale transformee (inverse transpose pour gerer le scale)
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;
    
    vUV = aUV;
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)";

const char* const PHONG_FS = R"(#version 330 core
in vec3 vFragPos;
in vec3 vNormal;
in vec2 vUV;

out vec4 FragColor;

uniform sampler2D uTex;
uniform vec3 uLightPos;
uniform vec3 uViewPos;
uniform vec3 uLightColor;

void main() {
    // Config materiau
    float ambientStrength = 0.4;
    float specularStrength = 0.8;
    float shininess = 32.0;

    // 1. Ambiant
    vec3 ambient = ambientStrength * uLightColor;

    // 2. Diffus
    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(uLightPos - vFragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor;

    // 3. Speculaire (Blinn-Phong)
    vec3 viewDir = normalize(uViewPos - vFragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * uLightColor;

    // Couleur finale = Lumiere * Texture
    vec4 texColor = texture(uTex, vUV);
    vec3 result = (ambient + diffuse + specular) * texColor.rgb;
    
    FragColor = vec4(result, texColor.a);
}
)";

// ==========================================
// SHADER OMBRE (Couleur unie + Transparence)
// ==========================================
const char* const SHADOW_VS = R"(#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 uMVP;

void main() {
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)";

const char* const SHADOW_FS = R"(#version 330 core
out vec4 FragColor;
uniform vec4 uColor; // vec4 pour gerer l'alpha

void main() {
    FragColor = uColor;
}
)";

} // namespace glx
