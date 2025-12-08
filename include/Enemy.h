#ifndef ENEMY_CPP
#define ENEMY_CPP
#pragma once
#include "ElementGraphique.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <random>

class Game; // forward

class Enemy : public ElementGraphique {
    sf::RectangleShape shape;
    float speed = 80.f; // px/s
    // path-following (optional)
    std::vector<sf::Vector2f> path;
    size_t pathIndex = 0;

    // pointers to world
    Game* game = nullptr;
    int tx = 0, ty = 0; // current tile coords
    sf::Vector2f targetPos; // target center when moving
    std::mt19937 rng;

    // HP and status
    float hp = 50.f;
    bool alive = true;
    float radius = 12.f;

public:
    Enemy(const sf::Vector2f& start, Game* gamePtr = nullptr);

    void setPath(const std::vector<sf::Vector2f>& p);
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
    sf::Vector2f getPosition() const override;
    
    // HP and damage
    void takeDamage(float dmg);
    bool isAlive() const;
    float getRadius() const;
    
    // helpers
    void snapToTileCenter();
};

#endif /* ENEMY_CPP */
