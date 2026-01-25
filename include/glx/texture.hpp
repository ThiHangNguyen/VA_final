#pragma once

#include <GL/glew.h>
#include <opencv2/core.hpp>
#include <string>

/**
 * @file texture.hpp
 * @brief Création, chargement et mise à jour de textures OpenGL depuis des images OpenCV.
 *
 * Ce module fournit des fonctions utilitaires permettant :
 * - de créer des textures OpenGL vides (RGBA)
 * - de mettre à jour des textures à partir de cv::Mat
 * - de charger des textures depuis des fichiers image
 *
 * Il est principalement utilisé pour afficher des images caméra,
 * des arrière-plans et des textures d'objets en réalité augmentée.
 */

namespace glx {

/**
 * @brief Crée une texture OpenGL vide au format RGBA.
 *
 * La texture est allouée côté GPU mais ne contient pas encore de données.
 * Elle est destinée à être mise à jour dynamiquement (ex: flux caméra).
 *
 * @param w Largeur de la texture (en pixels).
 * @param h Hauteur de la texture (en pixels).
 * @return Identifiant OpenGL de la texture créée.
 */
GLuint createTextureRGBA(int w, int h);

/**
 * @brief Met à jour une texture OpenGL RGBA existante à partir d'une image OpenCV.
 *
 * L'image fournie doit :
 * - être au format RGBA
 * - être déjà retournée verticalement (flip Y) pour correspondre
 *   au repère OpenGL
 *
 * @param tex Identifiant de la texture OpenGL à mettre à jour.
 * @param rgbaFlipped Image OpenCV RGBA avec axe Y inversé.
 */
void updateTextureRGBA(GLuint tex, const cv::Mat& rgbaFlipped);

/**
 * @brief Crée une texture OpenGL à partir d'une image OpenCV.
 *
 * Cette fonction :
 * - accepte des images RGB ou RGBA
 * - effectue les conversions nécessaires
 * - crée et initialise la texture OpenGL correspondante
 *
 * @param imgRGBorRGBA Image OpenCV en RGB ou RGBA.
 * @return Identifiant OpenGL de la texture créée.
 */
GLuint createTextureFromMat(const cv::Mat& imgRGBorRGBA);

/**
 * @brief Charge une texture OpenGL depuis un fichier image.
 *
 * Le chargement inclut :
 * - la lecture du fichier image
 * - la conversion vers un format compatible OpenGL
 * - la création de la texture associée
 *
 * @param path Chemin vers le fichier image.
 * @param flip Indique si l'image doit être retournée verticalement
 *             (par défaut : false).
 * @return Identifiant OpenGL de la texture chargée.
 */
GLuint loadTexture(const std::string& path, bool flip = false);

} // namespace glx
