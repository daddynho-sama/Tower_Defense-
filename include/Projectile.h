#ifndef PROJECTILE_HPP
#define PROJECTILE_HPP
#pragma once
#include "ElementGraphique.h"
#include <SFML/Graphics.hpp>

class Game; // forward

class Projectile : public ElementGraphique {
public:
    sf::Vector2f pos;
    sf::Vector2f dir;
    float speed;
    float damage;
    bool dead = false;
    Game* game = nullptr;
    int projType = 0; // 0=default, 1=fire arrow, 2=big sniper ball
    Projectile(sf::Vector2f p, sf::Vector2f d, float s, float dmg, Game* g = nullptr, int type = 0);

    void update(float dt) override;
    void render(sf::RenderWindow& w) override;
    sf::Vector2f getPosition() const override;
};

#endif /* PROJECTILE_HPP */
