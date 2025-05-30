#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <stack>
#include <chrono>
#include <queue>
#include <iostream>
#include <SFML/Window.hpp>
using namespace std;
using namespace sf;

class SlidingPuzzle {
public:
    SlidingPuzzle() : m_window(VideoMode(900, 600), "Sliding Puzzle") {
        if (!m_font.loadFromFile("C:/Users/T L S/Downloads/LunaLogic/DSA PROJECT/assets/ARIAL.TTF")) {
            cerr << "Failed to load font\n";
        }
        if (!m_backgroundTexture.loadFromFile("assets/pic2.jpg")) {
            cerr << "Failed to load background image\n";
        }
        else {
            m_backgroundSprite.setTexture(m_backgroundTexture);

            // Scale the background to fit window
            FloatRect bgBounds = m_backgroundSprite.getLocalBounds();
            float scaleX = (float)m_window.getSize().x / bgBounds.width;
            float scaleY = (float)m_window.getSize().y / bgBounds.height;
            m_backgroundSprite.setScale(scaleX, scaleY);
        }


        resetGame(3); // Start with 3x3 puzzle
    }

    void run() {
        while (m_window.isOpen()) {
            handleEvents();
            update();
            render();
        }
    }

    std::stack<std::pair<sf::Vector2i, sf::Vector2i>> m_undoStack;
    std::queue<std::pair<sf::Vector2i, sf::Vector2i>> m_moveHistory;
    const int MAX_HISTORY = 10;  // Keep last 10 moves

private:
    RenderWindow m_window;
    Font m_font;
    vector<vector<int>> m_board;
    Vector2i m_emptyPos;
    int m_size;
    int m_moves;
    bool m_victory;
    Clock m_clock;
    string m_message;
    Clock m_messageTimer;
    Texture m_backgroundTexture;
    Sprite m_backgroundSprite;
 



