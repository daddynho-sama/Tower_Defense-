#ifndef TOWER_HPP
#define TOWER_HPP
#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <cmath>
#include "ElementGraphique.h"

class Enemy; // forward
class Game;  // forward

class Tower : public ElementGraphique {
private:
    sf::Vector2f pos;
    sf::CircleShape baseShape;
    Game* gamePtr = nullptr;  // store Game pointer for update() override
    
    // Tower stats
    float range = 160.f;
    float damage = 20.f;
    float fireRate = 1.f;
    float cooldown = 0.f;
    float rotationSpeed = 3.14f; // rad/s
    float angle = 0.f;
    int level = 1;
    int cost = 60;
    int upgradeCost = 80;
    std::weak_ptr<Enemy> currentTarget;

public:
    Tower(const sf::Vector2f& position, int c = 60, Game* game = nullptr);
    void update(float dt) override;  // calls update(dt, *gamePtr) if gamePtr available
    void update(float dt, Game& game);  // actual implementation
    void render(sf::RenderWindow& window) override;
    sf::Vector2f getPosition() const override;
    
    // Tower methods
    std::shared_ptr<Enemy> findTarget(const Game& game) const;
    bool isValidTarget(const std::shared_ptr<Enemy>& e, const Game& game) const;
    bool updateAngle(float dt, const Game& game);
    void shoot(Game& game);
    void upgrade();
    
    // Getters
    int getCost() const { return cost; }
    float getRange() const { return range; }
    float getDamage() const { return damage; }
};

#endif /* TOWER_HPP */
