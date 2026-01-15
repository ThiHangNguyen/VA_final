#include "ar/render.hpp"
#include "glx/shaders.hpp"
#include "glx/texture.hpp"
#include <opencv2/imgproc.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

static GLuint makeProgram(std::initializer_list<std::pair<GLenum, const char*>> shaders)
{
    std::vector<GLuint> ids;
    for (auto& s : shaders)
        ids.push_back(glx::compile(s.first, s.second));

    GLuint prog = glx::link(ids);

    for (auto id : ids)
        glDeleteShader(id);

    return prog;
}

ARRenderContext createRenderContext()
{
    ARRenderContext ctx{};

    // === SHADERS ===
    ctx.bgProgram = makeProgram({
        {GL_VERTEX_SHADER,   glx::BG_VS},
        {GL_FRAGMENT_SHADER, glx::BG_FS}
    });

    ctx.lineProgram = makeProgram({
        {GL_VERTEX_SHADER,   glx::LINE_VS},
        {GL_GEOMETRY_SHADER, glx::LINE_GS},
        {GL_FRAGMENT_SHADER, glx::LINE_FS}
    });

    ctx.solidProgram = makeProgram({
        {GL_VERTEX_SHADER,   glx::SOLID_VS},
        {GL_FRAGMENT_SHADER, glx::SOLID_FS}
    });

    ctx.phongProgram = makeProgram({
        {GL_VERTEX_SHADER,   glx::PHONG_VS},
        {GL_FRAGMENT_SHADER, glx::PHONG_FS}
    });

    ctx.shadowProgram = makeProgram({
        {GL_VERTEX_SHADER,   glx::SHADOW_VS},
        {GL_FRAGMENT_SHADER, glx::SHADOW_FS}
    });

    // === UNIFORMS ===
    ctx.bg_uTex = glGetUniformLocation(ctx.bgProgram, "uTex");

    // === MESH ===
    ctx.bg    = glx::createBackgroundQuad();
    ctx.ball  = glx::createSphere(8.f, 32, 32);
    ctx.axes  = glx::createAxes(210.0f);

    return ctx;
}

AppAssets loadAppAssets(ARRenderContext& renderCtx, const cv::Mat& firstFrame) {
    AppAssets assets;

    // 1. Texture Vidéo
    cv::Mat frameRGBA;
    cv::cvtColor(firstFrame, frameRGBA, cv::COLOR_BGR2RGBA);
    assets.bgTex = glx::createTextureRGBA(frameRGBA.cols, frameRGBA.rows);

    // 2. Configuration OpenGL
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.05f, 0.05f, 0.06f, 1.0f);

    // 3. Chargement des Textures (Balle, Sol, Ciel)
    assets.ballTex  = glx::loadTexture("../data/balle.png");
    assets.grassTex = glx::loadTexture("../data/sol.png");
    assets.skyTex   = glx::loadTexture("../data/ciel.jpeg", true); // true pour flip

    // 4. Compilation des Shaders additionnels
    assets.phongProgram = glx::link({
        glx::compile(GL_VERTEX_SHADER, glx::PHONG_VS), 
        glx::compile(GL_FRAGMENT_SHADER, glx::PHONG_FS)
    });
    
    // 5. Récupération des Uniforms
    assets.phongU.mvp        = glGetUniformLocation(assets.phongProgram, "uMVP");
    assets.phongU.model      = glGetUniformLocation(assets.phongProgram, "uModel");
    assets.phongU.viewPos    = glGetUniformLocation(assets.phongProgram, "uViewPos");
    assets.phongU.lightPos   = glGetUniformLocation(assets.phongProgram, "uLightPos");
    assets.phongU.lightColor = glGetUniformLocation(assets.phongProgram, "uLightColor");
    assets.phongU.tex        = glGetUniformLocation(assets.phongProgram, "uTex");

    // ... faites de même pour shadowU et lineU ...

    return assets;
}

