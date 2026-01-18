#pragma once
#include "app/game.hpp"   // GameResult
#include <GLFW/glfw3.h>

enum class ScoreAction {
    REPLAY,
    MENU,
    EXIT
};

// Affiche l'écran de score et retourne l'action choisie
int showScoreWindow(const GameResult& result)  ;
