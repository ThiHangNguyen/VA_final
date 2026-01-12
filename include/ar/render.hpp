#pragma once

#include <opencv2/core.hpp>
#include <GL/glew.h>
#include "glx/mesh.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include "glx/texture.hpp"

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

struct AppAssets {
    // Textures
    GLuint bgTex;
    GLuint ballTex;
    GLuint grassTex;
    GLuint skyTex;

    // Shaders additionnels (si pas déjà dans renderCtx)
    GLuint phongProgram;
    GLuint shadowProgram;

    // Locations des Uniforms (pour éviter les glGetUniformLocation dans la boucle)
    struct {
        GLint mvp, model, viewPos, lightPos, lightColor, tex;
    } phongU;

    struct {
        GLint mvp, color;
    } shadowU;

    struct {
        GLint mvp, color, thickness, viewport;
    } lineU;
};

ARRenderContext createRenderContext();
AppAssets loadAppAssets(ARRenderContext& renderCtx, const cv::Mat& firstFrame);
void drawVRFloor(
    bool isVR,
    GLuint phongProgram,
    const glm::mat4& P, const glm::mat4& V, 
    const glm::vec3& camPos, const glm::vec3& lightPos,
    GLuint grassTexID,
    const glx::Mesh& floorMesh,
    GLint ph_uMVP, GLint ph_uModel, GLint ph_uViewPos, GLint ph_uLightPos, GLint ph_uLightColor, GLint ph_uTex
);
void drawBall(
    const glx::Mesh& ballMesh,
    GLuint ballTextureID,
    const glm::mat4& P, const glm::mat4& V,
    const glm::vec3& ballPos, const glm::mat4& ballRotationMatrix,
    const glm::vec3& camPos, const glm::vec3& lightPos,
    GLuint phongProgram,
    GLint ph_uMVP, GLint ph_uModel, GLint ph_uViewPos, GLint ph_uLightPos, GLint ph_uLightColor, GLint ph_uTex
) ;
void drawCoordinateAxes(const ARRenderContext& ctx, const glm::mat4& P, const glm::mat4& V, GLint uMVP, GLint uColor);
void drawBallShadow(
    const glx::Mesh& ballMesh,
    const glm::mat4& P, const glm::mat4& V,
    const glm::vec3& ballPos, const glm::mat4& ballRotationMatrix,
    const glm::vec3& lightPos,
    GLuint shadowProgram,
    GLint sh_uMVP, GLint sh_uColor
) ;

void setupLineStyle(GLuint lineProgram, GLint uViewport, GLint uThickness, int width, int height, float thickness);
void drawCoordinateAxes(
    const ARRenderContext& ctx, 
    const glm::mat4& P, const glm::mat4& V, 
    GLint uMVP, GLint uColor, GLint uViewport, GLint uThickness,
    int fbw, int fbh, float thickness
);
void drawWalls(
    const glx::Mesh& wallsMesh, 
    const glx::Mesh& wallsWireframe,
    const glm::mat4& P, const glm::mat4& V,
    GLuint solidProgram,
    GLint solid_uMVP, GLint solid_uColor
) ;
void updateVideoBackground(GLuint& bgTex, const cv::Mat& frameBGR);