#include <SFML/Graphics.hpp>
#include <stack>
#include <vector>
#include <iostream>
#include <sstream>
using namespace std;
using namespace sf;

class Disk {
public:
    RectangleShape shape;
    int size;

    Disk() = default;

    Disk(float width, float height, Color color, int size) : size(size) {
        shape.setSize(Vector2f(width, height));
        shape.setFillColor(color);
        shape.setOutlineThickness(2.f);
        shape.setOutlineColor(Color(255, 255, 255, 150));
        shape.setOrigin(width / 2, height / 2);
    }
};

class Tower {
private:
    stack<Disk> disks;
    Vector2f position;
    float width, height;
    RectangleShape rod, base;
    CircleShape rodCap;

public:
    Tower(Vector2f pos, float w, float h) : position(pos), width(w), height(h) {
        rod.setSize(Vector2f(16, height));
        rod.setFillColor(Color(160, 82, 45));
        rod.setOrigin(8, height);
        rod.setPosition(position);

        rodCap.setRadius(12);
        rodCap.setFillColor(Color(139, 69, 19));
        rodCap.setOrigin(12, 12);
        rodCap.setPosition(position.x, position.y - height);

        base.setSize(Vector2f(180, 16));
        base.setFillColor(Color(139, 69, 19));
        base.setOrigin(90, 0);
        base.setPosition(position.x, position.y);
    }

    void push(Disk d) { disks.push(d); }
    Disk pop() { Disk d = disks.top(); disks.pop(); return d; }
    bool empty() const { return disks.empty(); }
    Disk& top() { return disks.top(); }
    const Disk& top() const { return disks.top(); }
    size_t size() const { return disks.size(); }

    void draw(RenderWindow& window) const {
        window.draw(base);
        window.draw(rod);
        window.draw(rodCap);

        stack<Disk> temp = disks;
        vector<Disk> render;
        while (!temp.empty()) {
            render.push_back(temp.top());
            temp.pop();
        }

        for (int i = render.size() - 1; i >= 0; i--) {
            float y = position.y - (render.size() - 1 - i) * render[i].shape.getSize().y;
            render[i].shape.setPosition(position.x, y);
            window.draw(render[i].shape);
        }
    }

    bool contains(Vector2f point) const {
        float halfWidth = width / 2;
        return (point.x >= position.x - halfWidth && point.x <= position.x + halfWidth)
            && (point.y >= position.y - height && point.y <= position.y + 20);
    }

    Vector2f getPosition() const { return position; }
};

class HanoiGame {
private:
    vector<Tower> towers;
    Disk draggedDisk;
    Tower* sourceTower = nullptr;
    bool isDragging = false;
    int diskCount, moveCount = 0;
    Font font;
    Text moveText, instructionText, winText;
    Texture bgTexture;
    Sprite background;

    vector<Color> diskColors = {
        Color(255, 0, 0),      // Red
        Color(255, 153, 0),    // Orange
        Color(255, 255, 0),    // Yellow
        Color(0, 204, 0),      // Green
        Color(0, 128, 255),    // Blue
        Color(102, 0, 204),    // Indigo
        Color(255, 0, 255)     // Violet
    };

    void initializeTowers() {
        towers.clear();
        float towerWidth = 250, towerHeight = 400;
        float startX = 200, y = 500;

        towers.emplace_back(Vector2f(startX, y), towerWidth, towerHeight);
        towers.emplace_back(Vector2f(startX + 400, y), towerWidth, towerHeight);
        towers.emplace_back(Vector2f(startX + 800, y), towerWidth, towerHeight);

        float diskHeight = 30;
        float maxWidth = 200, minWidth = 60;
        float step = (maxWidth - minWidth) / diskCount;

        for (int i = 0; i < diskCount; ++i) {
            float w = maxWidth - i * step;
            Color diskColor = diskColors[i % diskColors.size()];
            Disk d(w, diskHeight, diskColor, diskCount - i);
            towers[0].push(d);
        }

        updateMoveText();
    }

    void updateMoveText() {
        stringstream ss;
        ss << "Moves: " << moveCount;
        moveText.setString(ss.str());
    }

public:
    HanoiGame(int count) : diskCount(count) {
        if (!font.loadFromFile("assets/ARIAL.TTF")) {
            cerr << "Failed to load font\n";
        }
        if (!bgTexture.loadFromFile("assets/pic2.jpg")) {
            cerr << "Failed to load background texture\n";
        }
        background.setTexture(bgTexture);
        background.setScale(1200.f / bgTexture.getSize().x, 600.f / bgTexture.getSize().y);

        moveText.setFont(font);
        moveText.setCharacterSize(24);
        moveText.setFillColor(Color::White);
        moveText.setPosition(30, 20);

        instructionText = moveText;
        instructionText.setCharacterSize(20);
        instructionText.setString("Drag & Drop Discs | Press R = Reset | Esc = Quit");
        instructionText.setPosition(30, 60);

        winText.setFont(font);
        winText.setCharacterSize(40);
        winText.setFillColor(Color::Yellow);
        winText.setStyle(Text::Bold);
        winText.setString("?? Puzzle Solved!");
        FloatRect w = winText.getLocalBounds();
        winText.setOrigin(w.width / 2, w.height / 2);
        winText.setPosition(600, 100);

        initializeTowers();
    }

    bool isSolved() const {
        return towers[2].size() == diskCount;
    }

    void handleEvent(Event& event, RenderWindow& window) {
        Vector2f mouse = window.mapPixelToCoords(Mouse::getPosition(window));

        if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
            for (auto& tower : towers) {
                if (tower.contains(mouse) && !tower.empty()) {
                    draggedDisk = tower.top();
                    tower.pop();
                    sourceTower = &tower;
                    isDragging = true;
                    break;
                }
            }
        }
        else if (event.type == Event::MouseButtonReleased && isDragging) {
            bool moved = false;
            for (auto& tower : towers) {
                if (tower.contains(mouse)) {
                    if (tower.empty() || draggedDisk.size < tower.top().size) {
                        tower.push(draggedDisk);
                        moveCount++;
                        updateMoveText();
                        moved = true;
                    }
                    break;
                }
            }
            if (!moved) {
                sourceTower->push(draggedDisk);
            }
            isDragging = false;
            sourceTower = nullptr;
        }
        else if (event.type == Event::MouseMoved && isDragging) {
            draggedDisk.shape.setPosition(mouse.x, mouse.y);
        }
        else if (event.type == Event::KeyPressed) {
            if (event.key.code == Keyboard::R) {
                moveCount = 0;
                initializeTowers();
            }
            else if (event.key.code == Keyboard::Escape) {
                window.close();
            }
        }
    }

    void draw(RenderWindow& window) {
        window.draw(background);

        for (auto& tower : towers) tower.draw(window);
        if (isDragging) window.draw(draggedDisk.shape);
        window.draw(moveText);
        window.draw(instructionText);

        if (isSolved()) {
            window.draw(winText);
        }
    }
};


