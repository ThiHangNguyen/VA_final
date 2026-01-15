#include <opencv2/opencv.hpp>      // OpenCV pour traitement d'image et capture vidéo
#include <GL/glew.h>               // GLEW pour charger les extensions OpenGL
#include <GLFW/glfw3.h>            // GLFW pour la gestion de la fenêtre et du contexte OpenGL
#include <glm/glm.hpp>             // GLM pour les opérations matricielles (maths 3D)
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ar/calib.hpp"           // Chargement des paramètres de calibration
#include "ar/pose.hpp"            // Projection / View OpenGL à partir de rvec/tvec
#include "ar/render.hpp"          // Création du contexte de rendu AR
#include "detect/a4.hpp"          // Détection des coins de la feuille A4
#include "glx/mesh.hpp"           // Création des maillages 3D
#include "glx/shaders.hpp"        // Compilation / linkage des shaders
#include "glx/texture.hpp"        // Gestion de la texture
#include "ar/physics.hpp"        // Gestion des collisions
#include "glx/cleanup.hpp"        // Nettoyage à la fin

#include "app/init.hpp"          // Initialisation OpenGL
#include "app/input.hpp"         // Parsing des arguments d'entrée
#include "app/game.hpp"          // Gestion des états de l'application

#include "game/maze.hpp"         // Génération du labyrinthe

#include <iostream>
#include <stdexcept>
#include <vector>

