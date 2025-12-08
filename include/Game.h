#ifndef GAME_HPP
#define GAME_HPP
#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "ElementGraphique.h"
#include "Map.h"
#include "Enemy.h"
#include "Tower.h"
#include "Projectile.h"

class Game {
public:
    // Game state
    sf::RenderWindow window;
    Map map;
    sf::Clock clock;
    
    // Entities
    std::vector<std::shared_ptr<Enemy>> enemies;
    std::vector<std::unique_ptr<Tower>> towers;
    std::vector<std::unique_ptr<Projectile>> projectiles;
    
    // Economy
    int money = 200;
    
    // BFS
    std::vector<std::vector<int>> distance;
    std::vector<std::vector<sf::Vector2i>> came_from;

    Game();
    void run();
    Map& getMap() { return map; }
    const Map& getMap() const { return map; }
    const std::vector<std::vector<int>>& getDistance() const { return distance; }
    std::vector<sf::Vector2i> getNeighborsPublic(int tx, int ty) const { return getNeighbors(tx, ty); }
    
    // Gameplay
    void spawnEnemyWave(int count);
    void cleanupDeadStuff();
    
private:
    void processEvents();
    void update(float dt);
    void render();
    std::vector<sf::Vector2i> getNeighbors(int tx, int ty) const;
    void computeBFS();
};



#endif /* GAME_HPP */
