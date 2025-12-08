#include "Game.h"
#include "Tower.h"
#include "Enemy.h"
#include "Projectile.h"
#include <deque>

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

    // compute BFS distance map once for all enemies (must be done after the map is loaded)
    computeBFS();

    // Place a few towers with different costs for testing
    towers.push_back(std::make_unique<Tower>(map.tileCenter(2, 2), 50, this));  // cheap
    towers.push_back(std::make_unique<Tower>(map.tileCenter(6, 2), 75, this));  // medium
    towers.push_back(std::make_unique<Tower>(map.tileCenter(2, 4), 100, this)); // expensive
    
    // Spawn first wave of enemies
    spawnEnemyWave(5);
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
                spawnTx = x; spawnTy = y; break;
            }
        }
        if (spawnTx != -1) break;
    }

    if (spawnTx == -1) {
        spawnTx = 0; spawnTy = std::min(map.getRows()-1, 6);
    }

    for (int i = 0; i < count; ++i) {
        auto e = std::make_shared<Enemy>(map.tileCenter(spawnTx, spawnTy), this);
        enemies.push_back(e);
    }
}

void Game::cleanupDeadStuff() {
    // remove dead projectiles
    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(),
            [](auto& p){ return p->dead; }),
        projectiles.end()
    );

    // remove dead enemies + reward
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [this](auto& e){
                if (!e->isAlive()) {
                    money += 10; // reward
                    return true;
                }
                return false;
            }),
        enemies.end()
    );
}

void Game::run() {
    while (window.isOpen()) {
        processEvents();
        float dt = clock.restart().asSeconds();
        update(dt);
        render();
    }
}

void Game::processEvents() {
    sf::Event ev;
    while (window.pollEvent(ev)) {
        if (ev.type == sf::Event::Closed) window.close();
        // input futur : construction tours via clic, pause, etc.
    }
}

void Game::update(float dt) {
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
    
    window.display();
}
