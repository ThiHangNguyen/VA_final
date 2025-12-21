#include "glx/mesh.hpp"
#include <vector>
#include <array>        // OBLIGATOIRE pour std::array
#include <glm/glm.hpp>  // OBLIGATOIRE pour glm::vec3


namespace glx {

/**
 * @brief Quad plein-écran (pour fond ou rendu en texture).
 * VBO = positions + UV interleavés.
 */
Mesh createBackgroundQuad() {
  const float data[] = {
    // x, y,   u, v
    -1.f,-1.f, 0.f,0.f,
     1.f,-1.f, 1.f,0.f,
     1.f, 1.f, 1.f,1.f,
    -1.f,-1.f, 0.f,0.f,
     1.f, 1.f, 1.f,1.f,
    -1.f, 1.f, 0.f,1.f
  };

  Mesh m; m.count = 6;
  glGenVertexArrays(1, &m.vao);
  glGenBuffers(1, &m.vbo);

  glBindVertexArray(m.vao);
  glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

  // position (x,y)
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

  // UV (u,v)
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

  glBindVertexArray(0);
  return m;
}

/**
 * @brief Cube en mode fil de fer, centré à l’origine.
 */
Mesh createCubeWireframe(float size) {
  const float s = size;

  // Sommets du cube
  const float V[] = {
    -s,-s,-s,  +s,-s,-s,  +s,+s,-s,  -s,+s,-s,
    -s,-s,+s,  +s,-s,+s,  +s,+s,+s,  -s,+s,+s
  };

  // Arêtes (indices)
  const GLuint E[] = {
    0,1, 1,2, 2,3, 3,0,  // base arrière
    4,5, 5,6, 6,7, 7,4,  // base avant
    0,4, 1,5, 2,6, 3,7   // arêtes verticales
  };

  Mesh m; m.count = static_cast<GLsizei>(sizeof(E) / sizeof(E[0]));
  glGenVertexArrays(1, &m.vao);
  glGenBuffers(1, &m.vbo);
  glGenBuffers(1, &m.ebo);

  glBindVertexArray(m.vao);
  glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(V), V, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(E), E, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

  glBindVertexArray(0);
  return m;
}

/**
 * @brief Crée 3 axes (X, Y, Z) de longueur L à partir de l’origine.
 */
Axes createAxes(float L) {
  Axes A{};

  // petite lambda pour générer un segment (x1,y1,z1 -> x2,y2,z2)
  auto makeLine = [](float x1, float y1, float z1, float x2, float y2, float z2) {
    Mesh m; m.count = 2;
    const float V[6] = {x1, y1, z1, x2, y2, z2};

    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);
    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(V), V, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    return m;
  };

  // X = rouge, Y = vert, Z = bleu (couleurs gérées ailleurs)
  A.x = makeLine(0,0,0, L,0,0);
  A.y = makeLine(0,0,0, 0,L,0);
  A.z = makeLine(0,0,0, 0,0,L);

  return A;
}

// ========================================================
//  CRÉATION D’UN MUR SIMPLE (segment 3D + hauteur)
// ========================================================
Mesh createWall(float x1, float y1, float x2, float y2, float height)
{
    glm::vec3 p1(x1, y1, 0.f);
    glm::vec3 p2(x2, y2, 0.f);
    glm::vec3 p3(x1, y1, height);
    glm::vec3 p4(x2, y2, height);

    float V[] = {
        p1.x, p1.y, p1.z,
        p2.x, p2.y, p2.z,
        p3.x, p3.y, p3.z,
        p4.x, p4.y, p4.z
    };

    GLuint E[] = {
        0,1,
        2,3,
        0,2,
        1,3
    };

    Mesh m;
    m.count = 8;   // 4 segments → 8 indices

    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);
    glGenBuffers(1, &m.ebo);

    glBindVertexArray(m.vao);

    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(V), V, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(E), E, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);

    glBindVertexArray(0);
    return m;
}


// ========================================================
//  CRÉATION DE PLUSIEURS MURS EN UN SEUL MESH
// ========================================================
Mesh createWalls(const std::vector<std::array<float,4>>& segments,
                 float height)
{
    std::vector<float> vertices;
    std::vector<GLuint> indices;

    GLuint indexOffset = 0;

    for (auto& s : segments)
    {
        float x1 = s[0], y1 = s[1];
        float x2 = s[2], y2 = s[3];

        float V[] = {
            x1,y1,0.f,
            x2,y2,0.f,
            x1,y1,height,
            x2,y2,height
        };

        vertices.insert(vertices.end(), V, V+12);

        GLuint E[] = {
            indexOffset+0, indexOffset+1, indexOffset+2,
            indexOffset+1, indexOffset+3, indexOffset+2
        };

        indices.insert(indices.end(), E, E+6);

        indexOffset += 4;
    }

    Mesh m;
    m.count = indices.size();

    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);
    glGenBuffers(1, &m.ebo);

    glBindVertexArray(m.vao);

    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size()*sizeof(float),
                 vertices.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size()*sizeof(GLuint),
                 indices.data(),
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);

    glBindVertexArray(0);
    return m;
}

} // namespace glx
