#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <cctype>
#include <vector>

using namespace sf;
using namespace std;

struct Node {
    string text;
    unique_ptr<Node> yes;
    unique_ptr<Node> no;

    bool isLeaf() const {
        return !yes && !no;
    }
};

class TwentyQuestions {
private:
    RenderWindow window;
    Font font;
    Texture bgTexture;
    Sprite bgSprite;

    // UI Elements
    RectangleShape yesButton;
    RectangleShape noButton;
    RectangleShape restartButton;
    RectangleShape quitButton;
    RectangleShape questionBox;


    Text questionText;
    Text yesText;
    Text noText;
    Text restartText;
    Text quitText;
    Text titleText;

    // Game State
    unique_ptr<Node> root;
    Node* currentNode;
    const string filename = "20q_tree.txt";

    // Learning Mode
    bool learningMode = false;
    bool inputQuestionPhase = false;
    bool inputAnswerPhase = false;
    string newAnswer;
    string newQuestion;
    char yesForAnswer = 0;

    // Input Handling
    string inputBuffer;
    Text inputPromptText;
    Text inputBufferText;
    RectangleShape inputBox;
    Clock cursorClock;
    bool showCursor = true;

    // Statistics
    int gamesPlayed = 0;
    int gamesWon = 0;
    Text statsText;

public:
    TwentyQuestions() : window(VideoMode(900, 600), "20 Questions Game") {
        window.setFramerateLimit(60);

        if (!loadResources()) {
            cerr << "Failed to load resources\n";
            exit(1);
        }

        setupUI();
        loadTree();
        currentNode = root.get();
    }

    bool loadResources() {
        // Try multiple font paths
        vector<string> fontPaths = {
            "C:/Users/T L S/Downloads/LunaLogic/DSA PROJECT/assets/ARIAL.TTF",
            "C:/Users/T L S/Downloads/LunaLogic/DSA PROJECT/assets/ARIAL.TTF",
            "C:/Users/T L S/Downloads/LunaLogic/DSA PROJECT/assets/ARIAL.TTF"
        };

        for (const auto& path : fontPaths) {
            if (font.loadFromFile(path)) {
                break;
            }
        }

        if (!font.loadFromFile("C:/Users/T L S/Downloads/LunaLogic/DSA PROJECT/assets/ARIAL.TTF")) {
            cerr << "Failed to load font\n";
            return false;
        }

        // Try multiple background paths
        vector<string> bgPaths = {
            "C:/Users/T L S/Downloads/LunaLogic/DSA PROJECT/assets/pic2",
            "C:/Users/T L S/Downloads/LunaLogic/DSA PROJECT/assets/pic2",
            "C:/Users/T L S/Downloads/LunaLogic/DSA PROJECT/assets/pic2"
        };

        for (const auto& path : bgPaths) {
            if (bgTexture.loadFromFile(path)) {
                break;
            }
        }

        if (!bgTexture.loadFromFile("C:/Users/T L S/Downloads/LunaLogic/DSA PROJECT/assets/pic2.jpg")) {
            cerr << "Failed to load background image\n";
            // Continue without background if not found
            bgTexture.create(window.getSize().x, window.getSize().y);
            bgTexture.update(window);
        }

        return true;
    }

