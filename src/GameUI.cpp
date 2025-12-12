#include "GameUI.h"
#include "Game.h"

GameUI::GameUI(const Game* g) : game(g) {
    // Try to load a font (optional - if it fails, we just won't render text)
    // On Linux, common locations:
    fontLoaded = font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
    if (!fontLoaded) {
        fontLoaded = font.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf");
    }
}

std::string GameUI::getTowerName(int type) const {
    switch (type) {
        case 0: return "SNIPER (1)";
        case 1: return "FREEZING (2)";
        case 2: return "CANNON (3)";
        default: return "UNKNOWN";
    }
}

int GameUI::getTowerCost(int type) const {
    switch (type) {
        case 0: return 75;
        case 1: return 50;
        case 2: return 100;
        default: return 0;
    }
}

void GameUI::render(sf::RenderWindow& window) {
    if (!fontLoaded) return;  // Skip text rendering if font not loaded
    
    // Prepare text settings
    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(16);
    text.setFillColor(sf::Color::White);
    
    // === Top-left: Player health ===
    text.setString("Health: " + std::to_string(game->playerHealth));
    text.setPosition(10.f, 10.f);
    window.draw(text);
    
    // === Top-left +25: Wave info ===
    text.setString("Wave: " + std::to_string(game->currentWave) + 
                   " Enemies: " + std::to_string(game->enemies.size()));
    text.setPosition(10.f, 35.f);
    window.draw(text);
    
    // === Top-left +50: Money ===
    text.setCharacterSize(20);
    text.setFillColor(sf::Color::Yellow);
    text.setString("Money: $" + std::to_string(game->money));
    text.setPosition(10.f, 60.f);
    window.draw(text);
    
    // === Top-right: Controls ===
    text.setCharacterSize(14);
    text.setFillColor(sf::Color::Green);
    text.setString("1=Sniper 2=Freeze 3=Cannon ESC=Cancel");
    text.setPosition(window.getSize().x - 350.f, 10.f);
    window.draw(text);
    
    // === Top-right tower info ===
    if (game->placingTower) {
        text.setCharacterSize(16);
        text.setFillColor(sf::Color::Cyan);
        
        std::string towerName = getTowerName(game->selectedTowerType);
        int cost = getTowerCost(game->selectedTowerType);
        
        std::string infoStr = towerName + " - Cost: $" + std::to_string(cost);
        if (game->money < cost) {
            infoStr += " (NOT ENOUGH!)";
        }
        
        text.setString(infoStr);
        text.setPosition(window.getSize().x - 350.f, 35.f);
        window.draw(text);
    }
    
    // === Bottom-left: Game over message ===
    if (game->gameOver) {
        text.setCharacterSize(30);
        text.setFillColor(sf::Color::Red);
        text.setString("GAME OVER!");
        text.setPosition(window.getSize().x / 2.f - 100.f, window.getSize().y / 2.f);
        window.draw(text);
    }
    
    // === Paused message ===
    if (game->paused) {
        text.setCharacterSize(30);
        text.setFillColor(sf::Color::White);
        text.setString("PAUSED");
        text.setPosition(window.getSize().x / 2.f - 60.f, window.getSize().y / 2.f - 40.f);
        window.draw(text);
    }
}
