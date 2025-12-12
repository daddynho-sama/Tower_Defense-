#ifndef TOWERTYPES_HPP
#define TOWERTYPES_HPP
#pragma once

#include "Tower.h"
#include <vector>

class Game;

// === TOWER 1: SNIPER TOWER (Normal, Single Target, High Damage) ===
class SniperTower : public Tower {
private:
    float slowness = 0.f;  // No slowness for sniper

public:
    SniperTower(sf::Vector2f pos, Game* game);
    void render(sf::RenderWindow& window) override;
    void shoot(Game& game) override;
};

// === TOWER 2: FREEZING TOWER (Slow, Single Target, Low Damage) ===
class FreezingTower : public Tower {
private:
    float slowFactor = 0.5f;  // Enemies move at 50% speed in range
    float slowRadius = 200.f;

public:
    FreezingTower(sf::Vector2f pos, Game* game);
    void render(sf::RenderWindow& window) override;
    float getSlowFactor() const { return slowFactor; }
    float getSlowRadius() const { return slowRadius; }
};

// === TOWER 3: CANNON TOWER (AOE, Multiple Targets, Medium Damage) ===
class CannonTower : public Tower {
private:
    float explosionRadius = 120.f;

public:
    CannonTower(sf::Vector2f pos, Game* game);
    void render(sf::RenderWindow& window) override;
    void shoot(Game& game) override;  // Override shoot to handle AoE
    float getExplosionRadius() const { return explosionRadius; }
};

#endif /* TOWERTYPES_HPP */