    void setupUI() {
        // Background setup
        bgSprite.setTexture(bgTexture);
        FloatRect bgBounds = bgSprite.getLocalBounds();
        bgSprite.setScale(
            window.getSize().x / bgBounds.width,
            window.getSize().y / bgBounds.height
        );

        // Title setup
        titleText.setFont(font);
        titleText.setString("20 Questions Game");
        titleText.setCharacterSize(42);
        titleText.setFillColor(Color::White);
        titleText.setStyle(Text::Bold | Text::Underlined);
        titleText.setOutlineColor(Color::Black);
        titleText.setOutlineThickness(2);
        FloatRect titleBounds = titleText.getLocalBounds();
        titleText.setOrigin(titleBounds.width / 2.25, titleBounds.height / 2.25);
        titleText.setPosition(window.getSize().x / 2, 50);

        // Question text box background
        questionBox.setSize(Vector2f(680, 100));
        questionBox.setFillColor(Color(0, 0, 0, 150));
        questionBox.setPosition(60, 140);

        // Question text
        questionText.setFont(font);
        questionText.setCharacterSize(32);
        questionText.setFillColor(Color::White);
        questionText.setStyle(Text::Bold);
        questionText.setPosition(70, 150);
        questionText.setLineSpacing(1.5f);

        // Stats text
        statsText.setFont(font);
        statsText.setCharacterSize(18);
        statsText.setFillColor(Color::White);
        statsText.setPosition(20, 20);
        updateStats();

        // Button styling lambda
        auto styleButton = [&](RectangleShape& button, Text& text, const string& label, Vector2f position, Vector2f size, Color color, int fontSize) {
            button.setSize(size);
            button.setFillColor(color);
            button.setOutlineColor(Color::Black);
            button.setOutlineThickness(2);
            button.setPosition(position);

            text.setFont(font);
            text.setString(label);
            text.setCharacterSize(fontSize);
            text.setFillColor(Color::White);
            FloatRect bounds = text.getLocalBounds();
            text.setOrigin(bounds.width / 2, bounds.height / 2);
            text.setPosition(position.x + size.x / 2, position.y + size.y / 2 - 4);
            };

        // Styled buttons
        // Styled buttons - placed in a horizontal row
        styleButton(yesButton, yesText, "Yes", { 80, 500 }, { 150, 60 }, Color(76, 175, 80), 28);         // Green
        styleButton(noButton, noText, "No", { 250, 500 }, { 150, 60 }, Color(244, 67, 54), 28);           // Red
        styleButton(restartButton, restartText, "Restart", { 420, 500 }, { 150, 60 }, Color(255, 235, 59), 24); // Yellow
        styleButton(quitButton, quitText, "Quit", { 590, 500 }, { 150, 60 }, Color(156, 39, 176), 24);    // Purple


        // Input box
        inputBox.setSize(Vector2f(680, 50));
        inputBox.setFillColor(Color(220, 220, 220, 200));
        inputBox.setOutlineColor(Color::Black);
        inputBox.setOutlineThickness(2);
        inputBox.setPosition(80, 400);

        inputPromptText.setFont(font);
        inputPromptText.setCharacterSize(24);
        inputPromptText.setFillColor(Color::White);
        inputPromptText.setPosition(60, 310);

        inputBufferText.setFont(font);
        inputBufferText.setCharacterSize(28);
        inputBufferText.setFillColor(Color::Black);
        inputBufferText.setPosition(inputBox.getPosition().x + 20, inputBox.getPosition().y + 15);
    }


    void run() {
        while (window.isOpen()) {
            processEvents();
            update();
            draw();
        }
    }

