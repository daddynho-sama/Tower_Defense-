#ifndef GAME_HPP
#define GAME_HPP
#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <deque>
#include "ElementGraphique.h"
#include "Map.h"
#include "Enemy.h"
#include "Tower.h"
#include "Projectile.h"
#include "GameUI.h"

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
    std::unique_ptr<GameUI> ui;  // UI system
    
    // Economy
    int money = 200; // starting money (user requested 150-200 dollars)
    
    // Player Health
    int playerHealth = 20;
    bool gameOver = false;
    
    // Wave system
    int currentWave = 0;
    float waveTimer = 0.f;
    float waveCooldown = 5.f;  // seconds between waves
    // Spawn queue: sequential spawn to form battalions
    struct SpawnInfo { int type; float hp; };
    std::deque<SpawnInfo> spawnQueue;  // queue of enemies to spawn (type + HP)
    float spawnInterval = 0.6f; // seconds between spawns
    float spawnTimer = 0.f;     // timer until next spawn
    float enemyBaseHP = 50.f;   // base hp for enemies
    float enemyHpScale = 10.f;  // additional hp per wave
    float nextSpawnHP = 50.f;   // HP for next spawns
    int spawnTileX = -1;
    int spawnTileY = -1;
    // Textures for sprites and projectiles
    sf::Texture enemy1Texture;
    sf::Texture enemy2Texture;
    sf::Texture fireArrowTexture;
    bool texturesLoaded = false;
    // Portal animation (vortex) timer
    float portalAnimTime = 0.f;
    float spawnPortalPulse = 0.f; // pulse factor 0..1
    float basePortalPulse = 0.f; // pulse factor 0..1
    
    // Tower placement
    int selectedTowerType = 0;  // 0=Sniper, 1=Freezing, 2=Cannon
    bool placingTower = false;
    sf::Vector2f previewPos = {-1000, -1000};
    
    // BFS
    std::vector<std::vector<int>> distance;
    std::vector<std::vector<sf::Vector2i>> came_from;
    std::vector<std::vector<bool>> tileBlocked; // track blocked tiles (towers)
    bool paused = false;
    int placementBanRadiusTiles = 2; // cannot place towers within this radius of spawn or base
    bool gameStarted = false; // main menu/started state
    int startingMoney = 200; // default starting money used at reset

    Game();
    void run();
    void startNewGame();
    Map& getMap() { return map; }
    const Map& getMap() const { return map; }
    const std::vector<std::vector<int>>& getDistance() const { return distance; }
    std::vector<sf::Vector2i> getNeighborsPublic(int tx, int ty) const { return getNeighbors(tx, ty); }
    
    // Gameplay
    void spawnEnemyWave(int count);
    void cleanupDeadStuff();
    void damagePlayer(int dmg);  // called when enemy reaches base
    int getWaveEnemyCount(int wave) const;  // returns enemy count for given wave
    
    // Tower placement
    void placeTower(int towerType);  // 0=Sniper, 1=Freezing, 2=Cannon
    void handleMouseMove(const sf::Vector2f& mousePos);
    void handleMouseClick(const sf::Vector2f& mousePos);
    
private:
    void processEvents();
    void update(float dt);
    void render();
    std::vector<sf::Vector2i> getNeighbors(int tx, int ty) const;
    void computeBFS();
    void drawPortals(sf::RenderWindow& window);
};



#endif /* GAME_HPP */
