/**
 * @file score.hpp
 * @brief Ecran de victoire avec affichage du temps et boutons.
 *
 * Affiche une fenetre GLFW avec le temps de completion,
 * et propose de rejouer, retourner au menu ou quitter.
 *
 * @author Thi Hang NGUYEN & Bichoy DAOUD
 */

#pragma once
#include "app/game.hpp"
#include <GLFW/glfw3.h>

/**
 * @enum ScoreAction
 * @brief Actions possibles apres l'ecran de victoire.
 */
enum class ScoreAction {
    REPLAY,  ///< Rejouer le meme niveau
    MENU,    ///< Retourner au menu principal
    EXIT     ///< Quitter le jeu
};

/**
 * @brief Affiche l'ecran de victoire et attend le choix du joueur.
 * @param result Resultat de la partie (temps, raison de fin)
 * @return Code d'action (0=replay, 1=menu, 2=exit)
 */
int showScoreWindow(const GameResult& result);
