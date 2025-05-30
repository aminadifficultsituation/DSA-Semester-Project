//#include "crossword.h"
//
//vector<Clue> clues = {
//    {"CAT", " A small domesticated feline", 0, 0, true, 1},
//    {"DOG", " A loyal animal", 1, 0, true, 2},
//    {"BEE", " Insect that makes honey", 2, 1, true, 3},
//    {"ATT", " Tiny hardworking insect", 3, 2, false, 4},
//    {"SUU", " Shines during the day", 4, 3, true, 5}
//};
//
//CrosswordGame::CrosswordGame() : window(VideoMode(windowWidth, windowHeight), "Enhanced Crossword Puzzle"),
//gameWon(false), showReplay(false), selectedCell(-1, -1), cellSelected(false), currentClueHighlight(-1) {
//
//    if (!font.loadFromFile("D:/FINE laptop/DSA PROJECT/assets/ARIAL.TTF")) {
//        cerr << "Error loading font!" << endl;
//    }
//
//    initializeGame();
//}
//
//void CrosswordGame::initializeGame() {
//    board = vector<vector<string>>(boardSize, vector<string>(boardSize, ""));
//    activeCells = vector<vector<bool>>(boardSize, vector<bool>(boardSize, false));
//    cellNumbers = vector<vector<int>>(boardSize, vector<int>(boardSize, 0));
//    correctCells = vector<vector<bool>>(boardSize, vector<bool>(boardSize, false));
//
//    if (!backgroundTexture.loadFromFile("D:/FINE laptop/DSA PROJECT/assets/pic2.jpg")) {
//        cerr << "Error loading background image!" << endl;
//    }
//    backgroundSprite.setTexture(backgroundTexture);
//    backgroundSprite.setScale(
//        float(windowWidth) / backgroundTexture.getSize().x,
//        float(windowHeight) / backgroundTexture.getSize().y
//    );
//
//    for (const auto& clue : clues) {
//        for (int i = 0; i < clue.answer.size(); ++i) {
//            int r = clue.row + (clue.across ? 0 : i);
//            int c = clue.col + (clue.across ? i : 0);
//            activeCells[r][c] = true;
//            if (i == 0) cellNumbers[r][c] = clue.number;
//        }
//    }
//
//    gameClock.restart();
//    gameWon = false;
//    showReplay = false;
//    selectedCell = Vector2i(-1, -1);
//    cellSelected = false;
//    currentClueHighlight = -1;
//}
//
//void CrosswordGame::run() {
//    while (window.isOpen()) {
//        handleEvents();
//        update();
//        render();
//    }
//}
//
//void CrosswordGame::handleEvents() {
//    Event event;
//    while (window.pollEvent(event)) {
//        if (event.type == Event::Closed)
//            window.close();
//        else if (event.type == Event::KeyPressed)
//            handleKeyInput(event.key.code);
//        else if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left)
//            handleMouseClick(Mouse::getPosition(window));
//        else if (event.type == Event::TextEntered && !gameWon && cellSelected)
//            handleTextInput(event.text.unicode);
//    }
//}
//
//void CrosswordGame::handleKeyInput(Keyboard::Key key) {
//    if (key == Keyboard::Escape)
//        window.close();
//    else if (key == Keyboard::R && gameWon)
//        initializeGame();
//    else if (key == Keyboard::N)
//        initializeGame();
//    else if (key == Keyboard::H && !gameWon)
//        giveHint();
//    else if (cellSelected && !gameWon) {
//        if (key == Keyboard::Up && selectedCell.y > 0)
//            selectCell(selectedCell.x, selectedCell.y - 1);
//        else if (key == Keyboard::Down && selectedCell.y < boardSize - 1)
//            selectCell(selectedCell.x, selectedCell.y + 1);
//        else if (key == Keyboard::Left && selectedCell.x > 0)
//            selectCell(selectedCell.x - 1, selectedCell.y);
//        else if (key == Keyboard::Right && selectedCell.x < boardSize - 1)
//            selectCell(selectedCell.x + 1, selectedCell.y);
//        else if (key == Keyboard::BackSpace || key == Keyboard::Delete)
//            clearSelectedCell();
//    }
//}
//
//void CrosswordGame::handleMouseClick(Vector2i pos) {
//    int col = (pos.x - windowPadding) / cellSize;
//    int row = (pos.y - windowPadding) / cellSize;
//    if (row >= 0 && row < boardSize && col >= 0 && col < boardSize && activeCells[row][col])
//        selectCell(col, row);
//    else {
//        selectedCell = { -1, -1 };
//        cellSelected = false;
//        currentClueHighlight = -1;
//    }
//}
//
//void CrosswordGame::handleTextInput(Uint32 unicode) {
//    if (isalpha(unicode)) {
//        board[selectedCell.y][selectedCell.x] = toupper(static_cast<char>(unicode));
//        checkForWin();
//        moveToNextCell();
//    }
//}
//
//void CrosswordGame::selectCell(int col, int row) {
//    if (!activeCells[row][col]) return;
//    selectedCell = { col, row };
//    cellSelected = true;
//    for (size_t i = 0; i < clues.size(); ++i) {
//        const auto& clue = clues[i];
//        if ((clue.across && row == clue.row && col >= clue.col && col < clue.col + clue.answer.size()) ||
//            (!clue.across && col == clue.col && row >= clue.row && row < clue.row + clue.answer.size())) {
//            currentClueHighlight = i;
//            break;
//        }
//    }
//}
//
//void CrosswordGame::moveToNextCell() {
//    if (selectedCell.x < boardSize - 1 && activeCells[selectedCell.y][selectedCell.x + 1])
//        selectCell(selectedCell.x + 1, selectedCell.y);
//    else if (selectedCell.y < boardSize - 1 && activeCells[selectedCell.y + 1][selectedCell.x])
//        selectCell(selectedCell.x, selectedCell.y + 1);
//}
//
//void CrosswordGame::clearSelectedCell() {
//    board[selectedCell.y][selectedCell.x] = "";
//}
//
//void CrosswordGame::giveHint() {
//    for (const auto& clue : clues) {
//        if ((clue.across && selectedCell.y == clue.row && selectedCell.x >= clue.col && selectedCell.x < clue.col + clue.answer.size()) ||
//            (!clue.across && selectedCell.x == clue.col && selectedCell.y >= clue.row && selectedCell.y < clue.row + clue.answer.size())) {
//            int i = clue.across ? selectedCell.x - clue.col : selectedCell.y - clue.row;
//            board[selectedCell.y][selectedCell.x] = clue.answer[i];
//            correctCells[selectedCell.y][selectedCell.x] = true;
//            checkForWin();
//            moveToNextCell();
//            break;
//        }
//    }
//}
//
//void CrosswordGame::checkForWin() {
//    gameWon = true;
//    for (const auto& clue : clues) {
//        for (int i = 0; i < clue.answer.size(); ++i) {
//            int r = clue.row + (clue.across ? 0 : i);
//            int c = clue.col + (clue.across ? i : 0);
//            if (board[r][c] != string(1, clue.answer[i])) {
//                gameWon = false;
//                return;
//            }
//        }
//    }
//    showReplay = true;
//    elapsedTime = gameClock.getElapsedTime();
//}
//
//void CrosswordGame::update() {
//    if (!gameWon)
//        elapsedTime = gameClock.getElapsedTime();
//}
//
//void CrosswordGame::render() {
//    window.clear(backgroundColor);
//    window.draw(backgroundSprite);
//    drawBoard();
//    drawClues();
//    drawTimer();
//    drawInstructions();
//    if (gameWon)
//        drawWinScreen();
//    window.display();
//}
//
//void CrosswordGame::drawBoard() {
//    RectangleShape cellShape(Vector2f(cellSize - 2, cellSize - 2));
//    for (int r = 0; r < boardSize; ++r) {
//        for (int c = 0; c < boardSize; ++c) {
//            if (!activeCells[r][c]) continue;
//            cellShape.setPosition(windowPadding + c * cellSize + 1, windowPadding + r * cellSize + 1);
//            if (selectedCell.x == c && selectedCell.y == r)
//                cellShape.setFillColor(selectedCellColor);
//            else if (correctCells[r][c])
//                cellShape.setFillColor(correctAnswerColor);
//            else
//                cellShape.setFillColor(activeCellColor);
//            window.draw(cellShape);
//
//            if (cellNumbers[r][c] != 0) {
//                Text numberText(to_string(cellNumbers[r][c]), font, 14);
//                numberText.setFillColor(Color::Black);
//                numberText.setPosition(cellShape.getPosition().x + 3, cellShape.getPosition().y + 1);
//                window.draw(numberText);
//            }
//
//            if (!board[r][c].empty()) {
//                Text letterText(board[r][c], font, 32);
//                letterText.setFillColor(Color::Black);
//                FloatRect bounds = letterText.getLocalBounds();
//                letterText.setOrigin(bounds.left + bounds.width / 2, bounds.top + bounds.height / 2);
//                letterText.setPosition(cellShape.getPosition().x + cellSize / 2, cellShape.getPosition().y + cellSize / 2);
//                window.draw(letterText);
//            }
//        }
//    }
//}
//
//void CrosswordGame::drawClues() {
//    int x = windowPadding + boardSize * cellSize + 20;
//    int y = windowPadding;
//
//    for (size_t i = 0; i < clues.size(); ++i) {
//        if ((int)i == currentClueHighlight) {
//            Text clueText(">" + to_string(clues[i].number) + ". " + clues[i].clue, font, 20);
//            clueText.setFillColor(Color::White);
//            clueText.setPosition(x, y);
//            window.draw(clueText);
//        }
//        else {
//            Text clueText(to_string(clues[i].number) + ". " + clues[i].clue, font, 20);
//            clueText.setFillColor(Color::White);
//            clueText.setPosition(x, y);
//            window.draw(clueText);
//        }
//        y += 30;
//    }
//}
//
//void CrosswordGame::drawTimer() {
//    int seconds = static_cast<int>(elapsedTime.asSeconds());
//    int minutes = seconds / 60;
//    seconds %= 60;
//
//    Text timerText("Time: " + to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + to_string(seconds), font, 20);
//    timerText.setFillColor(Color::Black);
//    timerText.setPosition(windowPadding, windowHeight - 80);
//    window.draw(timerText);
//}
//
//void CrosswordGame::drawInstructions() {
//    Text instructions("Arrows: Move | H: Hint | R: Replay | N: New | Esc: Quit", font, 22);
//    instructions.setFillColor(Color::White);
//    instructions.setPosition(windowPadding, windowHeight - 50);
//    window.draw(instructions);
//}
//
//void CrosswordGame::drawWinScreen() {
//    RectangleShape overlay(Vector2f(windowWidth, windowHeight));
//    overlay.setFillColor(Color(0, 0, 0, 180));
//    window.draw(overlay);
//
//    Text winText("You won!", font, 50);
//    winText.setFillColor(Color::White);
//    FloatRect bounds = winText.getLocalBounds();
//    winText.setOrigin(bounds.left + bounds.width / 2, bounds.top + bounds.height / 2);
//    winText.setPosition(windowWidth / 2, windowHeight / 2 - 50);
//    window.draw(winText);
//
//    Text timeText("Time: " + to_string(static_cast<int>(elapsedTime.asSeconds())) + " seconds", font, 30);
//    timeText.setFillColor(Color::White);
//    bounds = timeText.getLocalBounds();
//    timeText.setOrigin(bounds.left + bounds.width / 2, bounds.top + bounds.height / 2);
//    timeText.setPosition(windowWidth / 2, windowHeight / 2);
//    window.draw(timeText);
//
//    Text replayText("Press R to replay", font, 25);
//    replayText.setFillColor(Color::White);
//    bounds = replayText.getLocalBounds();
//    replayText.setOrigin(bounds.left + bounds.width / 2, bounds.top + bounds.height / 2);
//    replayText.setPosition(windowWidth / 2, windowHeight / 2 + 50);
//    window.draw(replayText);
//}
