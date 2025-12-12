#include "Game.h"
#include "Tower.h"
#include "TowerTypes.h"
#include "Enemy.h"
#include "Projectile.h"
#include "GameUI.h"
#include <deque>
#include <filesystem>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cmath>
#include <algorithm>

Game::Game() : window(sf::VideoMode(800,600), "TowerDefense - prototype"), map(48.f) {
    // charge la map depuis le fichier assets/Map.txt si possible
    // try common relative paths: when running from project root or from build/
    bool ok = map.loadFromFile("assets/Map.txt");
    if (!ok) ok = map.loadFromFile("../assets/Map.txt");
    if (!ok) {
        // fallback : crée une map 16x12 si le chargement échoue
        map = Map(16,12,48.f);
    }

    // now that map is initialized, resize the window to fit the map
    int width = map.getCols() * static_cast<int>(map.getTileSize());
    int height = map.getRows() * static_cast<int>(map.getTileSize());
    if (width > 0 && height > 0) {
        window.create(sf::VideoMode(width, height), "TowerDefense - prototype");
    }

    // initialize tileBlocked grid (no tower blocks at start)
    tileBlocked.assign(map.getRows(), std::vector<bool>(map.getCols(), false));

    // compute BFS distance map once for all enemies (must be done after the map is loaded)
    computeBFS();
    
    // Initialize UI
    ui = std::make_unique<GameUI>(this);
    // seed randomness
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    
    // load textures (enemy sprites, projectiles)
    texturesLoaded = false;
    bool ok1 = false, ok2 = false, ok3 = false;
    namespace fs = std::filesystem;
    if (fs::exists("assets/sprites/ennemie1.png")) ok1 = enemy1Texture.loadFromFile("assets/sprites/ennemie1.png");
    else if (fs::exists("../assets/sprites/ennemie1.png")) ok1 = enemy1Texture.loadFromFile("../assets/sprites/ennemie1.png");
    if (fs::exists("assets/sprites/ennemie2.png")) ok2 = enemy2Texture.loadFromFile("assets/sprites/ennemie2.png");
    else if (fs::exists("../assets/sprites/ennemie2.png")) ok2 = enemy2Texture.loadFromFile("../assets/sprites/ennemie2.png");
    if (fs::exists("assets/sprites/Fire.png")) ok3 = fireArrowTexture.loadFromFile("assets/sprites/Fire.png");
    else if (fs::exists("../assets/sprites/Fire.png")) ok3 = fireArrowTexture.loadFromFile("../assets/sprites/Fire.png");
    if (ok1 || ok2 || ok3) texturesLoaded = true;
    // Debug prints to confirm asset loading at runtime
    if (ok1) std::cout << "Loaded enemy1 sprite (assets/sprites/ennemie1.png)" << std::endl;
    if (ok2) std::cout << "Loaded enemy2 sprite (assets/sprites/ennemie2.png)" << std::endl;
    if (ok3) std::cout << "Loaded fire sprite (assets/sprites/Fire.png)" << std::endl;

    // Spawn first wave of enemies
        // show main menu at startup: wait for player to press Start
        gameStarted = false;
        // show main menu at startup: wait for player to press Start
        gameStarted = false;
}

void Game::startNewGame() {
    // Clear entities
    enemies.clear();
    towers.clear();
    projectiles.clear();
    // Reset state
    money = startingMoney;
    playerHealth = 20;
    gameOver = false;
    paused = false;
    currentWave = 0;
    waveTimer = 0.f;
    spawnQueue.clear();
    // recompute BFS
    computeBFS();
    // start first wave
    spawnEnemyWave(getWaveEnemyCount(0));
}

std::vector<sf::Vector2i> Game::getNeighbors(int tx, int ty) const {
    std::vector<sf::Vector2i> result;
    const std::pair<int,int> deltas[4] = {{1,0},{-1,0},{0,1},{0,-1}};
    for (auto d : deltas) {
        int nx = tx + d.first;
        int ny = ty + d.second;
        // check bounds and avoid obstacle tile value 2
        int val = map.getTile(nx, ny);
        // getTile returns 0 for out-of-bounds; ensure in-bounds by comparing coords
        if (nx < 0 || ny < 0 || nx >= map.getCols() || ny >= map.getRows()) continue;
        if (val == 2) continue; // treat 2 as obstacle
        // also treat tower blocked tiles as obstacles
        if (tx >=0 && ty >=0 && tx < (int)map.getCols() && ty < (int)map.getRows()) {
            if (tileBlocked[ny][nx]) continue;
        }
        result.emplace_back(nx, ny);
    } 
    return result;
}

