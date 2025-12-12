#include "Enemy.h"
#include "Game.h"
#include <cmath>
#include <algorithm>
#include <random>

Enemy::Enemy(const sf::Vector2f& start, Game* gamePtr, float initialHP, int t) : game(gamePtr), hp(initialHP), type(t) {
    // size the enemy visually relative to tileSize when game/map is available
    if (game) {
        const Map& m = game->getMap();
        float ts = m.getTileSize();
        float size = std::max(8.f, ts * 0.6f);
        shape.setSize({size, size});
        shape.setOrigin(size/2.f, size/2.f);
        shape.setPosition(start);
        // if textures are available, set sprite accordingly
        if (game->texturesLoaded) {
            if (type == 1 && game->enemy1Texture.getSize().x > 0) {
                sprite.setTexture(game->enemy1Texture);
            } else if (type == 2 && game->enemy2Texture.getSize().x > 0) {
                sprite.setTexture(game->enemy2Texture);
            }
            if (sprite.getTexture()) {
                sf::Vector2u tsize = sprite.getTexture()->getSize();
                if (tsize.x > 0 && tsize.y > 0) {
                    float scaleX = (size * 2.f) / float(tsize.x);
                    float scaleY = (size * 2.f) / float(tsize.y);
                    sprite.setScale(scaleX, scaleY);
                    sprite.setOrigin(tsize.x/2.f, tsize.y/2.f);
                    sprite.setPosition(start);
                }
            }
        }
    } else {
        shape.setSize({20.f, 20.f});
        shape.setOrigin(10.f, 10.f);
        shape.setPosition(start);
    }
    shape.setFillColor(sf::Color(200,50,50));
    shape.setOutlineColor(sf::Color::Black);
    shape.setOutlineThickness(1.f);
    // seed RNG
    std::random_device rd;
    rng.seed(rd());

    // if game provided, compute starting tile coords and set initial target
    if (game) {
        const Map& m = game->getMap();
        float ts = m.getTileSize();
        tx = static_cast<int>(shape.getPosition().x / ts);
        ty = static_cast<int>(shape.getPosition().y / ts);
        snapToTileCenter();
        targetPos = shape.getPosition();
        // update logical radius
        radius = shape.getSize().x * 0.45f;
    }
}

void Enemy::setPath(const std::vector<sf::Vector2f>& p) {
    path = p;
    pathIndex = 0;
}

void Enemy::snapToTileCenter() {
    if (!game) return;
    sf::Vector2f newPos = game->getMap().tileCenter(tx, ty);
    shape.setPosition(newPos);
    if (sprite.getTexture()) sprite.setPosition(newPos);
}

void Enemy::update(float dt) {
    if (!alive) return;
    
    // If following an explicit path, prefer that
    if (!path.empty() && pathIndex < path.size()) {
        sf::Vector2f pos = shape.getPosition();
        sf::Vector2f target = path[pathIndex];
        sf::Vector2f dir = target - pos;
        float dist = std::hypot(dir.x, dir.y);
        if (dist < 2.f) { 
            pathIndex++; 
            return; 
        }
        dir /= dist;
        shape.move(dir * speed * dt);
        if (sprite.getTexture()) sprite.setPosition(shape.getPosition());
        return;
    }

    // If we have a Game pointer, use BFS distance map to move toward base
    if (game) {
        const Map& m = game->getMap();
        const auto& distMap = game->getDistance();
        if (!distMap.empty()) {
            float ts = m.getTileSize();
            int curTx = static_cast<int>(shape.getPosition().x / ts);
            int curTy = static_cast<int>(shape.getPosition().y / ts);
            curTx = std::clamp(curTx, 0, m.getCols()-1);
            curTy = std::clamp(curTy, 0, m.getRows()-1);

            // Debug info: print current tile and distance
            // debug print removed
            // Check if reached base
            if (m.getTile(curTx, curTy) == 3) {
                alive = false;
                return;
            }

            int bestX = curTx, bestY = curTy;
            int bestDist = distMap[curTy][curTx];
            auto neighbors = game->getNeighborsPublic(curTx, curTy);
            for (auto n : neighbors) {
                int nx = n.x, ny = n.y;
                int d = distMap[ny][nx];
                if (d != -1 && (bestDist == -1 || d < bestDist)) {
                    bestDist = d; bestX = nx; bestY = ny;
                }
            }

            // move toward center of best tile
            sf::Vector2f target = m.tileCenter(bestX, bestY);
            sf::Vector2f dir = target - shape.getPosition();
            float dist = std::hypot(dir.x, dir.y);
                if (dist > 1.f) {
                    dir /= dist;
                    shape.move(dir * speed * dt);
                    if (sprite.getTexture()) sprite.setPosition(shape.getPosition());
            } else {
                // reached target tile center: snap and update tx/ty
                tx = bestX; ty = bestY;
                snapToTileCenter();
                if (tx != prevTx || ty != prevTy) {
                    prevTx = tx; prevTy = ty;
                }
            }
            return;
        }
    }

    // fallback: do nothing (or random-walk if desired)
}

void Enemy::render(sf::RenderWindow& window) {
    if (sprite.getTexture()) {
        window.draw(sprite);
    } else {
        window.draw(shape);
    }
}

sf::Vector2f Enemy::getPosition() const {
    return shape.getPosition();
}

void Enemy::takeDamage(float dmg) {
    hp -= dmg;
    if (hp <= 0) alive = false;
}

bool Enemy::isAlive() const { 
    return alive; 
}

float Enemy::getRadius() const { 
    return radius; 
}
