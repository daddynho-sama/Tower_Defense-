#include "TowerTypes.h"
#include "Game.h"
#include "Enemy.h"
#include "Projectile.h"
#include <cmath>

// ========== SNIPER TOWER (Normal, High Damage, Single Target) ==========
SniperTower::SniperTower(sf::Vector2f pos, Game* game)
    : Tower(pos, 75, game) {
    // Override base stats
    range = 250.f;      // Longest range
    damage = 40.f;      // Highest damage
    fireRate = 0.8f;    // Slower fire rate (0.8 shots/sec)
    cost = 75;
    upgradeCost = 120;
}

void SniperTower::render(sf::RenderWindow& window) {
    // Base circle (red for sniper)
    sf::CircleShape base(12.f);
    base.setPosition(pos - sf::Vector2f(12.f, 12.f));
    base.setFillColor(sf::Color(200, 50, 50));
    base.setOutlineColor(sf::Color::Black);
    base.setOutlineThickness(1.f);
    window.draw(base);

    // Range indicator (dashed red circle)
    sf::CircleShape rangeCircle(range);
    rangeCircle.setPosition(pos - sf::Vector2f(range, range));
    rangeCircle.setFillColor(sf::Color::Transparent);
    rangeCircle.setOutlineThickness(1.f);
    rangeCircle.setOutlineColor(sf::Color(200, 50, 50, 100));
    window.draw(rangeCircle);

    // Barrel (longer for sniper)
    sf::RectangleShape barrel(sf::Vector2f(20.f, 4.f));
    barrel.setPosition(pos);
    barrel.setOrigin(20.f, 2.f);
    barrel.setRotation(angle * 180.f / 3.14159f);
    barrel.setFillColor(sf::Color::Black);
    window.draw(barrel);
}

// ========== FREEZING TOWER (Slow, Low Damage, Single Target) ==========
FreezingTower::FreezingTower(sf::Vector2f pos, Game* game)
    : Tower(pos, 50, game) {
    // Override base stats
    range = 200.f;      // Medium range
    damage = 5.f;       // Very low damage (slowing is primary)
    fireRate = 2.f;     // Faster fire rate (2 shots/sec)
    cost = 50;
    upgradeCost = 80;
}

void FreezingTower::render(sf::RenderWindow& window) {
    // Base circle (cyan for freezing)
    sf::CircleShape base(12.f);
    base.setPosition(pos - sf::Vector2f(12.f, 12.f));
    base.setFillColor(sf::Color(100, 200, 255));
    base.setOutlineColor(sf::Color::Black);
    base.setOutlineThickness(1.f);
    window.draw(base);

    // Range indicator (cyan circle)
    sf::CircleShape rangeCircle(range);
    rangeCircle.setPosition(pos - sf::Vector2f(range, range));
    rangeCircle.setFillColor(sf::Color::Transparent);
    rangeCircle.setOutlineThickness(1.f);
    rangeCircle.setOutlineColor(sf::Color(100, 200, 255, 100));
    window.draw(rangeCircle);

    // Barrel (normal)
    sf::RectangleShape barrel(sf::Vector2f(15.f, 4.f));
    barrel.setPosition(pos);
    barrel.setOrigin(15.f, 2.f);
    barrel.setRotation(angle * 180.f / 3.14159f);
    barrel.setFillColor(sf::Color(100, 200, 255));
    window.draw(barrel);
}

// ========== CANNON TOWER (AOE, Medium Damage, Multiple Targets) ==========
CannonTower::CannonTower(sf::Vector2f pos, Game* game)
    : Tower(pos, 100, game) {
    // Override base stats
    range = 180.f;      // Medium range
    damage = 25.f;      // Medium damage per hit
    fireRate = 0.6f;    // Slower fire rate (0.6 shots/sec)
    cost = 100;
    upgradeCost = 150;
}

void CannonTower::render(sf::RenderWindow& window) {
    // Base circle (yellow for cannon)
    sf::CircleShape base(14.f);  // Slightly larger
    base.setPosition(pos - sf::Vector2f(14.f, 14.f));
    base.setFillColor(sf::Color(255, 200, 0));
    base.setOutlineColor(sf::Color::Black);
    base.setOutlineThickness(2.f);
    window.draw(base);

    // Range indicator (yellow circle)
    sf::CircleShape rangeCircle(range);
    rangeCircle.setPosition(pos - sf::Vector2f(range, range));
    rangeCircle.setFillColor(sf::Color::Transparent);
    rangeCircle.setOutlineThickness(1.f);
    rangeCircle.setOutlineColor(sf::Color(255, 200, 0, 100));
    window.draw(rangeCircle);

    // Barrel (thicker for cannon)
    sf::RectangleShape barrel(sf::Vector2f(18.f, 6.f));
    barrel.setPosition(pos);
    barrel.setOrigin(18.f, 3.f);
    barrel.setRotation(angle * 180.f / 3.14159f);
    barrel.setFillColor(sf::Color(200, 150, 0));
    window.draw(barrel);
}

void CannonTower::shoot(Game& game) {
    // Find target first (inherited method)
    auto target = findTarget(game);
    if (!target) return;

    // Direction toward target
    sf::Vector2f delta = target->getPosition() - pos;
    float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    if (dist <= 0.1f) return;

    sf::Vector2f direction = delta / dist;

    // Create primary projectile
    // Use the fire sprite for cannon projectile if available (projType 1)
    game.projectiles.push_back(
        std::make_unique<Projectile>(pos, direction, 250.f, damage, &game, 1)
    );

    // AOE explosion: damage all enemies within explosionRadius of target position
    sf::Vector2f explosionCenter = target->getPosition() + direction * 100.f;
    for (auto& e : game.enemies) {
        if (!e->isAlive()) continue;

        sf::Vector2f delta2 = e->getPosition() - explosionCenter;
        float distToExplosion = std::sqrt(delta2.x * delta2.x + delta2.y * delta2.y);

        if (distToExplosion <= explosionRadius) {
            e->takeDamage(damage * 0.7f);  // 70% damage in AOE
        }
    }

    // Reset cooldown
    cooldown = 1.f / fireRate;
}

// Sniper tower shoots larger bullets (projType 2)
void SniperTower::shoot(Game& game) {
    auto t = currentTarget.lock();
    if (!t) return;
    sf::Vector2f delta = t->getPosition() - pos;
    float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    if (dist <= 0.1f) return;
    sf::Vector2f direction = delta / dist;
    game.projectiles.push_back(
        std::make_unique<Projectile>(pos, direction, 500.f, damage, &game, 2)
    );
    cooldown = 1.f / fireRate;
}