void drawVRFloor(
    bool isVR,
    GLuint phongProgram,
    const glm::mat4& P, const glm::mat4& V, 
    const glm::vec3& camPos, const glm::vec3& lightPos,
    GLuint grassTexID,
    const glx::Mesh& floorMesh,
    GLint ph_uMVP, GLint ph_uModel, GLint ph_uViewPos, GLint ph_uLightPos, GLint ph_uLightColor, GLint ph_uTex
) {
    if (!isVR) return; // Si on n'est pas en VR, on sort immédiatement

    glUseProgram(phongProgram);
    
    // Calcul de la matrice du modèle (M_floor)
    glm::mat4 M_floor = glm::scale(glm::mat4(1.0f), glm::vec3(105.f, 148.5f, 1.f));
    
    // Envoi des matrices et paramètres d'éclairage
    glUniformMatrix4fv(ph_uMVP, 1, GL_FALSE, glm::value_ptr(P * V * M_floor));
    glUniformMatrix4fv(ph_uModel, 1, GL_FALSE, glm::value_ptr(M_floor));
    glUniform3fv(ph_uViewPos, 1, glm::value_ptr(camPos));
    glUniform3fv(ph_uLightPos, 1, glm::value_ptr(lightPos));
    glUniform3fv(ph_uLightColor, 1, glm::value_ptr(glm::vec3(2.0f)));

    // Gestion de la texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, grassTexID);
    glUniform1i(ph_uTex, 0);

    // Dessin du mesh
    glBindVertexArray(floorMesh.vao);
    glDrawArrays(GL_TRIANGLES, 0, floorMesh.count);
    glBindVertexArray(0); // Bonne pratique : délier le VAO
}
void drawBall(
    const glx::Mesh& ballMesh,
    GLuint ballTextureID,
    const glm::mat4& P, const glm::mat4& V,
    const glm::vec3& ballPos, const glm::mat4& ballRotationMatrix,
    const glm::vec3& camPos, const glm::vec3& lightPos,
    GLuint phongProgram,
    GLint ph_uMVP, GLint ph_uModel, GLint ph_uViewPos, GLint ph_uLightPos, GLint ph_uLightColor, GLint ph_uTex
) {
    glUseProgram(phongProgram);
    
    // Calcul de la matrice du modèle
    glm::mat4 M_ball = glm::translate(glm::mat4(1.0f), ballPos) * ballRotationMatrix;
    
    // Envoi des matrices au shader
    glUniformMatrix4fv(ph_uMVP, 1, GL_FALSE, glm::value_ptr(P * V * M_ball));
    glUniformMatrix4fv(ph_uModel, 1, GL_FALSE, glm::value_ptr(M_ball));
    
    // Paramètres d'éclairage et vue
    glUniform3fv(ph_uViewPos, 1, glm::value_ptr(camPos));
    glUniform3fv(ph_uLightPos, 1, glm::value_ptr(lightPos));
    glUniform3fv(ph_uLightColor, 1, glm::value_ptr(glm::vec3(2.0f)));

    // Gestion de la texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ballTextureID);
    glUniform1i(ph_uTex, 0);

    // Dessin de la sphère
    glBindVertexArray(ballMesh.vao);
    glDrawElements(GL_TRIANGLES, ballMesh.count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void drawCoordinateAxes(const ARRenderContext& ctx, const glm::mat4& P, const glm::mat4& V, GLint uMVP, GLint uColor) {
    glUseProgram(ctx.lineProgram);
    
    glm::mat4 MVP_axes = P * V;
    glUniformMatrix4fv(uMVP, 1, GL_FALSE, glm::value_ptr(MVP_axes));

    // Axe X — rouge
    glUniform3f(uColor, 1.0f, 0.0f, 0.0f);
    glBindVertexArray(ctx.axes.x.vao);
    glDrawArrays(GL_LINES, 0, ctx.axes.x.count);

    // Axe Y — vert
    glUniform3f(uColor, 0.0f, 1.0f, 0.0f);
    glBindVertexArray(ctx.axes.y.vao);
    glDrawArrays(GL_LINES, 0, ctx.axes.y.count);

    // Axe Z — bleu
    glUniform3f(uColor, 0.0f, 0.0f, 1.0f);
    glBindVertexArray(ctx.axes.z.vao);
    glDrawArrays(GL_LINES, 0, ctx.axes.z.count);

    glBindVertexArray(0);
}
void drawBallShadow(
    const glx::Mesh& ballMesh,
    const glm::mat4& P, const glm::mat4& V,
    const glm::vec3& ballPos, const glm::mat4& ballRotationMatrix,
    const glm::vec3& lightPos,
    GLuint shadowProgram,
    GLint sh_uMVP, GLint sh_uColor
) {
    // 1. Configuration du mélange pour la transparence
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(shadowProgram);

    // 2. Calcul de la matrice de projection sur le sol (Z=0)
    // Cette matrice "écrase" l'objet sur le plan Z=0 en fonction de la position de la lumière
    glm::mat4 shadowProj(1.0f);
    shadowProj[2][0] = -lightPos.x / lightPos.z;
    shadowProj[2][1] = -lightPos.y / lightPos.z;
    shadowProj[2][2] = 0.0f;

    // 3. Calcul de la matrice finale de l'ombre
    glm::mat4 M_ball_world = glm::translate(glm::mat4(1.0f), ballPos) * ballRotationMatrix;
    // On ajoute un léger décalage en Z (0.1f) pour éviter le "Z-fighting" (clignotement avec le sol)
    glm::mat4 M_shadow = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0.1f)) * shadowProj * M_ball_world;

    // 4. Envoi des uniforms
    glUniformMatrix4fv(sh_uMVP, 1, GL_FALSE, glm::value_ptr(P * V * M_shadow));
    glUniform4f(sh_uColor, 0.1f, 0.1f, 0.1f, 0.5f); // Noir à 50% d'opacité

    // 5. Dessin
    glBindVertexArray(ballMesh.vao);
    glDrawElements(GL_TRIANGLES, ballMesh.count, GL_UNSIGNED_INT, 0);
    
    // 6. Nettoyage de l'état
    glBindVertexArray(0);
    glDisable(GL_BLEND);
}