int runApp(int argc, char** argv) {
  try {

    bool paused = false;
    bool lastSpacePressed = false;
    bool lastEscPressed   = false;

    InputConfig cfg;
    if (!parseArgs(argc, argv, cfg)) {
        return -1;
    }
    cv::VideoCapture cap;
    if (!openVideoSource(cap, cfg)) return -1;

    // --- Chargement calibration ---
    const ar::Calibration calib = ar::loadCalibration(cfg.calibPath);
    // --- Lecture de la première frame ---
    cv::Mat frameBGR;
    if (!cap.read(frameBGR) || frameBGR.empty()) {
      std::cerr << "Erreur : première frame vide !\n";
      return -1;
    }
    int vw = frameBGR.cols, vh = frameBGR.rows;

    // --- Initialisation GLFW + fenêtre ---
    GLContext gl = initOpenGL("AR Ball", vw, vh);
    GLFWwindow* window = gl.window;


    // --- Shaders ---
    ARRenderContext renderCtx = createRenderContext();

    // ==========================================
    // CONFIG PHYSIQUE & DESIGN
    // ==========================================
    
    // 1. Physique (Speed & Bounce)
    float gameAccel = 2500.0f; // Valeur EARTH par défaut
    float gameBounce = 0.5f;   // Valeur EARTH par défaut

    if (cfg.speedMode == PhysicsMode::MOON) {
        gameAccel = 500.0f; // Gravité faible
        std::cout << "[PHYSICS] Mode MOON Speed (Lent)\n";
    }
    if (cfg.bounceMode == PhysicsMode::MOON) {
        gameBounce = 0.95f; // Rebond très fort
        std::cout << "[PHYSICS] Mode MOON Bounce (Elastique)\n";
    }
    glm::vec3 wallColor;  // variable pour la couleur du mur 

    if (cfg.designTheme == DesignTheme::DEFAULT) {
        // Marron bois
        wallColor = glm::vec3(0.55f, 0.27f, 0.07f); 
    }
    else if (cfg.designTheme == DesignTheme::SPACE) {
        // Bleu Nuit profond
        wallColor = glm::vec3(0.05f, 0.05f, 0.3f); 
    }
    else if (cfg.designTheme == DesignTheme::DESERT) {
        // Vert Martien (Alien)
        wallColor = glm::vec3(0.0f, 0.8f, 0.2f); 
        // OU si tu veux rouge martien : glm::vec3(0.8f, 0.3f, 0.1f);
    }
    // 2. Design (Choix du suffixe pour les fichiers)
    std::string suffix = "_1"; // Défaut
    if (cfg.designTheme == DesignTheme::SPACE) suffix = "_2";
    if (cfg.designTheme == DesignTheme::DESERT) suffix = "_3";
    std::cout << "[DESIGN] Chargement du theme " << suffix << "\n";
    // ==========================================

    // === MURS CADRE A4 (Assemblage "Menuisier") ===
    float WALL_HEIGHT = 40.f;
    float WALL_THICKNESS = 10.0f; 

    auto mazeWalls = glx::createMazeLayout(WALL_THICKNESS, cfg.difficulty);
    glx::Mesh wallsMesh = glx::createWalls(mazeWalls, WALL_HEIGHT, WALL_THICKNESS);
    glx::Mesh wallsWireframe = glx::createWallsWireframe(mazeWalls, WALL_HEIGHT, WALL_THICKNESS);

    // --- Texture pour la frame vidéo ---
    cv::Mat frameRGBA;
    cv::cvtColor(frameBGR, frameRGBA, cv::COLOR_BGR2RGBA);
    GLuint bgTex = glx::createTextureRGBA(frameRGBA.cols, frameRGBA.rows);


    cv::Mat rvec, tvec; // Rotation et translation
    // =========================
    // BALLE (état logique)
    // =========================
    glm::vec3 ballVel(0.f);             // Vitesse
    float ballRadius = 8.f; 

    // 2. Récupération des infos de la grille pour le placement
    auto dims = game::MazeGenerator::getDimensions(cfg.difficulty);
    int cols = dims.first;
    int rows = dims.second;

    // Dimensions physiques de la zone de jeu
    const float PLAY_W = 190.0f; 
    const float PLAY_H = 270.0f;

    float cellW = PLAY_W / cols;
    float cellH = PLAY_H / rows;

    // 3. Calcul du point de DÉPART (Bas Gauche)
    float startX = -PLAY_W / 2.0f + cellW / 2.0f;
    float startY = -PLAY_H / 2.0f + cellH / 2.0f;

    // 4. Initialisation de la balle (Maintenant ballRadius existe !)
    glm::vec3 ballPos(startX, startY, ballRadius); 
    
    // 5. Calcul du point d'ARRIVÉE (Haut Droite) pour plus tard
    float endX = -PLAY_W / 2.0f + (cols - 1) * cellW + cellW / 2.0f;
    float endY = -PLAY_H / 2.0f + (rows - 1) * cellH + cellH / 2.0f;
    glm::vec3 targetPos(endX, endY, 0.1f);

    // --- Uniforms pour les shaders ---
    GLint bg_uTex         = glGetUniformLocation(renderCtx.bgProgram,   "uTex");
    GLint line_uMVP       = glGetUniformLocation(renderCtx.lineProgram, "uMVP");
    GLint line_uColor     = glGetUniformLocation(renderCtx.lineProgram, "uColor");
    GLint line_uThickness = glGetUniformLocation(renderCtx.lineProgram, "uThicknessPx");
    GLint line_uViewport  = glGetUniformLocation(renderCtx.lineProgram, "uViewport");
    const float THICKNESS_PX = 3.0f;

    GLint solid_uMVP   = glGetUniformLocation(renderCtx.solidProgram, "uMVP");
    GLint solid_uColor = glGetUniformLocation(renderCtx.solidProgram, "uColor");

    // --- Coordonnées 3D de la feuille A4 ---
    std::vector<cv::Point3f> objectPts = detect::getA4ObjectPoints();

            // Rayon
    // 1. Charger le Shader de Texture et les ombres

    // Shader éclairage Phong (lumière + texture)
    GLuint phongProgram = glx::link({glx::compile(GL_VERTEX_SHADER, glx::PHONG_VS), glx::compile(GL_FRAGMENT_SHADER, glx::PHONG_FS)});
    GLint ph_uMVP = glGetUniformLocation(phongProgram, "uMVP");
    GLint ph_uModel = glGetUniformLocation(phongProgram, "uModel");
    GLint ph_uViewPos = glGetUniformLocation(phongProgram, "uViewPos");
    GLint ph_uLightPos = glGetUniformLocation(phongProgram, "uLightPos");
    GLint ph_uLightColor = glGetUniformLocation(phongProgram, "uLightColor");
    GLint ph_uTex = glGetUniformLocation(phongProgram, "uTex");

    // Shader ombres simples
    GLuint shadowProgram = glx::link({glx::compile(GL_VERTEX_SHADER, glx::SHADOW_VS), glx::compile(GL_FRAGMENT_SHADER, glx::SHADOW_FS)});
    GLint sh_uMVP = glGetUniformLocation(shadowProgram, "uMVP");
    GLint sh_uColor = glGetUniformLocation(shadowProgram, "uColor");

    // === CHARGEMENT ASSETS DYNAMIQUE ===
    // On construit le chemin du dossier : data/design_1, data/design_2, etc.
    std::string themePath = "../data/design_" + std::to_string((int)cfg.designTheme) + "/";
    std::cout << "[DESIGN] Chargement depuis : " << themePath << "\n";

    // 1. Balle
    cv::Mat ballImg = cv::imread(themePath + "balle.png");
    if(ballImg.empty()) { // Fallback si le dossier n'existe pas encore
        std::cerr << "WARN: Texture balle introuvable, chargement default.\n";
        ballImg = cv::imread("../data/design_1/balle.png");
    }
    if(!ballImg.empty()) cv::cvtColor(ballImg, ballImg, cv::COLOR_BGR2RGB);
    GLuint ballTextureID = glx::createTextureFromMat(ballImg.empty() ? frameRGBA : ballImg);

    // 2. Sol
    cv::Mat grassImg = cv::imread(themePath + "sol.png");
    if(grassImg.empty()) grassImg = cv::imread("../data/design_1/sol.png");
    if(!grassImg.empty()) cv::cvtColor(grassImg, grassImg, cv::COLOR_BGR2RGB);
    GLuint grassTexID = glx::createTextureFromMat(grassImg.empty() ? frameRGBA : grassImg);

    // 3. Ciel (Attention extension .jpg dans ta capture)
    cv::Mat skyImg = cv::imread(themePath + "ciel.jpg");
    if(skyImg.empty()) skyImg = cv::imread("../data/design_1/ciel.jpg");
    if(!skyImg.empty()) { 
        cv::cvtColor(skyImg, skyImg, cv::COLOR_BGR2RGB); 
        cv::flip(skyImg, skyImg, 0); 
    }
    GLuint skyTexID = glx::createTextureFromMat(skyImg.empty() ? frameRGBA : skyImg);
    // =========================
    
    // D. Mesh pour le sol en VR (Un simple rectangle)
    glx::Mesh floorMesh = glx::createBackgroundQuad();
    // =========================
    
    // Matrice qui stocke la rotation accumulée
    glm::mat4 ballRotationMatrix = glm::mat4(1.0f);
    double lastT = glfwGetTime();
    // Configuration Lumière (Soleil au milieu)
    glm::vec3 lightPos(0.0f, 0.0f, 200.0f);


      // --- GESTION AR / VR ---
      bool isVR = false;          // Par défaut on est en AR
      bool lastVPressed = false;  // Pour éviter que ça clignote si on reste appuyé

    // === BOUCLE PRINCIPALE ===
    while (!glfwWindowShouldClose(window)) {

      // =========================
      // INPUT CLAVIER (JEU)
      // =========================
      bool spacePressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
      if (spacePressed && !lastSpacePressed) {
          paused = !paused;
          std::cout << (paused ? "Game paused\n" : "Game resumed\n");
      }
      lastSpacePressed = spacePressed;

      bool escPressed = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
      if (escPressed && !lastEscPressed) {
          std::cout << "Return to menu\n";
          return 0;  //  SORTIE PROPRE DE runApp
      }
      lastEscPressed = escPressed;
      

      if (!cap.read(frameBGR) || frameBGR.empty()) break;

      std::vector<cv::Point2f> imagePts;
      bool okDetect = detect::detectA4Corners(frameBGR, imagePts);

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
          
          // On utilise les points potentiellement tournés pour que le tracking reste stable
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

      // MODIFICATION ICI : On ajoute "&& !paused" au début
      if (!paused && okDetect && !rvec.empty()) {
          // Appel avec les nouveaux paramètres
          ar::updatePhysics(rvec, dt, ballPos, ballVel, ballRotationMatrix, 
                      ballRadius, mazeWalls, WALL_THICKNESS,
                      gameAccel, gameBounce);
      }

      // -- Mise à jour de la texture de fond ---
      updateVideoBackground(bgTex, frameBGR);

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

      // --- Calcul des matrices ---
      glm::mat4 P = ar::projectionFromCV(calib.cameraMatrix, (float)fbw, (float)fbh, 0.1f, 2000.0f);
      glm::mat4 V = ar::viewFromRvecTvec(rvec, tvec);

      // Position caméra (extraction de la 4ème colonne de la matrice inverse de vue)
      glm::vec3 camPos = glm::vec3(glm::inverse(V)[3]);

      // --- 1. GESTION DU FOND (AR ou VR) ---
      glDisable(GL_DEPTH_TEST); // Le fond est derrière tout
      glUseProgram(renderCtx.bgProgram);
      glActiveTexture(GL_TEXTURE0);

      if (!isVR) {
          // === MODE AR : On dessine la webcam ===
          glBindTexture(GL_TEXTURE_2D, bgTex);
      } 
      else {
          // === MODE VR : On dessine le CIEL ===
          // (On affiche la texture Skybox)
          glBindTexture(GL_TEXTURE_2D, skyTexID); 
      }

      glUniform1i(bg_uTex, 0);
      glBindVertexArray(renderCtx.bg.vao);
      glDrawArrays(GL_TRIANGLES, 0, renderCtx.bg.count);
      glBindVertexArray(0);
      
      // On nettoie le depth buffer pour dessiner la 3D par dessus le fond
      glClear(GL_DEPTH_BUFFER_BIT); 
      glEnable(GL_DEPTH_TEST);
      
      // ==========================================
      // B. SOL PELOUSE (Uniquement en VR)
      // ==========================================
      if (isVR) {
          // --- Rendu du sol en mode VR ---
          drawVRFloor(isVR, phongProgram, P, V, camPos, lightPos, grassTexID, floorMesh, 
            ph_uMVP, ph_uModel, ph_uViewPos, ph_uLightPos, ph_uLightColor, ph_uTex);
      }


     // === MURS === (+ contours)
      drawWalls(wallsMesh, wallsWireframe, P, V, renderCtx.solidProgram, solid_uMVP, solid_uColor, wallColor);
      // === BALLE ===      
      // --- Rendu de l'Ombre ---
      drawBallShadow(renderCtx.ball, P, V, ballPos, ballRotationMatrix, lightPos, 
                    shadowProgram, sh_uMVP, sh_uColor);

      // ==========================================
      // 2. BALLE (Phong) - Eclairage Réaliste
      // ==========================================
      // --- Rendu de la Balle ---
      drawBall(renderCtx.ball, ballTextureID, P, V, ballPos, ballRotationMatrix, 
              camPos, lightPos, phongProgram, 
              ph_uMVP, ph_uModel, ph_uViewPos, ph_uLightPos, ph_uLightColor, ph_uTex);
      // === AXES ===
      drawCoordinateAxes(renderCtx, P, V, line_uMVP, line_uColor, line_uViewport, line_uThickness, fbw, fbh, THICKNESS_PX);
      
      if (paused) {
          drawText(10.0f, 30.0f, 1.0f, "PAUSED");
      }

      glfwSwapBuffers(window);
    }

    // --- Nettoyage des ressources ---
    glDeleteTextures(1, &grassTexID);
    glDeleteTextures(1, &skyTexID);
    glDeleteVertexArrays(1, &floorMesh.vao); glDeleteBuffers(1, &floorMesh.vbo);

    // --- Nettoyage OpenGL ---
    glx::cleanup(renderCtx.bgProgram, renderCtx.lineProgram, renderCtx.solidProgram, phongProgram, shadowProgram, bgTex, ballTextureID, renderCtx.bg, wallsMesh, renderCtx.ball, renderCtx.axes, window);

    return 0;
    
  } catch (const std::exception& e) {
    std::cerr << "Fatal: " << e.what() << std::endl;
    return -1;
  }
}


int main(int argc, char** argv) {

    AppConfig config;
    bool start = showMenuWindow(config);
    if (!start) {
        std::cout << "Quit from menu.\n";
        return 0;
    }

    // Lancement main AR
    return runApp(argc, argv);
}