    void resetGame(int size) {
        
        m_size = size;
        m_board.assign(size, vector<int>(size));
        m_moves = 0;
        m_victory = false;

        int num = 1;
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                m_board[i][j] = num++;
            }
        }
        m_emptyPos = Vector2i(size - 1, size - 1);
        m_board[m_emptyPos.x][m_emptyPos.y] = 0;

        shuffleBoard();
    }

    void shuffleBoard() {
        random_device rd;
        mt19937 gen(rd());

        for (int i = 0; i < 1000; i++) {
            vector<Vector2i> moves;
            if (m_emptyPos.x > 0) moves.emplace_back(m_emptyPos.x - 1, m_emptyPos.y);
            if (m_emptyPos.x < m_size - 1) moves.emplace_back(m_emptyPos.x + 1, m_emptyPos.y);
            if (m_emptyPos.y > 0) moves.emplace_back(m_emptyPos.x, m_emptyPos.y - 1);
            if (m_emptyPos.y < m_size - 1) moves.emplace_back(m_emptyPos.x, m_emptyPos.y + 1);

            uniform_int_distribution<> dist(0, moves.size() - 1);
            auto move = moves[dist(gen)];
            swap(m_board[move.x][move.y], m_board[m_emptyPos.x][m_emptyPos.y]);
            m_emptyPos = move;
        }

        m_clock.restart();
        m_victory = false;
        m_message = "Shuffled!";
        m_messageTimer.restart();
    }

    void undoLastMove() {
        if (m_undoStack.empty()) return;

        auto move = m_undoStack.top();
        m_undoStack.pop();

        // Reverse the move
        std::swap(m_board[move.first.x][move.first.y], m_board[move.second.x][move.second.y]);
        m_emptyPos = move.second;
        m_moves--; // Optional: reduce move count

        // Optional: display undo message
        m_message = "Undo move!";
        m_messageTimer.restart();
    }

    void handleEvents() {
        Event event;
        while (m_window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                m_window.close();
            }

            if (event.type == Event::MouseButtonPressed) {
                if (event.mouseButton.button == Mouse::Left) {
                    if (m_victory) return;
                    handleClick(event.mouseButton.x, event.mouseButton.y);
                }
            }

            if (event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::Space) {
                    shuffleBoard();
                }
                else if (event.key.code == Keyboard::Num1) {
                    resetGame(3);
                    m_message = "3x3 Mode";
                    m_messageTimer.restart();
                }
                else if (event.key.code == Keyboard::Num2) {
                    resetGame(4);
                    m_message = "4x4 Mode";
                    m_messageTimer.restart();
                }
               /* else if (!m_victory) {
                    moveEmpty(event.key.code);
                }*/
                else if (event.key.code == sf::Keyboard::Escape) {
                    m_window.close();  // Quit the game
                }
                else if (event.key.code == Keyboard::Z) { // Ctrl+Z optional
                    undoLastMove();
                }

            }
        }
    }

    void handleClick(int x, int y) {
        int tileSize = 100;
        int offsetX = (m_window.getSize().x - m_size * tileSize) / 2;
        int offsetY = (m_window.getSize().y - m_size * tileSize) / 2;

        if (x < offsetX || y < offsetY ||
            x >= offsetX + m_size * tileSize ||
            y >= offsetY + m_size * tileSize) return;

        int row = (y - offsetY) / tileSize;
        int col = (x - offsetX) / tileSize;

        if ((abs(row - m_emptyPos.x) == 1 && col == m_emptyPos.y) ||
            (abs(col - m_emptyPos.y) == 1 && row == m_emptyPos.x)) {

            // Store undo before the move
            m_undoStack.push({ m_emptyPos, {row, col} });

            // Store move history after the move
            m_moveHistory.push({ {row, col}, m_emptyPos });
            if (m_moveHistory.size() > MAX_HISTORY)
                m_moveHistory.pop();

            std::swap(m_board[row][col], m_board[m_emptyPos.x][m_emptyPos.y]);
            m_emptyPos = sf::Vector2i(row, col);
            m_moves++;

            checkVictory();
        }

        std::pair<sf::Vector2i, sf::Vector2i> move = { m_emptyPos, {row, col} };
        m_undoStack.push(move);
        m_moveHistory.push(move); // Optional: For display


    }

    void moveEmpty(Keyboard::Key key) {
        Vector2i dir(0, 0);
        if (key == Keyboard::Up) dir = Vector2i(1, 0);
        else if (key == Keyboard::Down) dir = Vector2i(-1, 0);
        else if (key == Keyboard::Left) dir = Vector2i(0, 1);
        else if (key == Keyboard::Right) dir = Vector2i(0, -1);

        Vector2i target = m_emptyPos + dir;
        if (target.x >= 0 && target.x < m_size && target.y >= 0 && target.y < m_size) {
            swap(m_board[target.x][target.y], m_board[m_emptyPos.x][m_emptyPos.y]);
            m_emptyPos = target;
            m_moves++;
            checkVictory();
        }
    }

    void checkVictory() {
        int num = 1;
        for (int i = 0; i < m_size; i++) {
            for (int j = 0; j < m_size; j++) {
                if (i == m_size - 1 && j == m_size - 1) {
                    if (m_board[i][j] != 0) return;
                }
                else if (m_board[i][j] != num++) {
                    return;
                }
            }
        }
        m_victory = true;
    }

    void update() {}

    void render() {
        m_window.clear();
        m_window.draw(m_backgroundSprite); // Draw background first

        m_window.clear(Color(50, 50, 80)); // Can remove this color if background image is opaque
        m_window.draw(m_backgroundSprite); // Draw background first

        int tileSize = 100;
        int offsetX = (m_window.getSize().x - m_size * tileSize) / 2;
        int offsetY = (m_window.getSize().y - m_size * tileSize) / 2;

        RectangleShape boardBg(Vector2f(m_size * tileSize, m_size * tileSize));
        boardBg.setPosition(offsetX, offsetY);
        boardBg.setFillColor(Color(70, 70, 100));
        m_window.draw(boardBg);

        for (int i = 0; i < m_size; i++) {
            for (int j = 0; j < m_size; j++) {
                if (m_board[i][j] != 0) {
                    RectangleShape tile(Vector2f(tileSize - 4, tileSize - 4));
                    tile.setPosition(offsetX + j * tileSize + 2, offsetY + i * tileSize + 2);
                    tile.setFillColor(getTileColor(m_board[i][j]));
                    tile.setOutlineThickness(2);
                    tile.setOutlineColor(Color::White);
                    m_window.draw(tile);

                    Text text(to_string(m_board[i][j]), m_font, 36);
                    text.setFillColor(Color::White);
                    FloatRect bounds = text.getLocalBounds();
                    text.setOrigin(bounds.width / 2, bounds.height / 2 + bounds.top);
                    text.setPosition(offsetX + j * tileSize + tileSize / 2,
                        offsetY + i * tileSize + tileSize / 2);
                    m_window.draw(text);
                }
            }
        }

        Text movesText("Moves: " + to_string(m_moves), m_font, 24);
        movesText.setPosition(20, 20);
        m_window.draw(movesText);

        Text timeText("Time: " + to_string((int)m_clock.getElapsedTime().asSeconds()) + "s", m_font, 24);
        timeText.setPosition(20, 50);
        m_window.draw(timeText);

        // Show last move from queue
        if (!m_moveHistory.empty()) {
            auto lastMove = m_moveHistory.back();
            string moveStr = "Last Move: (" +
                to_string(lastMove.first.x) + "," + to_string(lastMove.first.y) + ") ? (" +
                to_string(lastMove.second.x) + "," + to_string(lastMove.second.y) + ")";

            Text lastMoveText(moveStr, m_font, 20);
            lastMoveText.setPosition(20, 80);
            lastMoveText.setFillColor(Color::Green);
            m_window.draw(lastMoveText);
        }


        Text controls("Space: Shuffle  1: 3x3  2: 4x4  ESC: Quit", m_font, 24);
        controls.setPosition(20, m_window.getSize().y - 40);
        m_window.draw(controls);

        if (!m_message.empty() && m_messageTimer.getElapsedTime().asSeconds() < 2) {
            Text msg(m_message, m_font, 30);
            FloatRect b = msg.getLocalBounds();
            msg.setOrigin(b.width / 2, b.height / 2 + b.top);
            msg.setPosition(m_window.getSize().x / 2, 40);
            msg.setFillColor(Color::Cyan);
            m_window.draw(msg);
        }

        if (m_victory) renderVictoryScreen();

        m_window.display();
    }

    void renderVictoryScreen() {
        RectangleShape overlay(Vector2f(m_window.getSize().x, m_window.getSize().y));
        overlay.setFillColor(Color(0, 0, 0, 180));
        m_window.draw(overlay);

        Text victoryText("You Win!", m_font, 72);
        FloatRect vb = victoryText.getLocalBounds();
        victoryText.setOrigin(vb.width / 2, vb.height / 2 + vb.top);
        victoryText.setPosition(m_window.getSize().x / 2, m_window.getSize().y / 2 - 100);
        victoryText.setFillColor(Color::Yellow);
        m_window.draw(victoryText);

        Text statsText("Moves: " + to_string(m_moves) + "  Time: " + to_string((int)m_clock.getElapsedTime().asSeconds()) + "s", m_font, 36);
        FloatRect sb = statsText.getLocalBounds();
        statsText.setOrigin(sb.width / 2, sb.height / 2 + sb.top);
        statsText.setPosition(m_window.getSize().x / 2, m_window.getSize().y / 2);
        m_window.draw(statsText);

        Text restartText("Press Space to play again", m_font, 24);
        FloatRect rb = restartText.getLocalBounds();
        restartText.setOrigin(rb.width / 2, rb.height / 2 + rb.top);
        restartText.setPosition(m_window.getSize().x / 2, m_window.getSize().y / 2 + 80);
        m_window.draw(restartText);
    }

    Color getTileColor(int value) {
        float ratio = (float)value / (m_size * m_size);
        return Color(100 + (int)(155 * ratio), 100, 100 + (int)(155 * (1 - ratio)));
    }
};


