#include "Tower.h"
#include "Game.h"
#include "Enemy.h"
#include "Projectile.h"
#include <cmath>
#include <algorithm>

// Helper function to normalize vectors
sf::Vector2f normalize(sf::Vector2f v) {
    float len = std::sqrt(v.x * v.x + v.y * v.y);
    if (len == 0) return {0, 0};
    return v / len;
}

// Helper function to compute vector length
float length(sf::Vector2f v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

Tower::Tower(const sf::Vector2f& position, int c, Game* game)
    : pos(position), cost(c), gamePtr(game) {
    baseShape.setRadius(15.f);
    baseShape.setOrigin(15.f, 15.f);
    baseShape.setPosition(pos);
    baseShape.setFillColor(sf::Color::Blue);
}

void Tower::update(float dt) {
    if (gamePtr) {
        update(dt, *gamePtr);
    }
}

void Tower::update(float dt, Game& game) {
    cooldown -= dt;
    if (cooldown < 0) cooldown = 0;

    if (auto target = currentTarget.lock()) {
        if (!isValidTarget(target, game))
            currentTarget.reset();
    }

    if (currentTarget.expired())
        currentTarget = findTarget(game);

    if (updateAngle(dt, game)) {
        if (cooldown <= 0.f)
            shoot(game);
    }
}

std::shared_ptr<Enemy> Tower::findTarget(const Game& game) const {
    std::shared_ptr<Enemy> best;
    float bestDist = range;

    for (auto& e : game.enemies) {
        if (!e->isAlive()) continue;

        float d = length(e->getPosition() - pos);
        if (d <= range && d < bestDist) {
            best = e;
            bestDist = d;
        }
    }
    return best;
}

bool Tower::isValidTarget(const std::shared_ptr<Enemy>& e, const Game& game) const {
    if (!e || !e->isAlive()) return false;
    float d = length(e->getPosition() - pos);
    return d <= range;
}

bool Tower::updateAngle(float dt, const Game& game) {
    auto target = currentTarget.lock();
    if (!target) return false;

    sf::Vector2f toTarget = target->getPosition() - pos;
    float desired = std::atan2(toTarget.y, toTarget.x);

    float diff = desired - angle;
    // Normalize diff to [-pi, pi]
    while (diff > M_PI) diff -= 2.f * M_PI;
    while (diff < -M_PI) diff += 2.f * M_PI;

    float maxStep = rotationSpeed * dt;
    diff = std::clamp(diff, -maxStep, maxStep);
    angle += diff;

    return std::abs(desired - angle) < 0.05f;
}

void Tower::shoot(Game& game) {
    auto t = currentTarget.lock();
    if (!t) return;

    sf::Vector2f dir = normalize(t->getPosition() - pos);

    game.projectiles.push_back(
        std::make_unique<Projectile>(pos, dir, 300.f, damage, &game)
    );

    cooldown = 1.f / fireRate;
}

void Tower::upgrade() {
    level++;
    damage *= 1.4f;
    range += 20.f;
    fireRate += 0.2f;
    baseShape.setFillColor(sf::Color(100, 100, 255)); // lighter blue
}

void Tower::render(sf::RenderWindow& window) {
    baseShape.setRotation(angle * 180.f / M_PI);
    baseShape.setPosition(pos);
    window.draw(baseShape);
}

sf::Vector2f Tower::getPosition() const {
    return pos;
}
