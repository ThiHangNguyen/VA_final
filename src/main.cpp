#include <opencv2/opencv.hpp>      // OpenCV pour traitement d'image et capture vidéo
#include <GL/glew.h>               // GLEW pour charger les extensions OpenGL
#include <GLFW/glfw3.h>            // GLFW pour la gestion de la fenêtre et du contexte OpenGL
#include <glm/glm.hpp>             // GLM pour les opérations matricielles (maths 3D)
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ar/calib.hpp"           // Chargement des paramètres de calibration
#include "ar/pose.hpp"            // Projection / View OpenGL à partir de rvec/tvec
#include "detect/a4.hpp"          // Détection des coins de la feuille A4
#include "glx/mesh.hpp"           // Création des maillages 3D
#include "glx/shaders.hpp"        // Compilation / linkage des shaders
#include "glx/texture.hpp"        // Gestion de la texture
#include "ar/physics.hpp"        // Gestion des collisions
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

    GLuint solidVS = glx::compile(GL_VERTEX_SHADER, glx::SOLID_VS);
    GLuint solidFS = glx::compile(GL_FRAGMENT_SHADER, glx::SOLID_FS);
    GLuint solidProgram = glx::link({solidVS, solidFS});
    glDeleteShader(solidVS);
    glDeleteShader(solidFS);

    // --- Meshes (quad fond, cube, axes) ---
    glx::Mesh bg   = glx::createBackgroundQuad();
    // glx::Mesh cube = glx::createCubeWireframe(30.0f);
    glx::Axes axes = glx::createAxes(210.0f);
    // petite croix pour représenter la balle
    glx::Axes ballAxes = glx::createAxes(10.f);

    // === murs sur les bords A4 ===
    std::vector<std::array<float,4>> wallSegments = {
        // bas
        {-105.f, -148.5f, +105.f, -148.5f},
        // droite
        {+105.f, -148.5f, +105.f, +148.5f},
        // haut
        {+105.f, +148.5f, -105.f, +148.5f},
        // gauche
        {-105.f, +148.5f, -105.f, -148.5f}
    };

    // hauteur du mur = 40 mm par exemple
    float WALL_HEIGHT = 40.f;

    // création d’un seul mesh contenant tous les murs
    glx::Mesh wallsMesh = glx::createWalls(wallSegments, WALL_HEIGHT);

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

    GLint solid_uMVP   = glGetUniformLocation(solidProgram, "uMVP");
    GLint solid_uColor = glGetUniformLocation(solidProgram, "uColor");

    // --- Coordonnées 3D de la feuille A4 ---
    const float W = 210.f, H = 297.f;
    std::vector<cv::Point3f> objectPts = {
      {-W*0.5f, -H*0.5f, 0.0f},
      {+W*0.5f, -H*0.5f, 0.0f},
      {+W*0.5f, +H*0.5f, 0.0f},
      {-W*0.5f, +H*0.5f, 0.0f}
    };

    cv::Mat rvec, tvec; // Rotation et translation
    // =========================
    // BALLE (état logique)
    // =========================
    glm::vec3 ballPos(0.f, 0.f, 8.f);   // Position initiale
    glm::vec3 ballVel(0.f);             // Vitesse
    float ballRadius = 8.f;             // Rayon
    // 1. Charger le Shader de Texture
    GLuint texObjVS = glx::compile(GL_VERTEX_SHADER, glx::TEX_OBJ_VS);
    GLuint texObjFS = glx::compile(GL_FRAGMENT_SHADER, glx::TEX_OBJ_FS);
    GLuint texObjProgram = glx::link({texObjVS, texObjFS});
    GLint texObj_uMVP = glGetUniformLocation(texObjProgram, "uMVP");
    GLint texObj_uTex = glGetUniformLocation(texObjProgram, "uTex");

    // 2. Créer la sphère
    glx::Mesh ballMesh = glx::createSphere(ballRadius, 32, 32);

    // 3. Charger l'image de la balle
    cv::Mat ballImg = cv::imread("../data/balle.png"); 
    if (ballImg.empty()) std::cout << "ERREUR: Image balle introuvable !" << std::endl;
    else cv::cvtColor(ballImg, ballImg, cv::COLOR_BGR2RGB); // BGR -> RGB
    GLuint ballTextureID = glx::createTextureFromMat(ballImg);

    // Matrice qui stocke la rotation accumulée
    glm::mat4 ballRotationMatrix = glm::mat4(1.0f);
    double lastT = glfwGetTime();
    // === BOUCLE PRINCIPALE ===
    while (!glfwWindowShouldClose(window)) {
      if (!cap.read(frameBGR) || frameBGR.empty()) break;

      std::vector<cv::Point2f> imagePts;
      bool okDetect = detect::detectA4Corners(frameBGR, imagePts);

        // --- GESTION AR / VR ---
      bool isVR = false;          // Par défaut on est en AR
      bool lastVPressed = false;  // Pour éviter que ça clignote si on reste appuyé

      if (!okDetect) {
          // AFFICHER LE MESSAGE SI PAS DE DETECTION
          std::string msg = "Pas de A4 detecte ! Placez la feuille...";
          int baseline = 0;
          cv::Size textSize = cv::getTextSize(msg, cv::FONT_HERSHEY_SIMPLEX, 1.0, 2, &baseline);
          
          // Centrer le texte
          cv::Point textOrg((frameBGR.cols - textSize.width) / 2, (frameBGR.rows + textSize.height) / 2);
          
          // Fond noir semi-transparent pour lisibilité
          cv::rectangle(frameBGR, textOrg + cv::Point(0, baseline), textOrg + cv::Point(textSize.width, -textSize.height), cv::Scalar(0,0,0), -1);
          // Texte blanc
          cv::putText(frameBGR, msg, textOrg, cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 255), 2);
      } else {
          // Si détecté, on calcule la pose
          cv::solvePnP(objectPts, imagePts, calib.cameraMatrix, calib.distCoeffs,
                      rvec, tvec, !rvec.empty(), cv::SOLVEPNP_ITERATIVE);
      }
      // =========================
      // PHYSIQUE BALLE
      // =========================
      double nowT = glfwGetTime();
      float dt = float(nowT - lastT);
      lastT = nowT;
      if (dt > 0.05f) dt = 0.05f;

      if (okDetect && !rvec.empty()) {

          cv::Mat Rcv;
          cv::Rodrigues(rvec, Rcv);

          // Axes feuille (repère caméra)
          glm::vec3 X(
              Rcv.at<double>(0,0),
              Rcv.at<double>(1,0),
              Rcv.at<double>(2,0)
          );
          glm::vec3 Y(
              Rcv.at<double>(0,1),
              Rcv.at<double>(1,1),
              Rcv.at<double>(2,1)
          );
          glm::vec3 N(
              Rcv.at<double>(0,2),
              Rcv.at<double>(1,2),
              Rcv.at<double>(2,2)
          );

          // Normalisation OBLIGATOIRE
          X = glm::normalize(X);
          Y = glm::normalize(Y);
          N = glm::normalize(N);

          // Gravité monde (choix arbitraire mais stable)
          // OpenCV : Y vers le bas
          glm::vec3 gCam(0.f, 1.f, 0.f);
          // Projection de la gravité sur le plan
          glm::vec3 gPlane = gCam - glm::dot(gCam, N) * N;

          // Accélération dans le plan (repère feuille)
          float ax = glm::dot(gPlane, X);
          float ay = glm::dot(gPlane, Y);

          float accel = 2000.f;   // plus réaliste
          float damping = 1.f;

          ballVel += accel * glm::vec3(ax, ay, 0.f) * dt;
          ballVel *= 1.f / (1.f + damping * dt);
          glm::vec3 deplacement = ballVel * dt;
          ballPos += deplacement;
          float dist = glm::length(deplacement);
          
          if (dist > 0.0001f) {
              // Axe de rotation = Mouvement X Verticale
              glm::vec3 axis = glm::cross(deplacement, glm::vec3(0,0,1));
              axis = glm::normalize(axis);
              float angle = dist / ballRadius; // Angle = dist / rayon
              // On accumule la rotation
              ballRotationMatrix = glm::rotate(glm::mat4(1.0f), angle, axis) * ballRotationMatrix;
          }

          // Limites feuille A4
          float minX = -105.f + ballRadius;
          float maxX =  105.f - ballRadius;
          float minY = -148.5f + ballRadius;
          float maxY =  148.5f - ballRadius;

          // Collisions
          if (ballPos.x < minX) { ballPos.x = minX; ballVel.x *= -0.5f; }
          if (ballPos.x > maxX) { ballPos.x = maxX; ballVel.x *= -0.5f; }
          if (ballPos.y < minY) { ballPos.y = minY; ballVel.y *= -0.5f; }
          if (ballPos.y > maxY) { ballPos.y = maxY; ballVel.y *= -0.5f; }

          ballPos.z = ballRadius;
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

      // --- Gestion Touche 'V' (Toggle AR/VR) ---
      bool currentVPressed = (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS);
      if (currentVPressed && !lastVPressed) {
          isVR = !isVR; // On inverse le mode (AR -> VR ou VR -> AR)
          std::cout << "Mode change: " << (isVR ? "VR" : "AR") << std::endl;
      }
      lastVPressed = currentVPressed;

      int fbw, fbh;
      glfwGetFramebufferSize(window, &fbw, &fbh);
      glViewport(0, 0, fbw, fbh);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // --- 1. GESTION DU FOND (AR ou VR) ---
      
      if (!isVR) {
          // === MODE AR : On dessine la webcam ===
          glDisable(GL_DEPTH_TEST);
          glUseProgram(bgProgram);
          glActiveTexture(GL_TEXTURE0);
          glBindTexture(GL_TEXTURE_2D, bgTex);
          glUniform1i(bg_uTex, 0);
          glBindVertexArray(bg.vao);
          glDrawArrays(GL_TRIANGLES, 0, bg.count);
          glBindVertexArray(0);
          
          // On nettoie le depth buffer pour dessiner la 3D par dessus
          glClear(GL_DEPTH_BUFFER_BIT); 
      } 
      else {
          // === MODE VR : Fond virtuel ===
          // Pour l'instant un fond gris foncé "propre" (Critère Excellent: Skybox)
          // On change la couleur de fond juste pour ce mode
          glClearColor(0.2f, 0.2f, 0.2f, 1.0f); 
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      }

      // --- 2. Cube et axes ---
      glEnable(GL_DEPTH_TEST);
      glm::mat4 P = ar::projectionFromCV(calib.cameraMatrix, (float)fbw, (float)fbh, 0.1f, 2000.0f);
      glm::mat4 V = ar::viewFromRvecTvec(rvec, tvec);
      glm::mat4 M_axes = glm::mat4(1.0f);
      glm::mat4 M_cube = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 0.f, 30.f));

      glUseProgram(lineProgram);
      glUniform2f(line_uViewport, (float)fbw, (float)fbh);
      glUniform1f(line_uThickness, THICKNESS_PX);

      // === MURS ===
      glUseProgram(solidProgram);

      glm::mat4 MVP_walls = P * V;
      glUniformMatrix4fv(solid_uMVP, 1, GL_FALSE, glm::value_ptr(MVP_walls));
      glUniform3f(solid_uColor, 0.2f, 1.0f, 1.0f);

      glBindVertexArray(wallsMesh.vao);
      glDrawElements(GL_TRIANGLES, wallsMesh.count, GL_UNSIGNED_INT, 0);
      glBindVertexArray(0);

      
      glUseProgram(lineProgram);
      glUniform2f(line_uViewport, (float)fbw, (float)fbh);
      glUniform1f(line_uThickness, THICKNESS_PX);
    // === BALLE ===      
      glUseProgram(texObjProgram); // Utiliser le shader de texture
      
      // Matrice finale = Projection * Vue * Translation * Rotation
      glm::mat4 M_ball = glm::translate(glm::mat4(1.f), ballPos) * ballRotationMatrix;
      glm::mat4 MVP_ball = P * V * M_ball;
      
      glUniformMatrix4fv(texObj_uMVP, 1, GL_FALSE, glm::value_ptr(MVP_ball));
      
      // Activer la texture
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, ballTextureID);
      glUniform1i(texObj_uTex, 0);

      // Dessiner la sphère
      glBindVertexArray(ballMesh.vao);
      glDrawElements(GL_TRIANGLES, ballMesh.count, GL_UNSIGNED_INT, 0);
      glBindVertexArray(0);
      
      // === AXES ===
      glUseProgram(lineProgram);
      glm::mat4 MVP_axes = P * V;
      glUniformMatrix4fv(line_uMVP, 1, GL_FALSE, glm::value_ptr(MVP_axes));

      // Axe X — rouge
      glUniform3f(line_uColor, 1.f, 0.f, 0.f);
      glBindVertexArray(axes.x.vao);
      glDrawArrays(GL_LINES, 0, axes.x.count);

      // Axe Y — vert
      glUniform3f(line_uColor, 0.f, 1.f, 0.f);
      glBindVertexArray(axes.y.vao);
      glDrawArrays(GL_LINES, 0, axes.y.count);

      // Axe Z — bleu
      glUniform3f(line_uColor, 0.f, 0.f, 1.f);
      glBindVertexArray(axes.z.vao);
      glDrawArrays(GL_LINES, 0, axes.z.count);

      glBindVertexArray(0);

      // glm::mat4 MVP_cube = P * V * M_cube;
      // glUniformMatrix4fv(line_uMVP, 1, GL_FALSE, glm::value_ptr(MVP_cube));
      // glUniform3f(line_uColor, 0.f, 0.f, 0.f);
      // glBindVertexArray(cube.vao);
      // glDrawElements(GL_LINES, cube.count, GL_UNSIGNED_INT, 0);
      // glBindVertexArray(0);
      
      glfwSwapBuffers(window);
    }

    // --- Nettoyage OpenGL ---
    glx::cleanup(bgProgram, lineProgram, solidProgram, bgTex, bg, wallsMesh, axes, window);
    return 0;

  } catch (const std::exception& e) {
    std::cerr << "Fatal: " << e.what() << std::endl;
    return -1;
  }
}
