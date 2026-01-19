/**
 * @file render.hpp
 * @brief Fonctions de rendu OpenGL pour l'application AR/VR.
 *
 * Contient toutes les fonctions de dessin: fond video, bille texturee,
 * murs du labyrinthe, ombres, axes de coordonnees, etc.
 * On utilise plusieurs shaders (phong, solid, line) selon l'objet.
 *
 * @author Thi Hang NGUYEN & Bichoy DAOUD
 */

#pragma once

#include <opencv2/core.hpp>
#include <GL/glew.h>
#include "glx/mesh.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include "glx/texture.hpp"
#include <glm/glm.hpp>

/**
 * @struct ARRenderContext
 * @brief Contexte de rendu contenant les shaders et meshes principaux.
 */
struct ARRenderContext {
    GLuint bgProgram;
    GLuint lineProgram;
    GLuint solidProgram;
    GLuint phongProgram;
    GLuint shadowProgram;

    GLint bg_uTex;

    // Mesh
    glx::Mesh bg;
    glx::Mesh ball;
    glx::Mesh walls;
    glx::Axes axes;
};

/**
 * @struct AppAssets
 * @brief Ressources de l'application (textures, uniforms caches).
 *
 * On stocke les locations des uniforms ici pour eviter de faire
 * glGetUniformLocation a chaque frame (optimisation).
 */
struct AppAssets {
    GLuint bgTex;      ///< Texture du fond video
    GLuint ballTex;    ///< Texture de la bille (terre, lune, etc)
    GLuint grassTex;   ///< Texture sol pour mode VR
    GLuint skyTex;     ///< Texture ciel (pas utilise pour l'instant)

    GLuint phongProgram;
    GLuint shadowProgram;

    /// Uniforms du shader Phong (eclairage)
    struct {
        GLint mvp, model, viewPos, lightPos, lightColor, tex;
    } phongU;

    /// Uniforms du shader d'ombre
    struct {
        GLint mvp, color;
    } shadowU;

    /// Uniforms du shader de lignes (axes)
    struct {
        GLint mvp, color, thickness, viewport;
    } lineU;
};

/**
 * @brief Cree le contexte de rendu (compile les shaders, cree les meshes).
 * @return ARRenderContext initialise
 */
ARRenderContext createRenderContext();

/**
 * @brief Charge les textures et recupere les locations des uniforms.
 * @param renderCtx Contexte de rendu
 * @param firstFrame Premiere frame video (pour dimensionner la texture)
 * @return AppAssets charge
 */
AppAssets loadAppAssets(ARRenderContext& renderCtx, const cv::Mat& firstFrame);

/**
 * @brief Dessine le sol en mode VR (plan texture herbe).
 */
void drawVRFloor(
    bool isVR,
    GLuint phongProgram,
    const glm::mat4& P, const glm::mat4& V, 
    const glm::vec3& camPos, const glm::vec3& lightPos,
    GLuint grassTexID,
    const glx::Mesh& floorMesh,
    GLint ph_uMVP, GLint ph_uModel, GLint ph_uViewPos, GLint ph_uLightPos, GLint ph_uLightColor, GLint ph_uTex
);
/**
 * @brief Dessine la bille avec eclairage Phong et texture.
 * La bille tourne visuellement grace a ballRotationMatrix.
 */
void drawBall(
    const glx::Mesh& ballMesh,
    GLuint ballTextureID,
    const glm::mat4& P, const glm::mat4& V,
    const glm::vec3& ballPos, const glm::mat4& ballRotationMatrix,
    const glm::vec3& camPos, const glm::vec3& lightPos,
    GLuint phongProgram,
    GLint ph_uMVP, GLint ph_uModel, GLint ph_uViewPos, GLint ph_uLightPos, GLint ph_uLightColor, GLint ph_uTex
);

/// @brief Dessine les axes X(rouge) Y(vert) Z(bleu) pour debug
void drawCoordinateAxes(const ARRenderContext& ctx, const glm::mat4& P, const glm::mat4& V, GLint uMVP, GLint uColor);

/**
 * @brief Dessine l'ombre de la bille sur le plan Z=0.
 * On projette la bille sur le sol avec une matrice de projection d'ombre.
 */
void drawBallShadow(
    const glx::Mesh& ballMesh,
    const glm::mat4& P, const glm::mat4& V,
    const glm::vec3& ballPos, const glm::mat4& ballRotationMatrix,
    const glm::vec3& lightPos,
    GLuint shadowProgram,
    GLint sh_uMVP, GLint sh_uColor
) ;

/// @brief Configure l'epaisseur des lignes pour le shader de lignes
void setupLineStyle(GLuint lineProgram, GLint uViewport, GLint uThickness, int width, int height, float thickness);

/// @brief Version avec parametre d'epaisseur des axes
void drawCoordinateAxes(
    const ARRenderContext& ctx,
    const glm::mat4& P, const glm::mat4& V,
    GLint uMVP, GLint uColor, GLint uViewport, GLint uThickness,
    int fbw, int fbh, float thickness
);

/**
 * @brief Dessine les murs du labyrinthe (faces pleines + contours).
 * @param wallsMesh Mesh des murs (faces)
 * @param wallsWireframe Mesh wireframe (contours noirs)
 * @param color Couleur des murs (ex: marron)
 */
void drawWalls(
    const glx::Mesh& wallsMesh,
    const glx::Mesh& wallsWireframe,
    const glm::mat4& P, const glm::mat4& V,
    GLuint solidProgram,
    GLint solid_uMVP, GLint solid_uColor,
    const glm::vec3& color
);

/// @brief Met a jour la texture de fond avec la frame video courante
void updateVideoBackground(GLuint& bgTex, const cv::Mat& frameBGR);

/// @brief Dessine le fond video en mode AR
void drawBackground(const ARRenderContext& ctx, GLuint bgTex);

/**
 * @brief Dessine le cercle vert de la zone d'arrivee.
 * @param targetPos Position du centre
 * @param radius Rayon du cercle
 */
void drawTargetCircle(
    const glm::vec3& targetPos,
    float radius,
    const glm::mat4& P, const glm::mat4& V,
    GLuint solidProgram,
    GLint solid_uMVP, GLint solid_uColor
);