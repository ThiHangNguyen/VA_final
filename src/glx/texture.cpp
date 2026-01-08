#include "glx/texture.hpp"
#include <stdexcept>

namespace glx {

/**
 * @brief Crée une texture 2D RGBA vide (non initialisée).
 * @param w largeur en pixels
 * @param h hauteur en pixels
 * @return ID OpenGL de la texture
 */
GLuint createTextureRGBA(int w, int h) {
  GLuint tex = 0;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

  // filtres linéaires + clamp aux bords
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glBindTexture(GL_TEXTURE_2D, 0);
  return tex;
}

/**
 * @brief Met à jour le contenu d’une texture RGBA avec une image OpenCV (déjà retournée verticalement).
 * @param tex ID de la texture OpenGL
 * @param rgbaFlipped image RGBA (cv::Mat) alignée sur 1 octet
 */
void updateTextureRGBA(GLuint tex, const cv::Mat& rgbaFlipped) {
  glBindTexture(GL_TEXTURE_2D, tex);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                  rgbaFlipped.cols, rgbaFlipped.rows,
                  GL_RGBA, GL_UNSIGNED_BYTE, rgbaFlipped.data);
  glBindTexture(GL_TEXTURE_2D, 0);
}

/**
 * @brief Crée une texture OpenGL directement depuis une image OpenCV.
 * Gère automatiquement le nombre de canaux (1, 3 ou 4).
 * @param img image source OpenCV (8 bits par canal)
 * @return ID OpenGL de la texture
 * @throw std::runtime_error si l’image est vide
 */
GLuint createTextureFromMat(const cv::Mat& img) {
  if (img.empty())
    throw std::runtime_error("Image OpenCV vide.");

  GLuint tex = 0;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // Choix du format interne selon le nombre de canaux
  GLint internalFormat = GL_RGB8;
  GLenum format = GL_RGB;
  if (img.channels() == 4) { internalFormat = GL_RGBA8; format = GL_RGBA; }
  else if (img.channels() == 3) { internalFormat = GL_RGB8; format = GL_RGB; }
  else if (img.channels() == 1) { internalFormat = GL_R8; format = GL_RED; }

  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
               img.cols, img.rows, 0,
               format, GL_UNSIGNED_BYTE, img.data);

  // Génération mipmaps (pour texture haute qualité)
  glGenerateMipmap(GL_TEXTURE_2D);
  
  // Paramètres classiques de texture
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glBindTexture(GL_TEXTURE_2D, 0);
  return tex;
}

} // namespace glx
