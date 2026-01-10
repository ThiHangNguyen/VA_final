#include "glx/mesh.hpp"
#include <vector>
#include <array>        // OBLIGATOIRE pour std::array
#include <glm/glm.hpp>  // OBLIGATOIRE pour glm::vec3
#include <cmath> // Pour sqrt

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
                 float height, float thickness)
{
    std::vector<float> vertices;
    std::vector<GLuint> indices;
    GLuint idx = 0; // Compteur pour les indices

    float halfT = thickness / 2.0f;

    for (const auto& s : segments)
    {
        float x1 = s[0], y1 = s[1];
        float x2 = s[2], y2 = s[3];

        // 1. Calcul du vecteur direction du mur
        float dx = x2 - x1;
        float dy = y2 - y1;
        float len = std::sqrt(dx*dx + dy*dy);
        
        if (len < 0.001f) continue; // Sécurité si longueur nulle

        // 2. Calcul du vecteur normal (perpendiculaire) unitaire
        // Normal (-dy, dx) normalisé
        float nx = -dy / len;
        float ny =  dx / len;

        // 3. Calcul du décalage pour l'épaisseur
        float ox = nx * halfT;
        float oy = ny * halfT;

        // 4. Les 4 coins de la base du mur (au sol)
        // A (x1 décalé gauche), B (x1 décalé droite)
        // C (x2 décalé droite), D (x2 décalé gauche)
        float Ax = x1 + ox, Ay = y1 + oy;
        float Bx = x1 - ox, By = y1 - oy;
        float Cx = x2 - ox, Cy = y2 - oy;
        float Dx = x2 + ox, Dy = y2 + oy;

        // --- Construction des sommets (Vertices) ---
        // On va créer un cube : 8 sommets par mur (4 en bas, 4 en haut)
        
        // Z = 0 (Bas) -> Indices idx+0 à idx+3
        float baseV[] = {
            Ax, Ay, 0.0f,  // 0: A bas
            Bx, By, 0.0f,  // 1: B bas
            Cx, Cy, 0.0f,  // 2: C bas
            Dx, Dy, 0.0f   // 3: D bas
        };
        vertices.insert(vertices.end(), baseV, baseV + 12);

        // Z = height (Haut) -> Indices idx+4 à idx+7
        float topV[] = {
            Ax, Ay, height, // 4: A haut
            Bx, By, height, // 5: B haut
            Cx, Cy, height, // 6: C haut
            Dx, Dy, height  // 7: D haut
        };
        vertices.insert(vertices.end(), topV, topV + 12);

        // --- Construction des triangles (Indices) ---
        // Il faut fermer la boîte (Côtés + Haut + Bas éventuellement)
        
        // Face 1 (Longer A->D) : 0,3,7,4
        indices.push_back(idx+0); indices.push_back(idx+3); indices.push_back(idx+7);
        indices.push_back(idx+0); indices.push_back(idx+7); indices.push_back(idx+4);

        // Face 2 (Longer B->C) : 1,5,6,2
        indices.push_back(idx+1); indices.push_back(idx+5); indices.push_back(idx+6);
        indices.push_back(idx+1); indices.push_back(idx+6); indices.push_back(idx+2);

        // Face 3 (Bout 1 A->B) : 0,4,5,1
        indices.push_back(idx+0); indices.push_back(idx+4); indices.push_back(idx+5);
        indices.push_back(idx+0); indices.push_back(idx+5); indices.push_back(idx+1);

        // Face 4 (Bout 2 D->C) : 3,2,6,7
        indices.push_back(idx+3); indices.push_back(idx+2); indices.push_back(idx+6);
        indices.push_back(idx+3); indices.push_back(idx+6); indices.push_back(idx+7);

        // Face 5 (Dessus - Le toit) : 4,7,6,5
        indices.push_back(idx+4); indices.push_back(idx+7); indices.push_back(idx+6);
        indices.push_back(idx+4); indices.push_back(idx+6); indices.push_back(idx+5);

        // On passe au mur suivant (8 sommets ajoutés)
        idx += 8;
    }

    Mesh m;
    m.count = (GLsizei)indices.size();

    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);
    glGenBuffers(1, &m.ebo);

    glBindVertexArray(m.vao);

    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);

    glBindVertexArray(0);
    return m;
}

