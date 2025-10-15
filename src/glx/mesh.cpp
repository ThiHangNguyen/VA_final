#include "glx/mesh.hpp"
#include <vector>

namespace glx {

Mesh createBackgroundQuad() {
  const float data[] = {
    -1.f,-1.f, 0.f,0.f,  1.f,-1.f, 1.f,0.f,  1.f, 1.f, 1.f,1.f,
    -1.f,-1.f, 0.f,0.f,  1.f, 1.f, 1.f,1.f, -1.f, 1.f, 0.f,1.f
  };
  Mesh m; m.count = 6;
  glGenVertexArrays(1, &m.vao);
  glGenBuffers(1, &m.vbo);
  glBindVertexArray(m.vao);
  glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
  glBindVertexArray(0);
  return m;
}

Mesh createCubeWireframe(float size) {
  const float s = size;
  const float V[] = {
    -s,-s,-s,  +s,-s,-s,  +s,+s,-s,  -s,+s,-s,
    -s,-s,+s,  +s,-s,+s,  +s,+s,+s,  -s,+s,+s
  };
  const GLuint E[] = {
    0,1, 1,2, 2,3, 3,0,
    4,5, 5,6, 6,7, 7,4,
    0,4, 1,5, 2,6, 3,7
  };
  Mesh m; m.count = static_cast<GLsizei>(sizeof(E)/sizeof(E[0]));
  glGenVertexArrays(1, &m.vao);
  glGenBuffers(1, &m.vbo);
  glGenBuffers(1, &m.ebo);

  glBindVertexArray(m.vao);
  glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(V), V, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(E), E, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
  glBindVertexArray(0);
  return m;
}

Axes createAxes(float L) {
  Axes A{};
  auto makeLine = [](float x1,float y1,float z1,float x2,float y2,float z2){
    Mesh m; m.count = 2;
    const float V[6] = {x1,y1,z1, x2,y2,z2};
    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);
    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(V), V, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glBindVertexArray(0);
    return m;
  };
  A.x = makeLine(0,0,0, L,0,0);
  A.y = makeLine(0,0,0, 0,L,0);
  A.z = makeLine(0,0,0, 0,0,L);
  return A;
}

} // namespace glx
