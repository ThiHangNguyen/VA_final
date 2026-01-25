/**
 * @file main.cpp
 * @brief Point d'entrée principal du jeu de labyrinthe en réalité augmentée.
 *
 * Ce fichier contient la boucle de jeu principale qui orchestre :
 * - La capture vidéo (webcam ou fichier)
 * - La détection de la feuille A4 (marqueur AR)
 * - Le calcul de la pose 3D (solvePnP)
 * - La simulation physique de la bille
 * - Le rendu OpenGL (murs, balle, ombres)
 * - L'affichage du HUD (FPS, temps, pause)
 *
 * Architecture du programme :
 * ```
 * main()
 *   └── showMenuWindow()     // Menu de sélection (difficulté, thème)
 *         └── runApp()       // Boucle de jeu principale
 *               ├── Détection A4 (detect::detectA4Corners)
 *               ├── Estimation pose (cv::solvePnP)
 *               ├── Physique (ar::updatePhysics)
 *               └── Rendu OpenGL (drawWalls, drawBall, etc.)
 * ```
 *
 * @author Thi Hang NGUYEN & Bichoy DAOUD
 * @date 2026
 */

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <opencv2/opencv.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <thread>
#include <chrono>

#include "ar/calib.hpp"           // Chargement des paramètres de calibration
#include "ar/pose.hpp"            // Projection / View OpenGL à partir de rvec/tvec
#include "ar/render.hpp"          // Création du contexte de rendu AR
#include "ar/filter.hpp"          // Filtrage EMA pour anti-jitter
#include "detect/a4.hpp"          // Détection des coins de la feuille A4
#include "glx/mesh.hpp"           // Création des maillages 3D
#include "glx/shaders.hpp"        // Compilation / linkage des shaders
#include "glx/texture.hpp"        // Gestion de la texture
#include "ar/physics.hpp"         // Gestion des collisions
#include "glx/cleanup.hpp"        // Nettoyage à la fin

#include "app/init.hpp"           // Initialisation OpenGL
#include "app/input.hpp"          // Parsing des arguments d'entrée
#include "app/game.hpp"           // Gestion des états de l'application
#include "app/score.hpp"          // Affichage de l'écran de score
#include "app/assets.hpp"         // Chargement des textures par thème
#include "app/hud.hpp"            // Affichage HUD (pause, FPS, temps)

#include "game/maze.hpp"          // Génération du labyrinthe

#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>

/**
 * @brief Exécute une partie de jeu complète.
 *
 * Cette fonction initialise tous les composants nécessaires (vidéo, OpenGL,
 * physique, labyrinthe) puis lance la boucle de jeu. Elle retourne quand
 * le joueur gagne, quitte, ou qu'une erreur survient.
 *
 * @param argc Nombre d'arguments de la ligne de commande.
 * @param argv Arguments de la ligne de commande.
 * @param config Configuration du jeu (difficulté, thème, physique).
 * @return GameResult Résultat de la partie (WIN, QUIT, ERROR) avec le temps.
 */