Mesh createSphere(float radius, int slices, int stacks) {
    Mesh m;
    std::vector<float> vertices;
    std::vector<GLuint> indices;

    for (int i = 0; i <= stacks; ++i) {
        float V = i / (float)stacks;
        float phi = V * 3.14159265f;

        for (int j = 0; j <= slices; ++j) {
            float U = j / (float)slices;
            float theta = U * 3.14159265f * 2.0f;

            // Position x,y,z
            float x = cos(theta) * sin(phi);
            float y = cos(phi);
            float z = sin(theta) * sin(phi);

            // On ajoute: Position (3 floats) + Texture UV (2 floats)
            vertices.push_back(x * radius);
            vertices.push_back(y * radius);
            vertices.push_back(z * radius);
            vertices.push_back(1.0f - U); // 1-U pour inverser texture
            vertices.push_back(V);
        }
    }

    // Indices (Triangles)
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int p1 = i * (slices + 1) + j;
            int p2 = p1 + (slices + 1);
            indices.push_back(p1); indices.push_back(p2); indices.push_back(p1 + 1);
            indices.push_back(p1 + 1); indices.push_back(p2); indices.push_back(p2 + 1);
        }
    }

    m.count = (GLsizei)indices.size();
    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);
    glGenBuffers(1, &m.ebo);

    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
    return m;
}

// ========================================================
//  CRÉATION DE PLUSIEURS MURS EN UN SEUL MESH
// ========================================================
Mesh createWallsWireframe(const std::vector<std::array<float,4>>& segments,
                          float height, float thickness)
{
    std::vector<float> vertices;
    std::vector<GLuint> indices;
    GLuint idx = 0; 

    float halfT = thickness / 2.0f;

    for (const auto& s : segments)
    {
        float x1 = s[0], y1 = s[1];
        float x2 = s[2], y2 = s[3];
        float dx = x2 - x1;
        float dy = y2 - y1;
        float len = std::sqrt(dx*dx + dy*dy);
        if (len < 0.001f) continue;
        float nx = -dy / len;
        float ny =  dx / len;
        float ox = nx * halfT;
        float oy = ny * halfT;
        float Ax = x1 + ox, Ay = y1 + oy;
        float Bx = x1 - ox, By = y1 - oy;
        float Cx = x2 - ox, Cy = y2 - oy;
        float Dx = x2 + ox, Dy = y2 + oy;

        // --- Sommets (identique à createWalls) ---
        float baseV[] = { Ax, Ay, 0.0f,  Bx, By, 0.0f,  Cx, Cy, 0.0f,  Dx, Dy, 0.0f };
        vertices.insert(vertices.end(), baseV, baseV + 12);
        float topV[] = { Ax, Ay, height, Bx, By, height, Cx, Cy, height, Dx, Dy, height };
        vertices.insert(vertices.end(), topV, topV + 12);


        // 1. Le rectangle du BAS (4 lignes)
        indices.push_back(idx+0); indices.push_back(idx+1); // A->B
        indices.push_back(idx+1); indices.push_back(idx+2); // B->C
        indices.push_back(idx+2); indices.push_back(idx+3); // C->D
        indices.push_back(idx+3); indices.push_back(idx+0); // D->A

        // 2. Le rectangle du HAUT (4 lignes)
        indices.push_back(idx+4); indices.push_back(idx+5); // A'->B'
        indices.push_back(idx+5); indices.push_back(idx+6); // B'->C'
        indices.push_back(idx+6); indices.push_back(idx+7); // C'->D'
        indices.push_back(idx+7); indices.push_back(idx+4); // D'->A'

        // 3. Les 4 Piliers VERTICAUX
        indices.push_back(idx+0); indices.push_back(idx+4); // A->A'
        indices.push_back(idx+1); indices.push_back(idx+5); // B->B'
        indices.push_back(idx+2); indices.push_back(idx+6); // C->C'
        indices.push_back(idx+3); indices.push_back(idx+7); // D->D'

        idx += 8;
    }

    Mesh m;
    m.count = (GLsizei)indices.size();
    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);
    glGenBuffers(1, &m.ebo);

    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glBindVertexArray(0);
    return m;
}
} // namespace glx
