#include "Projectile.h"
#include "Game.h"
#include "Enemy.h"
#include <cmath>

Projectile::Projectile(sf::Vector2f p, sf::Vector2f d, float s, float dmg, Game* g, int type)
    : pos(p), dir(d), speed(s), damage(dmg), game(g), projType(type) {}

void Projectile::update(float dt) {
    pos += dir * speed * dt;

    if (!game) return; // no collision if game ptr not set

    // More generous collision radius for projectile
    float projectileRadius = 5.f;
    if (projType == 1) {
        // larger radius for the cannon projectile (fire)
        if (game) {
            float ts = game->getMap().getTileSize();
            projectileRadius = std::max(12.f, ts * 0.25f); // ~25% of tile size, min 12px
        } else {
            projectileRadius = 14.f;
        }
    } else if (projType == 2) {
        projectileRadius = 8.f; // big sniper ball
    }
    
    for (auto& e : game->enemies) {
        if (!e->isAlive()) continue;

        sf::Vector2f delta = e->getPosition() - pos;
        float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);
        float collisionDist = e->getRadius() + projectileRadius;
        
        if (dist <= collisionDist) {
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
    if (projType == 1 && game && game->texturesLoaded && game->fireArrowTexture.getSize().x > 0) {
        sf::Sprite s;
        s.setTexture(game->fireArrowTexture);
        sf::Vector2u ts = game->fireArrowTexture.getSize();
        if (ts.x > 0 && ts.y > 0) {
            // compute a desired size based on map tile size for consistent appearance
            float desiredPx = std::max(24.f, game->getMap().getTileSize() * 0.5f); // 50% of tile or min 24px
            float scale = desiredPx / float(std::max(ts.x, ts.y));
            s.setScale(scale, scale);
            s.setOrigin(ts.x/2.f, ts.y/2.f);
            s.setPosition(pos);
            w.draw(s);
            return;
        }
    }

    float radius = 3.f;
    sf::Color color = sf::Color::Yellow;
    if (projType == 2) { radius = 6.f; color = sf::Color::Red; }
    else if (projType == 1) { radius = 4.f; color = sf::Color(255,140,0); }

    sf::CircleShape bullet(radius);
    bullet.setPosition(pos.x - radius, pos.y - radius);
    bullet.setFillColor(color);
    w.draw(bullet);
}

sf::Vector2f Projectile::getPosition() const {
    return pos;
}