GameResult runApp(int argc, char** argv, AppConfig& config) {
    GameResult result= { EndReason::QUIT, 0.0, config.difficulty };
    bool gameFinished = false;
    try {
        bool stepByStep = true;
        bool paused = false;
        bool lastSpacePressed = false;
        bool lastEscPressed   = false;

        // Toggles affichage (F = FPS, T = Temps)
        bool showFPS = true;  // FPS affichés par défaut
        bool showTime = true; // Temps affiché par défaut
        bool lastFPressed = false;
        bool lastTPressed = false;

        // Pour calcul FPS
        double fpsTimer = 0.0;
        int frameCount = 0;
        float currentFPS = 0.0f;

        InputConfig cfg;
        if (!parseArgs(argc, argv, cfg)) {
            return GameResult{
                    EndReason::ERROR,
                    0.0,
                    config.difficulty
                };
        }

        // Appliquer les paramètres du menu aux configs CLI
        cfg.difficulty = config.difficulty;
        cfg.speedMode = config.speedMode;
        cfg.bounceMode = config.bounceMode;
        cfg.designTheme = config.designTheme;
        cv::VideoCapture cap;
        if (!openVideoSource(cap, cfg)) 
            return GameResult{
                EndReason::ERROR,
                0.0,
                config.difficulty
            };
        // Filtrage EMA pour stabiliser la pose (anti-jitter)
        // Alpha = 0.4 : réactif mais lisse
        ar::LowPassFilter3D rvecFilter(0.4f);
        ar::LowPassFilter3D tvecFilter(0.4f);

        // --- Chargement calibration ---
        const ar::Calibration calib = ar::loadCalibration(cfg.calibPath);
        // --- Lecture de la première frame ---
        cv::Mat frameBGR;
        if (!cap.read(frameBGR) || frameBGR.empty()) {
        std::cerr << "Erreur : première frame vide !\n";
            return GameResult{
                EndReason::ERROR,
                0.0,
                config.difficulty
            };
        }
        int vw = frameBGR.cols, vh = frameBGR.rows;

        // --- Initialisation GLFW + fenêtre ---
        GLContext gl = initOpenGL("Maze game", vw, vh);
        GLFWwindow* window = gl.window;


        // --- Shaders ---
        ARRenderContext renderCtx = createRenderContext();

        // ==========================================
        // CONFIG PHYSIQUE & DESIGN
        // ==========================================
        
        // 1. Physique (Speed & Bounce)
        float gameAccel = 3000.0f; // Valeur EARTH par défaut
        float gameBounce = 0.95f;   // Valeur EARTH par défaut

        if (cfg.speedMode == PhysicsMode::MOON) {
            gameAccel = 4000.0f; // Gravité faible
            std::cout << "[PHYSICS] Mode MOON Speed (Lent)\n";
        }
        if (cfg.bounceMode == PhysicsMode::MOON) {
            gameBounce = 1.2f; // Rebond très fort
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
            auto mazeWalls = game::MazeGenerator::generate(
            config.difficulty,
            WALL_THICKNESS
        );

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
        float ballRadius = 5.f; 

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
        
        // 5. Calcul du point d'ARRIVÉE (Bas Droite) pour plus tard
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
        app::ThemeAssets themeAssets = app::loadThemeAssets(cfg.designTheme, frameRGBA);
        GLuint ballTextureID = themeAssets.ballTexture;
        GLuint grassTexID = themeAssets.groundTexture;
        GLuint skyTexID = themeAssets.skyTexture;
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

        // === GESTION DU SCORE ===
        double startTime = glfwGetTime();
        double finalTime = 0.0;

        const float WIN_THRESHOLD = 17.0f; // Distance en mm pour gagner (environ une demi-cellule)
        // === BOUCLE PRINCIPALE ===
        while (!glfwWindowShouldClose(window)) {
        // --- LOGIQUE DE VICTOIRE ---
            if (!gameFinished && !paused) {
                // Calcul de la distance entre la balle et la sortie
                float distToTarget = glm::distance(glm::vec2(ballPos.x, ballPos.y), glm::vec2(targetPos.x, targetPos.y));
                
                if (distToTarget < WIN_THRESHOLD) {
                    gameFinished = true;
                    finalTime = glfwGetTime() - startTime;
                    std::cout << "FINISHED - TIME: " << finalTime << " s" << std::endl;
                    result = GameResult{
                        EndReason::WIN,
                        finalTime,
                        config.difficulty
                    };
                    break;
                }
            }
            double nowT = glfwGetTime();
            float dt = float(nowT - lastT);
            lastT = nowT;
            if (dt > 0.05f) dt = 0.05f;

            // Temps à afficher
            double currentTime = gameFinished ? finalTime : (glfwGetTime() - startTime);
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
                result = GameResult{
                    EndReason::QUIT,
                    0.0,
                    config.difficulty
                };
                break; // Sort de la boucle proprement (comme WIN)
            }
            lastEscPressed = escPressed;

            // Toggle FPS (touche F)
            bool fPressed = glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS;
            if (fPressed && !lastFPressed) {
                showFPS = !showFPS;
            }
            lastFPressed = fPressed;

            // Toggle Temps (touche T)
            bool tPressed = glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS;
            if (tPressed && !lastTPressed) {
                showTime = !showTime;
            }
            lastTPressed = tPressed;

            // Calcul FPS
            frameCount++;
            fpsTimer += dt;
            if (fpsTimer >= 0.5) { // Mise à jour toutes les 0.5s
                currentFPS = frameCount / fpsTimer;
                frameCount = 0;
                fpsTimer = 0.0;
            }

            if (!cap.read(frameBGR) || frameBGR.empty()) break;

            const double BLUR_THRESHOLD = 100.0; // Seuil de netteté (ajustable)
            double sharpness = detect::measureBlur(frameBGR);
            bool imageIsBlurry = sharpness < BLUR_THRESHOLD;

            // Détection A4 simple (sans tracking complexe)
            std::vector<cv::Point2f> imagePts;
            bool okDetect = false;

            // Si l'image est trop floue, on saute la détection et on utilise les dernières valeurs
            if (!imageIsBlurry) {
                okDetect = detect::detectA4Corners(frameBGR, imagePts);
            } else {

                okDetect = detect::detectA4Corners(frameBGR, imagePts);
            }

            if (okDetect) {
                // Calcul de la pose avec solvePnP
                cv::solvePnP(objectPts, imagePts, calib.cameraMatrix, calib.distCoeffs,
                           rvec, tvec, !rvec.empty(), cv::SOLVEPNP_ITERATIVE);

                // Appliquer filtrage EMA pour lisser la pose
                glm::vec3 rvecGlm(rvec.at<double>(0), rvec.at<double>(1), rvec.at<double>(2));
                glm::vec3 tvecGlm(tvec.at<double>(0), tvec.at<double>(1), tvec.at<double>(2));

                glm::vec3 rvecSmooth = rvecFilter.update(rvecGlm);
                glm::vec3 tvecSmooth = tvecFilter.update(tvecGlm);

                // Appliquer les valeurs filtrées
                rvec.at<double>(0) = rvecSmooth.x;
                rvec.at<double>(1) = rvecSmooth.y;
                rvec.at<double>(2) = rvecSmooth.z;
                tvec.at<double>(0) = tvecSmooth.x;
                tvec.at<double>(1) = tvecSmooth.y;
                tvec.at<double>(2) = tvecSmooth.z;
            }

            if (!paused && okDetect) {
                ar::updatePhysics(
                    rvec, dt,
                    ballPos, ballVel,
                    ballRotationMatrix,
                    ballRadius,
                    mazeWalls,
                    WALL_THICKNESS,
                    gameAccel,
                    gameBounce
                );
            }
            // === AFFICHAGE HUD (messages et indicateurs) ===
            if (!okDetect) {
                app::drawCenteredMessage(frameBGR, "Pas de A4 detecte ! Placez la feuille...");
            }
            if (imageIsBlurry && !okDetect) {
                app::drawWarningMessage(frameBGR, "Image floue ! Stabilisez la camera...");
            }
            if (paused) {
                app::drawPauseOverlay(frameBGR);
            }
            if (showFPS) {
                app::drawFPS(frameBGR, currentFPS);
            }
            if (showTime && !gameFinished) {
                app::drawTimer(frameBGR, currentTime);
            }

            // -- Mise à jour de la texture de fond ---
            updateVideoBackground(bgTex, frameBGR);


            // === RENDU OPENGL ===
            glfwPollEvents();

            // --- Gestion Touche 'V' (Toggle AR/VR) ---
            bool currentVPressed = !(glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS);
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
            // === CERCLE D'ARRIVÉE (Cible) ===
            if (!gameFinished) {
                drawTargetCircle(targetPos, 15.0f, P, V, renderCtx.solidProgram, solid_uMVP, solid_uColor);
            }
            // === AFFICHAGE SCORE (Temps) avec scaling automatique ===
            float uiScale = getUIScale(fbw, fbh);

            if (!gameFinished) {
                std::string timeStr = "Time: " + std::to_string((int)currentTime) + "s";
                drawText(10.0f * uiScale, fbh - 20.0f * uiScale, 2.0f * uiScale, timeStr);
            }


            glfwSwapBuffers(window);

        }

        // --- Nettoyage des ressources ---
        app::freeThemeAssets(themeAssets);
        glDeleteVertexArrays(1, &floorMesh.vao); glDeleteBuffers(1, &floorMesh.vbo);

        // --- Nettoyage OpenGL ---
        glx::cleanup(renderCtx.bgProgram, renderCtx.lineProgram, renderCtx.solidProgram, phongProgram, shadowProgram, bgTex, ballTextureID, renderCtx.bg, wallsMesh, renderCtx.ball, renderCtx.axes, window);
        cap.release();
        glfwDestroyWindow(window);
        // NE PAS appeler glfwTerminate() ici - le menu va réutiliser GLFW
        return result;
        
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << std::endl;
            return GameResult{
                        EndReason::ERROR,
                        0.0,
                        config.difficulty
                    };
        }
        
    }