void Game::computeBFS(){
    auto base = map.findBase();
    int bx = base.first;
    int by = base.second;
    if (bx < 0 || by < 0) return; // no base found
    distance.assign(map.getRows(), std::vector<int>(map.getCols(), -1));
    came_from.assign(map.getRows(), std::vector<sf::Vector2i>(map.getCols(), {-1,-1}));
    std::deque<sf::Vector2i> frontier;
    frontier.push_back({bx, by});
    distance[by][bx] = 0;
    while (!frontier.empty()){
        sf::Vector2i current = frontier.front();
        frontier.pop_front();
        auto neighbors = getNeighbors(current.x, current.y);
        for (auto n : neighbors){
            int nx = n.x;
            int ny = n.y;
            if (distance[ny][nx] == -1){
                distance[ny][nx]= distance[current.y][current.x]+1;
                came_from[ny][nx] = current;
                frontier.push_back(n);
            }
        }
    }
}

void Game::spawnEnemyWave(int count) {
    // find spawn tile (value 4)
    int spawnTx = -1, spawnTy = -1;
    for (int y = 0; y < map.getRows(); ++y) {
        for (int x = 0; x < map.getCols(); ++x) {
            if (map.getTile(x, y) == 4) {
                spawnTx = x; spawnTy = y;
                goto found;  // break double loop
            }
        }
    }
    found:
    
    if (spawnTx == -1) {
        spawnTx = 0; spawnTy = std::min(map.getRows()-1, 6);
    }
    spawnTileX = spawnTx;
    spawnTileY = spawnTy;

    // Instead of spawning all at once, queue them with spawnInterval spacing
    spawnQueue.clear();
    spawnTimer = 0.f; // spawn first immediately
    // set HP for wave (base + wave * scale)
    float hp = enemyBaseHP + currentWave * enemyHpScale;
    // store spawn HP for queued spawns
    nextSpawnHP = hp;

    // Determine enemy type composition: initial waves are enemy1, later waves use enemy2 and mixed waves
    for (int i = 0; i < count; ++i) {
        int type = 1; // default enemy1
        if (currentWave <= 2) {
            type = 1;
        } else if (currentWave == 3) {
            type = 2;
        } else if (currentWave == 4 || currentWave == 5) {
            // mostly type2, some type1
            int r = std::rand() % 100;
            type = (r < 70) ? 2 : 1; // 70% type2
        } else {
            // fully mixed 50/50 for later waves
            type = (std::rand() % 2 == 0) ? 2 : 1;
        }
        spawnQueue.push_back({type, hp});
    }
}

void Game::cleanupDeadStuff() {
    // remove dead projectiles
    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(),
            [](auto& p){ return p->dead; }),
        projectiles.end()
    );

    // remove dead enemies + track if they reached base or were killed
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [this](auto& e){
                if (!e->isAlive()) {
                    // Check if at base tile
                    const Map& m = this->getMap();
                    float ts = m.getTileSize();
                    int tx = static_cast<int>(e->getPosition().x / ts);
                    int ty = static_cast<int>(e->getPosition().y / ts);
                    
                    if (m.getTile(tx, ty) == 3) {
                        // Reached base: damage player
                        damagePlayer(1);
                    } else {
                        // Killed by tower: reward
                        money += 10;
                    }
                    return true;
                }
                return false;
            }),
        enemies.end()
    );
}

void Game::damagePlayer(int dmg) {
    playerHealth -= dmg;
    if (playerHealth <= 0) {
        gameOver = true;
    }
    // Pulse base portal visually when base is hit
    basePortalPulse = 1.0f;
}

int Game::getWaveEnemyCount(int wave) const {
    // Progressive waves: more enemies each wave (gentle growth)
    return 3 + wave;  // Wave 0:3, Wave1:4, Wave2:5, etc.
}