    void processEvents() {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }
            else if (learningMode) {
                processLearningInput(event);
            }
            else {
                if (event.type == Event::MouseMoved) {
                    updateButtonHoverStates();
                }
                else if (event.type == Event::MouseButtonPressed) {
                    Vector2i mousePos = Mouse::getPosition(window);
                    if (yesButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        onYesClicked();
                    }
                    else if (noButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        onNoClicked();
                    }
                    else if (restartButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        currentNode = root.get();
                    }
                    else if (quitButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        window.close();
                    }
                }
            }
        }
    }

    void update() {
        if (learningMode && cursorClock.getElapsedTime().asSeconds() > 0.5f) {
            showCursor = !showCursor;
            cursorClock.restart();
        }
    }

    void updateButtonHoverStates() {
        Vector2i mousePos = Mouse::getPosition(window);

        /* auto updateButton = [](RectangleShape& button, const Color& normal, const Color& hover) {
             button.setFillColor(button.getGlobalBounds().contains(Mouse::getPosition(window).x, Mouse::getPosition(window).y) ? hover : normal);
             };*/

             /*updateButton(yesButton, Color(100, 200, 100), Color(120, 230, 120));
             updateButton(noButton, Color(200, 100, 100), Color(230, 120, 120));
             updateButton(restartButton, Color(255, 255, 100), Color(255, 255, 150));
             updateButton(quitButton, Color(200, 100, 150), Color(230, 120, 180));*/
    }

    void draw() {
        window.clear();
        window.draw(bgSprite);
        window.draw(titleText);
        window.draw(statsText);

        if (learningMode) {
            drawLearningMode();
        }
        else {
            drawGameMode();
        }

        window.display();
    }

    void drawGameMode() {
        string wrappedText = wrapText(currentNode->text, 60);
        questionText.setString(wrappedText);
        window.draw(questionText);

        window.draw(yesButton);
        window.draw(yesText);
        window.draw(noButton);
        window.draw(noText);
        window.draw(restartButton);
        window.draw(restartText);
        window.draw(quitButton);
        window.draw(quitText);
    }

    void drawLearningMode() {
        window.draw(inputPromptText);
        window.draw(inputBox);

        string displayText = inputBuffer;
        if (showCursor) {
            displayText += "|";
        }
        inputBufferText.setString(displayText);
        window.draw(inputBufferText);
    }

    string wrapText(const string& text, size_t lineLength) {
        string result;
        size_t lastSpace = 0;
        size_t currentLineLength = 0;

        for (size_t i = 0; i < text.length(); ++i) {
            if (text[i] == ' ') {
                lastSpace = i;
            }

            if (currentLineLength >= lineLength && lastSpace != 0) {
                result += '\n';
                currentLineLength = 0;
                i = lastSpace + 1;
                lastSpace = 0;
            }

            result += text[i];
            currentLineLength++;
        }

        return result;
    }

    void onYesClicked() {
        if (currentNode->isLeaf()) {
            gamesWon++;
            gamesPlayed++;
            updateStats();
            showPopup("I guessed it right! Play again?");
            currentNode = root.get();
        }
        else {
            currentNode = currentNode->yes.get();
        }
    }

    void onNoClicked() {
        if (currentNode->isLeaf()) {
            gamesPlayed++;
            updateStats();
            startLearning();
        }
        else {
            currentNode = currentNode->no.get();
        }
    }

    void updateStats() {
        float winPercentage = gamesPlayed > 0 ?
            (static_cast<float>(gamesWon) * 100.0f) / static_cast<float>(gamesPlayed) : 0.0f;

        statsText.setString("Games: " + to_string(gamesPlayed) +
            "  Wins: " + to_string(gamesWon) +
            "  Win%: " + to_string(static_cast<int>(winPercentage)));
    }

    void showPopup(const string& message) {
        Clock clock;
        RectangleShape popupBg(Vector2f(700, 200));
        popupBg.setFillColor(Color(0, 0, 0, 220));
        popupBg.setOutlineColor(Color::White);
        popupBg.setOutlineThickness(2);
        popupBg.setPosition(100, 200);

        Text msgText(message, font, 32);
        msgText.setFillColor(Color::White);
        FloatRect bounds = msgText.getLocalBounds();
        msgText.setOrigin(bounds.width / 2, bounds.height / 2);
        msgText.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f - 30);

        Text continueText("(Click anywhere to continue)", font, 20);
        continueText.setFillColor(Color::White);
        FloatRect contBounds = continueText.getLocalBounds();
        continueText.setOrigin(contBounds.width / 2, contBounds.height / 2);
        continueText.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f + 40);

        window.clear(Color::Black);
        window.draw(bgSprite);
        drawGameMode();
        window.draw(popupBg);
        window.draw(msgText);
        window.draw(continueText);
        window.display();

        bool clicked = false;
        while (clock.getElapsedTime().asSeconds() < 5.f && !clicked) {
            Event e;
            while (window.pollEvent(e)) {
                if (e.type == Event::Closed) {
                    window.close();
                    return;
                }
                else if (e.type == Event::MouseButtonPressed) {
                    clicked = true;
                }
            }
        }
    }

    void startLearning() {
        learningMode = true;
        inputQuestionPhase = true;
        inputAnswerPhase = false;
        inputBuffer.clear();
        cursorClock.restart();
        showCursor = true;

        inputPromptText.setString("I give up! What were you thinking of?");
    }

    void processLearningInput(const Event& event) {
        if (event.type == Event::TextEntered) {
            if (event.text.unicode == 8) { // Backspace
                if (!inputBuffer.empty())
                    inputBuffer.pop_back();
            }
            else if (event.text.unicode == 13) { // Enter
                processLearningEnter();
            }
            else if (event.text.unicode >= 32 && event.text.unicode < 127) {
                if (inputBuffer.size() < 100)
                    inputBuffer += static_cast<char>(event.text.unicode);
            }
        }
    }

    void processLearningEnter() {
        if (inputBuffer.empty()) return;

        if (inputQuestionPhase) {
            newAnswer = inputBuffer;
            inputBuffer.clear();
            inputQuestionPhase = false;
            inputAnswerPhase = true;
            inputPromptText.setString("Enter a yes/no question to distinguish\n\"" + newAnswer + "\" from \"" + currentNode->text + "\":");
        }
        else if (inputAnswerPhase) {
            newQuestion = inputBuffer;
            inputBuffer.clear();
            inputAnswerPhase = false;
            inputPromptText.setString("For \"" + newAnswer + "\", is the answer to\n\"" + newQuestion + "\" 'Yes' or 'No'? (y/n):");
        }
        else {
            if (inputBuffer.size() == 1) {
                char c = tolower(inputBuffer[0]);
                if (c == 'y' || c == 'n') {
                    yesForAnswer = c;
                    finalizeLearning();
                    learningMode = false;
                }
            }
            inputBuffer.clear();
        }
    }

    void finalizeLearning() {
        string oldAnswer = currentNode->text;
        currentNode->text = newQuestion;

        if (yesForAnswer == 'y') {
            currentNode->yes = make_unique<Node>(Node{ newAnswer });
            currentNode->no = make_unique<Node>(Node{ oldAnswer });
        }
        else {
            currentNode->no = make_unique<Node>(Node{ newAnswer });
            currentNode->yes = make_unique<Node>(Node{ oldAnswer });
        }

        saveTree();
        showPopup("Thanks! I've learned something new.");
        currentNode = root.get();
    }

    void loadTree() {
        ifstream fin(filename);
        if (fin.good()) {
            root = loadNode(fin);
            if (!root) {
                createDefaultTree();
            }
        }
        else {
            createDefaultTree();
        }
    }

    void createDefaultTree() {
        root = make_unique<Node>(Node{ "Is it an animal?" });
        root->yes = make_unique<Node>(Node{ "Is it commonly kept as a pet?" });
        root->yes->yes = make_unique<Node>(Node{ "Is it a dog?" });
        root->yes->yes->yes = make_unique<Node>(Node{ "Dog" });
        root->yes->yes->no = make_unique<Node>(Node{ "Cat" });
        root->yes->no = make_unique<Node>(Node{ "Is it a bird?" });
        root->yes->no->yes = make_unique<Node>(Node{ "Bird" });
        root->yes->no->no = make_unique<Node>(Node{ "Elephant" });
        root->no = make_unique<Node>(Node{ "Is it man-made?" });
        root->no->yes = make_unique<Node>(Node{ "Is it electronic?" });
        root->no->yes->yes = make_unique<Node>(Node{ "Computer" });
        root->no->yes->no = make_unique<Node>(Node{ "Car" });
        root->no->no = make_unique<Node>(Node{ "Is it a plant?" });
        root->no->no->yes = make_unique<Node>(Node{ "Tree" });
        root->no->no->no = make_unique<Node>(Node{ "Rock" });
        saveTree();
    }

    unique_ptr<Node> loadNode(ifstream& fin) {
        string line;
        if (!getline(fin, line)) return nullptr;
        if (line == "#") return nullptr;

        auto node = make_unique<Node>();
        node->text = line;
        node->yes = loadNode(fin);
        node->no = loadNode(fin);

        return node;
    }

    void saveTree() {
        ofstream fout(filename);
        saveNode(fout, root.get());
    }

    void saveNode(ofstream& fout, Node* node) {
        if (!node) {
            fout << "#\n";
            return;
        }
        fout << node->text << "\n";
        saveNode(fout, node->yes.get());
        saveNode(fout, node->no.get());
    }
};