/**
 * @brief Point d'entrée principal de l'application.
 *
 * Gère la boucle menu/jeu/score :
 * 1. Affiche le menu principal (showMenuWindow)
 * 2. Lance une partie (runApp)
 * 3. Affiche le score si victoire (showScoreWindow)
 * 4. Retourne au menu ou relance une partie
 *
 * @param argc Nombre d'arguments CLI.
 * @param argv Arguments CLI (--video, --calib, etc.).
 * @return 0 si succès, autre valeur si erreur.
 */
int main(int argc, char** argv) {
    AppConfig config;

    // === BOUCLE MENU PRINCIPAL ===
    while (true) {
        // Affichage du menu de sélection
        if (!showMenuWindow(config)) break;

        // === BOUCLE DE JEU ===
        bool stayInGame = true;
        while (stayInGame) {
            GameResult result = runApp(argc, argv, config);

            if (result.reason == EndReason::WIN) {
                // Victoire : afficher l'écran de score
                int choice = showScoreWindow(result);
                if (choice == 1) stayInGame = false; // Retour Menu
                // choice == 0 -> Rejouer
            } else if (result.reason == EndReason::QUIT) {
                stayInGame = false; // Retour au menu principal
            } else {
                stayInGame = false; // Erreur
            }
        }
    }
    return 0;
}