void Game::run() {
    currentWave = 0;
    waveTimer = 0.f;

    while (window.isOpen()) {
        // process events
        processEvents();
        float dt = clock.restart().asSeconds();

        if (gameStarted && !gameOver) {
            update(dt);
        }
        // if game is over, you can choose to display overlay and wait for start
        render();
    }
}

void Game::processEvents() {
    sf::Event ev;
    while (window.pollEvent(ev)) {
        if (ev.type == sf::Event::Closed) window.close();
        else if (ev.type == sf::Event::MouseMoved) {
            sf::Vector2f mousePos(ev.mouseMove.x, ev.mouseMove.y);
            handleMouseMove(mousePos);
        }
        else if (ev.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f mousePos(ev.mouseButton.x, ev.mouseButton.y);
            if (ev.mouseButton.button == sf::Mouse::Left) {
                if (!gameStarted) {
                    // start game on any left click when on menu
                    gameStarted = true;
                    startNewGame();
                } else {
                    handleMouseClick(mousePos);
                }
            }
        }
        else if (ev.type == sf::Event::KeyPressed) {
            // Number keys to select tower type
            if (ev.key.code == sf::Keyboard::Num1) {
                placeTower(0);  // Sniper
            } else if (ev.key.code == sf::Keyboard::Num2) {
                placeTower(1);  // Freezing
            } else if (ev.key.code == sf::Keyboard::Num3) {
                placeTower(2);  // Cannon
            } else if (ev.key.code == sf::Keyboard::Escape) {
                placingTower = false;
            }
            // Pause/resume
            if (ev.key.code == sf::Keyboard::P) {
                paused = !paused;
            } else if (ev.key.code == sf::Keyboard::Space) {
                paused = false; // resume
            }
            // Start or restart
            if (ev.key.code == sf::Keyboard::Enter) {
                if (!gameStarted) {
                    gameStarted = true;
                    startNewGame();
                } else if (gameOver) {
                    startNewGame();
                }
            }
        }
    }
}

void Game::placeTower(int towerType) {
    if (placingTower && selectedTowerType == towerType) {
        placingTower = false;  // Toggle off
    } else {
        selectedTowerType = towerType;
        placingTower = true;
    }
}

void Game::handleMouseMove(const sf::Vector2f& mousePos) {
    if (placingTower) {
        previewPos = mousePos;
    }
}

void Game::handleMouseClick(const sf::Vector2f& mousePos) {
    if (!placingTower) return;
    
    // Get tower cost based on type
    int cost = 0;
    std::unique_ptr<Tower> newTower = nullptr;
    
    // compute tile coords under mouse
    int tx = static_cast<int>(mousePos.x / map.getTileSize());
    int ty = static_cast<int>(mousePos.y / map.getTileSize());

    // basic tile validity
    if (tx < 0 || ty < 0 || tx >= map.getCols() || ty >= map.getRows()) return;
    int tileVal = map.getTile(tx, ty);
    if (tileVal == 2) return; // can't place on obstacle
    if (tileBlocked[ty][tx]) return; // can't place on existing tower
    
    // don't allow placing on the spawn or base tiles
    if (tileVal == 3 || tileVal == 4) return;

    // don't allow placement too close to spawn/base
    auto base = map.findBase();
    std::pair<int,int> spawnTile = {-1,-1};
    // find spawn
    for (int sy = 0; sy < map.getRows(); ++sy) {
        for (int sx = 0; sx < map.getCols(); ++sx) {
            if (map.getTile(sx, sy) == 4) { spawnTile = {sx, sy}; goto spawn_found; }
        }
    }
spawn_found:;
    auto distTiles = [](int ax, int ay, int bx, int by){ int dx = ax - bx; int dy = ay - by; return std::sqrt(dx*dx + dy*dy); };
    if (spawnTile.first != -1) {
        if (distTiles(tx, ty, spawnTile.first, spawnTile.second) <= placementBanRadiusTiles) return;
    }
    if (base.first != -1) {
        if (distTiles(tx, ty, base.first, base.second) <= placementBanRadiusTiles) return;
    }

    // create tower at tile center
    sf::Vector2f placementPos = map.tileCenter(tx, ty);
    
    switch (selectedTowerType) {
        case 0:  // Sniper
            cost = 75;
            newTower = std::make_unique<SniperTower>(placementPos, this);
            break;
        case 1:  // Freezing
            cost = 50;
            newTower = std::make_unique<FreezingTower>(placementPos, this);
            break;
        case 2:  // Cannon
            cost = 100;
            newTower = std::make_unique<CannonTower>(placementPos, this);
            break;
    }
    
    // Check if player has enough money
    if (newTower && money >= cost) {
        // reserve the funds first
        money -= cost;
        // mark tile blocked tentatively
        tileBlocked[ty][tx] = true;
        // recompute BFS to account for new obstacle and verify spawn has valid path
        computeBFS();
        // find spawn tile
        int sX=-1, sY=-1;
        for (int sy = 0; sy < map.getRows(); ++sy) {
            for (int sx = 0; sx < map.getCols(); ++sx) {
                if (map.getTile(sx, sy) == 4) { sX=sx; sY=sy; goto spawn_found2; }
            }
        }
spawn_found2:;
        bool spawnReachable = true;
        if (sX >=0 && sY >=0) {
            if (distance[sY][sX] == -1) spawnReachable = false;
        }
        if (!spawnReachable) {
            // revert block and refund
            tileBlocked[ty][tx] = false;
            money += cost; // refund
            // do not place the tower
        } else {
            // commit the tower
            towers.push_back(std::move(newTower));
        }
    }
    
    // Continue placing towers of same type
}

