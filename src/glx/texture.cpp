#include "glx/texture.hpp"
#include <stdexcept>

namespace glx {

GLuint createTextureRGBA(int w, int h) {
  GLuint tex=0; glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_2D, 0);
  return tex;
}

void updateTextureRGBA(GLuint tex, const cv::Mat& rgbaFlipped) {
  glBindTexture(GL_TEXTURE_2D, tex);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, rgbaFlipped.cols, rgbaFlipped.rows,
                  GL_RGBA, GL_UNSIGNED_BYTE, rgbaFlipped.data);
  glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint createTextureFromMat(const cv::Mat& img) {
  if (img.empty()) throw std::runtime_error("Image OpenCV vide.");
  GLuint tex=0; glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex); glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  GLint internalFormat = GL_RGB8; GLenum format = GL_RGB;
  if (img.channels() == 4) { internalFormat = GL_RGBA8; format = GL_RGBA; }
  else if (img.channels() == 3) { internalFormat = GL_RGB8; format = GL_RGB; }
  else if (img.channels() == 1) { internalFormat = GL_R8;    format = GL_RED; }
  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, img.cols, img.rows, 0,
               format, GL_UNSIGNED_BYTE, img.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_2D, 0);
  return tex;
}

} // namespace glx
