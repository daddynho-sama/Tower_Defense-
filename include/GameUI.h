#ifndef GAMEUI_HPP
#define GAMEUI_HPP
#pragma once

#include <SFML/Graphics.hpp>
#include <string>

class Game;

class GameUI {
private:
    const Game* game;
    sf::Font font;
    bool fontLoaded = false;

public:
    GameUI(const Game* g);
    void render(sf::RenderWindow& window);
    
private:
    std::string getTowerName(int type) const;
    int getTowerCost(int type) const;
};

#endif /* GAMEUI_HPP */
