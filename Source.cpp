#include <SFML/Graphics.hpp>
#include <iostream>
#include"crossword.h"
#include"SlidingPuzzle.h"
#include"Hanoi.h"
#include"Questions.h"
using namespace std;
using namespace sf;

const int MENU_ITEMS = 4;

int launchquestions() {
    TwentyQuestions game;
    game.run();
    return 0;
}
int launchcrossword() {
    CrosswordGame game;
    game.run();
    return 0;
}

int launchpuzzle() {
    SlidingPuzzle game;
    game.run();
    return 0;
}

int launchhanoi() {
    RenderWindow window(VideoMode(1200, 600), "Tower of Hanoi");
    window.setFramerateLimit(60);

    HanoiGame game(5);

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) window.close();
            game.handleEvent(event, window);
        }

        window.clear(Color(70, 130, 180));
        game.draw(window);
        window.display();
    }

    return 0;
}




int main() {
    RenderWindow window(VideoMode(900, 600), "LunaLogic - Main Menu");

    Texture bgTexture;
    if (!bgTexture.loadFromFile("assets/pic2.jpg")) {
        cout << "Failed to load background image.\n";
        return 1;
    }
    Sprite background(bgTexture);
    background.setScale(
        (float)window.getSize().x / bgTexture.getSize().x,
        (float)window.getSize().y / bgTexture.getSize().y
    );

    Font font;
    if (!font.loadFromFile("assets/ARIAL.TTF")) {
        cout << "Error loading font.\n";
        return 1;
    }

    Texture puzzleTextures[MENU_ITEMS];
    string textureFiles[MENU_ITEMS] = {
       "C:/Users/T L S/Downloads/LunaLogic/DSA PROJECT/assets/pic3.jpg",
        "C:/Users/T L S/Downloads/LunaLogic/DSA PROJECT/assets/pic3.jpg",
        "C:/Users/T L S/Downloads/LunaLogic/DSA PROJECT/assets/pic3.jpg",
        "C:/Users/T L S/Downloads/LunaLogic/DSA PROJECT/assets/pic3.jpg"
    };

    Sprite puzzleIcons[MENU_ITEMS];
    for (int i = 0; i < MENU_ITEMS; ++i) {
        if (!puzzleTextures[i].loadFromFile(textureFiles[i])) {
            cout << "Failed to load " << textureFiles[i] << endl;
            return 1;
        }
        puzzleIcons[i].setTexture(puzzleTextures[i]);
        puzzleIcons[i].setScale(0.6f, 0.6f);
        puzzleIcons[i].setPosition(75 + i * 210, 400);
    }

    string labels[MENU_ITEMS] = {
        "Tower of Hanoi", "Sliding Puzzle", "Crossword", "20 Questions"
    };

    Text menuText[MENU_ITEMS];
    for (int i = 0; i < MENU_ITEMS; ++i) {
        menuText[i].setFont(font);
        menuText[i].setString(labels[i]);
        menuText[i].setCharacterSize(22);
        menuText[i].setFillColor(Color::White);
        FloatRect bounds = puzzleIcons[i].getGlobalBounds();
        menuText[i].setPosition(
            bounds.left + (bounds.width - menuText[i].getGlobalBounds().width) / 2,
            bounds.top + bounds.height + 10
        );
    }

    // Title Text
    Text title("LunaLogic", font, 80);
    title.setFillColor(Color::Magenta);
    title.setOutlineColor(Color::Black);
    title.setOutlineThickness(3);
    title.setPosition(270, 50);

    // Shadow
    Text shadow = title;
    shadow.setFillColor(Color(100, 0, 100)); // Dark magenta
    shadow.setPosition(274, 54);

    // Glow Effect
    Text glow = title;
    glow.setFillColor(Color(255, 0, 255, 100)); // Semi-transparent
    glow.setCharacterSize(90);
    glow.setPosition(265, 45);

    // Background Element
    RectangleShape bg(Vector2f(500, 100));
    bg.setFillColor(Color(0, 0, 0, 150));
    bg.setOutlineColor(Color::White);
    bg.setOutlineThickness(2);
    bg.setPosition(260, 45);


    while (window.isOpen()) {
        Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));

        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();

            if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                for (int i = 0; i < MENU_ITEMS; ++i) {
                    if (puzzleIcons[i].getGlobalBounds().contains(mousePos)) {
                        cout << "Launching: " << labels[i] << endl;

                        switch (i) {
                        case 0: launchhanoi(); break;
                        case 1: launchpuzzle(); break;
                        case 2: launchcrossword(); break;
                        case 3: launchquestions(); break;
                        }
                    }
                }
            }
        }

        window.clear();
        window.draw(background);
        window.draw(title);
        for (int i = 0; i < MENU_ITEMS; ++i) {
            window.draw(puzzleIcons[i]);
            window.draw(menuText[i]);
        }
        window.display();
    }

    return 0;
}