void setupLineStyle(GLuint lineProgram, GLint uViewport, GLint uThickness, int width, int height, float thickness) {
    glUseProgram(lineProgram);
    glUniform2f(uViewport, (float)width, (float)height);
    glUniform1f(uThickness, thickness);
}

void drawCoordinateAxes(
    const ARRenderContext& ctx, 
    const glm::mat4& P, const glm::mat4& V, 
    GLint uMVP, GLint uColor, GLint uViewport, GLint uThickness,
    int fbw, int fbh, float thickness
) {
    glEnable(GL_DEPTH_TEST);
    glUseProgram(ctx.lineProgram);

    // Configuration du style (Viewport et épaisseur)
    glUniform2f(uViewport, (float)fbw, (float)fbh);
    glUniform1f(uThickness, thickness);

    glm::mat4 MVP_axes = P * V;
    glUniformMatrix4fv(uMVP, 1, GL_FALSE, glm::value_ptr(MVP_axes));

    // Rendu des axes (X, Y, Z)
    glUniform3f(uColor, 1.f, 0.f, 0.f);
    glBindVertexArray(ctx.axes.x.vao);
    glDrawArrays(GL_LINES, 0, ctx.axes.x.count);

    glUniform3f(uColor, 0.f, 1.f, 0.f);
    glBindVertexArray(ctx.axes.y.vao);
    glDrawArrays(GL_LINES, 0, ctx.axes.y.count);

    glUniform3f(uColor, 0.f, 0.f, 1.f);
    glBindVertexArray(ctx.axes.z.vao);
    glDrawArrays(GL_LINES, 0, ctx.axes.z.count);

    glBindVertexArray(0);
}

void drawWalls(
    const glx::Mesh& wallsMesh, 
    const glx::Mesh& wallsWireframe,
    const glm::mat4& P, const glm::mat4& V,
    GLuint solidProgram,
    GLint solid_uMVP, GLint solid_uColor
) {
    glUseProgram(solidProgram);
    
    // Matrice identique pour les deux passes (solide et contours)
    glm::mat4 M_walls = glm::mat4(1.0f);
    glm::mat4 MVP_walls = P * V * M_walls;
    glUniformMatrix4fv(solid_uMVP, 1, GL_FALSE, glm::value_ptr(MVP_walls));

    // --- 1. DESSIN SOLIDE (MARRON) ---
    glUniform3f(solid_uColor, 0.6f, 0.3f, 0.2f);
    
    // PolygonOffset pour éviter le Z-fighting avec les lignes noires
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 1.0f);
    
    glBindVertexArray(wallsMesh.vao);
    glDrawElements(GL_TRIANGLES, wallsMesh.count, GL_UNSIGNED_INT, 0);
    glDisable(GL_POLYGON_OFFSET_FILL);

    // --- 2. DESSIN CONTOURS (NOIR) ---
    glUniform3f(solid_uColor, 0.0f, 0.0f, 0.0f);
    
    glBindVertexArray(wallsWireframe.vao);
    // On dessine des LIGNES (GL_LINES) pour le contour
    glDrawElements(GL_LINES, wallsWireframe.count, GL_UNSIGNED_INT, 0);
    
    glBindVertexArray(0);
};

void updateVideoBackground(GLuint& bgTex, const cv::Mat& frameBGR) {
    // 1. Préparation de l'image (Conversion + Flip pour OpenGL)
    cv::Mat frameRGBA;
    cv::cvtColor(frameBGR, frameRGBA, cv::COLOR_BGR2RGBA);
    cv::flip(frameRGBA, frameRGBA, 0);

    // 2. Gestion dynamique de la résolution
    // On utilise static pour se souvenir de la taille précédente entre deux appels
    static int currentTexW = 0;
    static int currentTexH = 0;

    if (frameRGBA.cols != currentTexW || frameRGBA.rows != currentTexH) {
        // Si la taille a changé, on libère l'ancienne texture et on recrée la nouvelle
        if (bgTex != 0) {
            glDeleteTextures(1, &bgTex);
        }
        bgTex = glx::createTextureRGBA(frameRGBA.cols, frameRGBA.rows);
        currentTexW = frameRGBA.cols;
        currentTexH = frameRGBA.rows;
    }

    // 3. Envoi des données à la carte graphique
    glx::updateTextureRGBA(bgTex, frameRGBA);
}

void drawBackground(const ARRenderContext& ctx, GLuint bgTex) {
    glDisable(GL_DEPTH_TEST); 
    glUseProgram(ctx.bgProgram);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bgTex);
    
    // On récupère l'emplacement de la texture dans le shader
    GLint uTex = glGetUniformLocation(ctx.bgProgram, "uTex");
    glUniform1i(uTex, 0);
    
    glBindVertexArray(ctx.bg.vao);
    glDrawArrays(GL_TRIANGLES, 0, ctx.bg.count);
    
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST); 
}