void Game::update(float dt) {
    if (paused) return;
    // Update portal animation global timer
    portalAnimTime += dt;
    // Update portal pulse states
    if (!spawnQueue.empty()) spawnPortalPulse = std::min(spawnPortalPulse + dt * 2.5f, 1.f);
    else spawnPortalPulse = std::max(spawnPortalPulse - dt * 1.2f, 0.f);
    basePortalPulse = std::max(basePortalPulse - dt * 1.4f, 0.f);

    // spawn queue handling: spawn enemies sequentially in the spawn queue
    if (!spawnQueue.empty()) {
        spawnTimer -= dt;
        if (spawnTimer <= 0.f) {
            // spawn one enemy at spawnTileX/Y
            if (spawnTileX >= 0 && spawnTileY >= 0) {
                sf::Vector2f spawnPos = map.tileCenter(spawnTileX, spawnTileY);
                // offset to avoid overlap
                float offx = (std::rand() % 3 - 1) * 8.f; // -8, 0, 8
                float offy = (std::rand() % 3) * 4.f;
                spawnPos.x += offx;
                spawnPos.y += offy;
                auto info = spawnQueue.front();
                float hp = info.hp; // hp set during spawnEnemyWave
                int type = info.type;
                auto e = std::make_shared<Enemy>(spawnPos, this, hp, type);
                enemies.push_back(e);
            }
            spawnQueue.pop_front();
            spawnTimer = spawnInterval;
        }
    }
    // Check if wave is complete (all enemies dead and no pending spawns)
    if (enemies.empty() && spawnQueue.empty() && !gameOver) {
        waveTimer += dt;
        if (waveTimer >= waveCooldown) {
            currentWave++;
            waveTimer = 0.f;
            spawnEnemyWave(getWaveEnemyCount(currentWave));
        }
    }
    
    // Update enemies (BFS-guided)
    for (auto& e : enemies) {
        e->update(dt);
    }

    // Update towers (targeting, cooldown, shooting)
    for (auto& t : towers) {
        t->update(dt, *this);
    }

    // Update projectiles (movement and collision)
    for (auto& p : projectiles) {
        p->update(dt);
    }

    // Cleanup dead objects
    cleanupDeadStuff();
}

