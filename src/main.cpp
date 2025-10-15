#include <opencv2/opencv.hpp>      // OpenCV pour traitement d'image et capture vidéo
#include <GL/glew.h>               // GLEW pour charger les extensions OpenGL
#include <GLFW/glfw3.h>            // GLFW pour la gestion de la fenêtre et du contexte OpenGL
#include <glm/glm.hpp>             // GLM pour les opérations matricielles (maths 3D)
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ar/calib.hpp"            // Chargement des paramètres de calibration
#include "ar/pose.hpp"             // Projection / View OpenGL à partir de rvec/tvec
#include "detect/a4.hpp"          // Détection des coins de la feuille A4
#include "glx/mesh.hpp"           // Création des maillages 3D
#include "glx/shaders.hpp"        // Compilation / linkage des shaders
#include "glx/texture.hpp"        // Gestion de la texture
#include "glx/cleanup.hpp"        // Nettoyage à la fin

#include <iostream>
#include <stdexcept>
#include <vector>

int main(int argc, char** argv) {
  try {
    // --- Lecture des arguments : choix entre webcam ou vidéo ---
    std::string calibPath;
    cv::VideoCapture cap;
    bool useWebcam = (argc > 1 && std::string(argv[1]) == "--webcam");

    std::string videoPath = "../data/Video_AR_1.mp4";       // Par défaut : chemin de la vidéo
    calibPath = "../data/camera.yaml";                      // Par défaut : calibration

    // --- Interprétation des arguments ---
    if (argc > 1) {
        std::string arg1 = argv[1];
        if (arg1 == "--webcam") {
            useWebcam = true;
            calibPath = "../data/camera_webcam.yaml";
        } else if (arg1 == "--video") {
            if (argc < 4) {
                std::cerr << "Usage: ./AR_A4_Video --video <video_path> <calibration_path>\n";
                return -1;
            }
            videoPath = argv[2];
            calibPath = argv[3];
        } else {
            std::cerr << "Argument inconnu : " << arg1 << "\n";
            std::cerr << "Utilisation :\n";
            std::cerr << "  ./AR_A4_Video --webcam\n";
            std::cerr << "  ./AR_A4_Video --video <video_path> <calibration_path>\n";
            return -1;
        }
    }

    // --- Ouverture de la source vidéo ---
    if (useWebcam) {
        int camIndex = 0;
        int reqW = 1280, reqH = 720, reqFPS = 30;

        if (!cap.open(camIndex, cv::CAP_V4L2)) {
            std::cerr << "Erreur : webcam non accessible !\n";
            return -1;
        }

        // Premier essai avec MJPEG
        cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M','J','P','G'));
        cap.set(cv::CAP_PROP_FRAME_WIDTH,  reqW);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, reqH);
        cap.set(cv::CAP_PROP_FPS,          reqFPS);

        // Si échec, fallback YUYV
        if ((int)cap.get(cv::CAP_PROP_FRAME_WIDTH) != reqW ||
            (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT) != reqH ||
            (int)std::round(cap.get(cv::CAP_PROP_FPS)) != reqFPS) {
            cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('Y','U','Y','V'));
            cap.set(cv::CAP_PROP_FRAME_WIDTH,  reqW);
            cap.set(cv::CAP_PROP_FRAME_HEIGHT, reqH);
            cap.set(cv::CAP_PROP_FPS,          reqFPS);
        }

        std::cout << "[INFO] Webcam ouverte => "
                  << (int)cap.get(cv::CAP_PROP_FRAME_WIDTH) << "x"
                  << (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT) << " @ "
                  << (int)cap.get(cv::CAP_PROP_FPS) << " FPS\n";
    } else {
        if (!cap.open(videoPath)) {
            std::cerr << "Erreur : impossible d’ouvrir la vidéo : " << videoPath << "\n";
            return -1;
        }
    }

    // --- Chargement calibration ---
    const ar::Calibration calib = ar::loadCalibration(calibPath);

    // --- Lecture de la première frame ---
    cv::Mat frameBGR;
    if (!cap.read(frameBGR) || frameBGR.empty()) {
      std::cerr << "Erreur : première frame vide !\n";
      return -1;
    }
    int vw = frameBGR.cols, vh = frameBGR.rows;

    // --- Initialisation GLFW + fenêtre ---
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  #endif
    GLFWwindow* window = glfwCreateWindow(vw, vh, "ARCube", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window); glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { std::cerr << "GLEW init failed\n"; return -1; }
    glGetError(); // Ignore l'erreur générée par glewInit

    // --- Shaders ---
    GLuint bgVS = glx::compile(GL_VERTEX_SHADER,   glx::BG_VS);
    GLuint bgFS = glx::compile(GL_FRAGMENT_SHADER, glx::BG_FS);
    GLuint bgProgram = glx::link({ bgVS, bgFS });
    glDeleteShader(bgVS); glDeleteShader(bgFS);

    GLuint lineVS = glx::compile(GL_VERTEX_SHADER,   glx::LINE_VS);
    GLuint lineGS = glx::compile(GL_GEOMETRY_SHADER, glx::LINE_GS);
    GLuint lineFS = glx::compile(GL_FRAGMENT_SHADER, glx::LINE_FS);
    GLuint lineProgram = glx::link({ lineVS, lineGS, lineFS });
    glDeleteShader(lineVS); glDeleteShader(lineGS); glDeleteShader(lineFS);

    // --- Meshes (quad fond, cube, axes) ---
    glx::Mesh bg   = glx::createBackgroundQuad();
    glx::Mesh cube = glx::createCubeWireframe(30.0f);
    glx::Axes axes = glx::createAxes(210.0f);

    // --- Texture pour la frame vidéo ---
    cv::Mat frameRGBA;
    cv::cvtColor(frameBGR, frameRGBA, cv::COLOR_BGR2RGBA);
    GLuint bgTex = glx::createTextureRGBA(frameRGBA.cols, frameRGBA.rows);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.05f, 0.05f, 0.06f, 1.0f);

    // --- Uniforms pour les shaders ---
    GLint bg_uTex         = glGetUniformLocation(bgProgram,   "uTex");
    GLint line_uMVP       = glGetUniformLocation(lineProgram, "uMVP");
    GLint line_uColor     = glGetUniformLocation(lineProgram, "uColor");
    GLint line_uThickness = glGetUniformLocation(lineProgram, "uThicknessPx");
    GLint line_uViewport  = glGetUniformLocation(lineProgram, "uViewport");
    const float THICKNESS_PX = 3.0f;

    // --- Coordonnées 3D de la feuille A4 ---
    const float W = 210.f, H = 297.f;
    std::vector<cv::Point3f> objectPts = {
      {-W*0.5f, -H*0.5f, 0.0f},
      {+W*0.5f, -H*0.5f, 0.0f},
      {+W*0.5f, +H*0.5f, 0.0f},
      {-W*0.5f, +H*0.5f, 0.0f}
    };

    cv::Mat rvec, tvec; // Rotation et translation

    // === BOUCLE PRINCIPALE ===
    while (!glfwWindowShouldClose(window)) {
      if (!cap.read(frameBGR) || frameBGR.empty()) break;

      std::vector<cv::Point2f> imagePts;
      bool okDetect = detect::detectA4Corners(frameBGR, imagePts);

      if (okDetect) {
        cv::solvePnP(objectPts, imagePts, calib.cameraMatrix, calib.distCoeffs,
                     rvec, tvec, !rvec.empty(), cv::SOLVEPNP_ITERATIVE);
      }

      // Conversion + flip (OpenGL en bas à gauche)
      cv::cvtColor(frameBGR, frameRGBA, cv::COLOR_BGR2RGBA);
      cv::flip(frameRGBA, frameRGBA, 0);

      // Resize si résolution change (webcam)
      static int texW = frameRGBA.cols, texH = frameRGBA.rows;
      if (frameRGBA.cols != texW || frameRGBA.rows != texH) {
        glDeleteTextures(1, &bgTex);
        bgTex = glx::createTextureRGBA(frameRGBA.cols, frameRGBA.rows);
        texW = frameRGBA.cols; texH = frameRGBA.rows;
      }
      glx::updateTextureRGBA(bgTex, frameRGBA);

      // === RENDU OPENGL ===
      glfwPollEvents();
      int fbw, fbh;
      glfwGetFramebufferSize(window, &fbw, &fbh);
      glViewport(0, 0, fbw, fbh);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // --- 1. Fond vidéo ---
      glDisable(GL_DEPTH_TEST);
      glUseProgram(bgProgram);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, bgTex);
      glUniform1i(bg_uTex, 0);
      glBindVertexArray(bg.vao);
      glDrawArrays(GL_TRIANGLES, 0, bg.count);
      glBindVertexArray(0);

      // --- 2. Cube et axes ---
      glEnable(GL_DEPTH_TEST);
      glm::mat4 P = ar::projectionFromCV(calib.cameraMatrix, (float)fbw, (float)fbh, 0.1f, 2000.0f);
      glm::mat4 V = ar::viewFromRvecTvec(rvec, tvec);
      glm::mat4 M_axes = glm::mat4(1.0f);
      glm::mat4 M_cube = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 0.f, 30.f));

      glUseProgram(lineProgram);
      glUniform2f(line_uViewport, (float)fbw, (float)fbh);
      glUniform1f(line_uThickness, THICKNESS_PX);

      glm::mat4 MVP_axes = P * V * M_axes;
      glUniformMatrix4fv(line_uMVP, 1, GL_FALSE, glm::value_ptr(MVP_axes));
      glBindVertexArray(axes.x.vao); glUniform3f(line_uColor, 1.f, 0.f, 0.f); glDrawArrays(GL_LINES, 0, axes.x.count);
      glBindVertexArray(axes.y.vao); glUniform3f(line_uColor, 0.f, 1.f, 0.f); glDrawArrays(GL_LINES, 0, axes.y.count);
      glBindVertexArray(axes.z.vao); glUniform3f(line_uColor, 0.f, 0.f, 1.f); glDrawArrays(GL_LINES, 0, axes.z.count);
      glBindVertexArray(0);

      glm::mat4 MVP_cube = P * V * M_cube;
      glUniformMatrix4fv(line_uMVP, 1, GL_FALSE, glm::value_ptr(MVP_cube));
      glUniform3f(line_uColor, 0.f, 0.f, 0.f);
      glBindVertexArray(cube.vao);
      glDrawElements(GL_LINES, cube.count, GL_UNSIGNED_INT, 0);
      glBindVertexArray(0);

      glfwSwapBuffers(window);
    }

    // --- Nettoyage OpenGL ---
    glx::cleanup(bgProgram, lineProgram, bgTex, bg, cube, axes, window);
    return 0;

  } catch (const std::exception& e) {
    std::cerr << "Fatal: " << e.what() << std::endl;
    return -1;
  }
}
