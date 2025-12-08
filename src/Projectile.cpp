#include "Projectile.h"
#include "Game.h"
#include "Enemy.h"
#include <cmath>

Projectile::Projectile(sf::Vector2f p, sf::Vector2f d, float s, float dmg, Game* g)
    : pos(p), dir(d), speed(s), damage(dmg), game(g) {}

void Projectile::update(float dt) {
    pos += dir * speed * dt;

    if (!game) return; // no collision if game ptr not set

    for (auto& e : game->enemies) {
        if (!e->isAlive()) continue;

        sf::Vector2f delta = e->getPosition() - pos;
        float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);
        if (dist <= e->getRadius()) {
            e->takeDamage(damage);
            dead = true;
            return;
        }
    }

    // check if projectile went off-screen (simple bounds check)
    if (pos.x < -100 || pos.x > 3000 || pos.y < -100 || pos.y > 3000) {
        dead = true;
    }
}

void Projectile::render(sf::RenderWindow& w) {
    sf::CircleShape bullet(3.f);
    bullet.setPosition(pos.x - 3.f, pos.y - 3.f);
    bullet.setFillColor(sf::Color::Yellow);
    w.draw(bullet);
}

sf::Vector2f Projectile::getPosition() const {
    return pos;
}