void Game::render() {
    window.clear(sf::Color::Black);
    map.draw(window);
    // draw spawn/base portals (vortices)
    drawPortals(window);
    
    // Draw towers
    for (auto& t : towers) {
        t->render(window);
    }
    
    // Draw projectiles
    for (auto& p : projectiles) {
        p->render(window);
    }
    
    // Draw enemies
    for (auto& e : enemies) {
        e->render(window);
    }
    
    // Draw tower placement preview
    if (placingTower) {
        sf::CircleShape preview(10.f);
        preview.setPosition(previewPos - sf::Vector2f(10.f, 10.f));
        
        // Color based on selected tower type
        switch (selectedTowerType) {
            case 0: preview.setFillColor(sf::Color(200, 50, 50, 100)); break;     // Red for Sniper
            case 1: preview.setFillColor(sf::Color(100, 200, 255, 100)); break;   // Cyan for Freezing
            case 2: preview.setFillColor(sf::Color(255, 200, 0, 100)); break;     // Yellow for Cannon
        }
        preview.setOutlineColor(sf::Color::White);
        preview.setOutlineThickness(2.f);
        window.draw(preview);
    }
    
    // Draw UI
    if (ui) {
        ui->render(window);
    }
    // If the game hasn't started yet, render a welcome/start overlay
    if (!gameStarted) {
        sf::RectangleShape overlay({window.getSize().x, window.getSize().y});
        overlay.setFillColor(sf::Color(0, 0, 0, 180));
        window.draw(overlay);
        sf::Font font;
        bool loaded = font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
        if (!loaded) loaded = font.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf");
        sf::Text title("TOWER DEFENSE", font, 42);
        title.setFillColor(sf::Color::White);
        title.setPosition(window.getSize().x/2 - title.getLocalBounds().width/2, window.getSize().y*0.2f);
        window.draw(title);
        sf::Text startText("Press ENTER to Start", font, 26);
        startText.setFillColor(sf::Color::Yellow);
        startText.setPosition(window.getSize().x/2 - startText.getLocalBounds().width/2, window.getSize().y*0.6f);
        window.draw(startText);
    }

    // If game is over, show Game Over overlay and option to restart
    if (gameOver) {
        sf::RectangleShape overlay({window.getSize().x, window.getSize().y});
        overlay.setFillColor(sf::Color(0, 0, 0, 200));
        window.draw(overlay);
        sf::Font font;
        bool loaded = font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
        if (!loaded) loaded = font.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf");
        sf::Text overText("GAME OVER", font, 64);
        overText.setFillColor(sf::Color::Red);
        overText.setPosition(window.getSize().x/2 - overText.getLocalBounds().width/2, window.getSize().y*0.2f);
        window.draw(overText);
        sf::Text restartText("Press ENTER to Restart", font, 26);
        restartText.setFillColor(sf::Color::White);
        restartText.setPosition(window.getSize().x/2 - restartText.getLocalBounds().width/2, window.getSize().y*0.6f);
        window.draw(restartText);
    }

    window.display();
}

