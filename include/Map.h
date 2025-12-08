#ifndef MAP_HPP
#define MAP_HPP
#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class Map {
private:
    int cols, rows;
    float tileSize;
    std::vector<int> tiles;
public:
    Map(float tileSize = 32.f);
    Map(int cols, int rows, float tileSize);
    bool loadFromFile(const std::string& filename);
    bool saveToFile(const std::string& filename) ;
    void setTile(int tx, int ty, int value);
    int getTile(int tx, int ty) const;
    sf::Vector2f tileCenter(int tx, int ty) const;
    float getTileSize() const;
    void draw(sf::RenderWindow& window);
    int getCols() const { return cols; }
    int getRows() const { return rows; }
    std::pair<int,int> findBase() const;




};

#endif /* MAP_HPP */
