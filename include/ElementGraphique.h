#ifndef ELEMENTGRAPHIQUE_HPP
#define ELEMENTGRAPHIQUE_HPP
#pragma once
#include <SFML/Graphics.hpp>

class ElementGraphique {
public:
    virtual ~ElementGraphique() = default;
    virtual void update(float dt) = 0;
    virtual void render(sf::RenderWindow& window) = 0;
    virtual sf::Vector2f getPosition() const = 0;
};


#endif /* ELEMENTGRAPHIQUE_HPP */