void Game::drawPortals(sf::RenderWindow& window) {
    // draw portals at spawn tile (spawnTileX/Y) and base tile
    float ts = map.getTileSize();
    auto base = map.findBase();
    std::vector<std::pair<sf::Vector2f, sf::Color>> portals;
    if (spawnTileX >= 0 && spawnTileY >= 0) {
        portals.push_back({map.tileCenter(spawnTileX, spawnTileY), sf::Color(200,50,50)}); // red spawn
    } else {
        // try to find spawn tile by scanning
        for (int y=0; y<map.getRows(); ++y) {
            for (int x=0; x<map.getCols(); ++x) {
                if (map.getTile(x,y) == 4) portals.push_back({map.tileCenter(x,y), sf::Color(200,50,50)});
            }
        }
    }
    if (base.first != -1 && base.second != -1) {
        portals.push_back({map.tileCenter(base.first, base.second), sf::Color(70,130,180)}); // blue base
    }

    auto hsv2rgb = [](float h, float s, float v) -> sf::Color {
        while (h < 0) h += 360.f;
        while (h >= 360.f) h -= 360.f;
        s = std::clamp(s, 0.f, 1.f);
        v = std::clamp(v, 0.f, 1.f);
        float c = v * s;
        float x = c * (1.f - std::fabs(std::fmod(h / 60.0f, 2.f) - 1.f));
        float m = v - c;
        float r=0.f, g=0.f, b=0.f;
        if (h < 60) { r=c; g=x; b=0; }
        else if (h < 120) { r=x; g=c; b=0; }
        else if (h < 180) { r=0; g=c; b=x; }
        else if (h < 240) { r=0; g=x; b=c; }
        else if (h < 300) { r=x; g=0; b=c; }
        else { r=c; g=0; b=x; }
        uint8_t R = static_cast<uint8_t>(std::round((r + m) * 255));
        uint8_t G = static_cast<uint8_t>(std::round((g + m) * 255));
        uint8_t B = static_cast<uint8_t>(std::round((b + m) * 255));
        return sf::Color(R, G, B);
    };

    for (auto &p : portals) {
        sf::Vector2f center = p.first;
        sf::Color color = p.second;
        float baseHue = (color.r > color.b) ? 20.f : 220.f;
        float portalOffset = (center.x + center.y) * 0.123f;
        for (int i = 0; i < 6; ++i) {
            float radius = ts * (0.18f + i * 0.12f);
            sf::CircleShape ring(radius);
            ring.setOrigin(radius, radius);
            ring.setPosition(center);
            ring.setFillColor(sf::Color::Transparent);
            float thickness = ts * (0.06f + i * 0.02f);
            ring.setOutlineThickness(thickness);
            float localPulse = 0.f;
            if (color.r > color.b) localPulse = spawnPortalPulse;
            else localPulse = basePortalPulse;
            int alpha = static_cast<int>(140 + 120 * std::sin(portalAnimTime * (0.8f + i*0.4f) + portalOffset) + 80 * localPulse);
            alpha = std::clamp(alpha, 50, 255);
            float hue = baseHue + 20.f * std::sin(portalAnimTime * 0.7f + i * 0.3f + portalOffset);
            sf::Color hueColor = hsv2rgb(hue, 0.9f, 0.9f);
            hueColor.a = static_cast<sf::Uint8>(alpha);
            ring.setOutlineColor(hueColor);
            ring.setRotation(portalAnimTime * (20.f + i * 40.f));
            float puls = 1.f + 0.06f * std::sin(portalAnimTime * (1.2f + i * 0.4f));
            ring.setScale(puls, puls);
            window.draw(ring);
        }
        float radiusC = ts * (0.14f + 0.12f * localPulse);
        sf::CircleShape centerDisc(radiusC);
        centerDisc.setOrigin(radiusC, radiusC);
        centerDisc.setPosition(center);
        int alphaC = static_cast<int>(200 + 55 * std::sin(portalAnimTime * 2.2f));
        alphaC = std::clamp(alphaC, 60, 255);
        float centerHue = baseHue + 40.f * std::sin(portalAnimTime * 1.2f + portalOffset);
        sf::Color fill = hsv2rgb(centerHue, 1.f, 0.95f);
        fill.a = static_cast<sf::Uint8>(alphaC + 60 * localPulse);
        fill.r = std::min(255, fill.r + 20);
        fill.b = std::min(255, fill.b + 20);
        centerDisc.setFillColor(fill);
        window.draw(centerDisc);
        int nArms = 7;
        float armLen = ts * 0.85f;
        float armWidth = ts * 0.08f;
        for (int a = 0; a < nArms; ++a) {
            float t = portalAnimTime * 1.4f + a * (2 * 3.14159f / nArms);
            float angleDeg = std::fmod(t, 2*3.14159f) * 180.f / 3.14159f;
            float rInner = ts * 0.2f + 0.08f * ts * std::sin(t * 0.6f + a);
            sf::ConvexShape wedge;
            wedge.setPointCount(3);
            wedge.setPoint(0, sf::Vector2f(0.f, 0.f));
            wedge.setPoint(1, sf::Vector2f(armLen, -armWidth));
            wedge.setPoint(2, sf::Vector2f(armLen, armWidth));
            wedge.setOrigin(0, 0);
            wedge.setPosition(center + sf::Vector2f(std::cos(t) * rInner, std::sin(t) * rInner));
            wedge.setRotation(angleDeg);
            float armHue = baseHue + 40.f * std::sin(portalAnimTime * 2.0f + a + portalOffset);
            sf::Color armColor = hsv2rgb(armHue, 0.95f, 0.95f);
            armColor.a = static_cast<sf::Uint8>(120 + 80 * std::sin(portalAnimTime + a));
            wedge.setFillColor(armColor);
            window.draw(wedge);
        }
    }
}
