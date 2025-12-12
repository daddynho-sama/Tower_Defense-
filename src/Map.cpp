#include "Map.h"
#include <filesystem>
#include <fstream>
#include <sstream>

bool Map::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;
    tiles.clear();
    rows = 0;
    cols = 0;
    std::string line;
    while (std::getline(file, line)){
        std::istringstream ss(line);
        std::vector<int>values;
        int v;
        while (ss>>v) values.push_back(v);
        if (values.empty()) continue;
        if (rows == 0) cols = values.size();
        for (int val : values) tiles.push_back(val);
        rows++;
    }
    file.close();
    // attempt to load tile textures as well
    loadTileTextures();
    return true;
}

bool Map::saveToFile(const std::string& filename)  {
    std::ofstream file(filename);
    if (!file.is_open()) return false;
    // basic consistency check
    if (cols <= 0 || rows <= 0) return false;
    if (tiles.size() < static_cast<size_t>(rows * cols)) return false;
    for (int y=0;y<rows;y++) {
        for (int x=0;x<cols;x++) {
            file << tiles[y*cols + x] << " ";
        }
        file << "\n";
    }
    file.close();
    return true;
}


Map::Map(int c, int r, float tsize) : cols(c), rows(r), tileSize(tsize) {
    tiles.assign(cols*rows, 0);
    for (int x=0;x<cols;x++) tiles[(rows/2)*cols + x] = 1;
}

Map::Map(float tsize) : cols(0), rows(0), tileSize(tsize) {
    // empty map by default; use Map(cols, rows, tileSize) to create a grid
}

void Map::setTile(int tx, int ty, int value) {
    if (tx < 0 || ty < 0 || tx >= cols || ty >= rows) return;
    tiles[ty*cols + tx] = value;
}

int Map::getTile(int tx, int ty) const {
    if (tx < 0 || ty < 0 || tx >= cols || ty >= rows) return 0;
    return tiles[ty*cols + tx];
}

void Map::draw(sf::RenderWindow& window) {
    sf::RectangleShape rect({tileSize, tileSize});
    sf::Sprite sprite;
    for (int y=0;y<rows;y++) {
        for (int x=0;x<cols;x++) {
            int v = tiles[y*cols + x];
            rect.setPosition(x*tileSize, y*tileSize);
            switch (v) {
                case 0: // herbe (vert) -> grass2 texture
                    if (texturesLoaded && grassTexture.getSize().x > 0) {
                        sprite.setTexture(grassTexture);
                        sprite.setScale(tileSize / grassTexture.getSize().x, tileSize / grassTexture.getSize().y);
                        sprite.setOrigin(0,0);
                        sprite.setPosition(x*tileSize, y*tileSize);
                        window.draw(sprite);
                    } else {
                        rect.setFillColor(sf::Color(50,180,50));
                        window.draw(rect);
                    }
                    break;
                case 1: // chemin (gris) -> paving1 texture
                    if (texturesLoaded && pavingTexture.getSize().x > 0) {
                        sprite.setTexture(pavingTexture);
                        sprite.setScale(tileSize / pavingTexture.getSize().x, tileSize / pavingTexture.getSize().y);
                        sprite.setOrigin(0,0);
                        sprite.setPosition(x*tileSize, y*tileSize);
                        window.draw(sprite);
                    } else {
                        rect.setFillColor(sf::Color(180,180,180));
                        window.draw(rect);
                    }
                    break;
                case 2: // obstacles (marron) -> stone wall texture
                    if (texturesLoaded && stoneTexture.getSize().x > 0) {
                        sprite.setTexture(stoneTexture);
                        sprite.setScale(tileSize / stoneTexture.getSize().x, tileSize / stoneTexture.getSize().y);
                        sprite.setOrigin(0,0);
                        sprite.setPosition(x*tileSize, y*tileSize);
                        window.draw(sprite);
                    } else {
                        rect.setFillColor(sf::Color(150,120,80));
                        window.draw(rect);
                    }
                    break;
                case 3: // base finale (bleu)
                    rect.setFillColor(sf::Color(70,130,180));
                    window.draw(rect);
                    break;
                case 4: // spawn (rouge)
                    rect.setFillColor(sf::Color(200,50,50));
                    window.draw(rect);
                    break;
                default:
                    rect.setFillColor(sf::Color::Magenta);
                    window.draw(rect);
                    break;
            }
        }
    }
}

void Map::loadTileTextures() {
    texturesLoaded = false;
    bool ok1 = false, ok2 = false, ok3 = false;
    namespace fs = std::filesystem;
    if (fs::exists("assets/tiles/grass2.png")) ok1 = grassTexture.loadFromFile("assets/tiles/grass2.png");
    else if (fs::exists("../assets/tiles/grass2.png")) ok1 = grassTexture.loadFromFile("../assets/tiles/grass2.png");
    if (fs::exists("assets/tiles/paving 1.png")) ok2 = pavingTexture.loadFromFile("assets/tiles/paving 1.png");
    else if (fs::exists("../assets/tiles/paving 1.png")) ok2 = pavingTexture.loadFromFile("../assets/tiles/paving 1.png");
    if (fs::exists("assets/tiles/stone wall 10.png")) ok3 = stoneTexture.loadFromFile("assets/tiles/stone wall 10.png");
    else if (fs::exists("../assets/tiles/stone wall 10.png")) ok3 = stoneTexture.loadFromFile("../assets/tiles/stone wall 10.png");
    if (ok1 || ok2 || ok3) texturesLoaded = true;
}

std::pair<int,int> Map::findBase() const {
    for (int y=0;y<rows;y++) {
        for (int x=0;x<cols;x++) {
            if (tiles[y*cols + x] == 3) {
                return {x,y};
            }
        }
    }
    return {-1,-1};
}

sf::Vector2f Map::tileCenter(int tx, int ty) const {
    return {(tx+0.5f)*tileSize, (ty+0.5f)*tileSize};
}

float Map::getTileSize() const { return tileSize; }
