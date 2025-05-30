#include<SFML\Graphics.hpp>
#include <windows.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>
#include <iostream>
#include <string>
#include <vector>


using namespace std;
using namespace sf;

const int boardSize = 6;
const int cellSize = 70;
const int windowPadding = 50;
const int windowWidth = boardSize * cellSize + windowPadding * 2 + 300;
const int windowHeight = boardSize * cellSize + windowPadding * 2 + 100;
const Color backgroundColor(240, 240, 255);
const Color inactiveCellColor(210, 210, 210);
const Color activeCellColor(255, 255, 255);
const Color selectedCellColor(200, 230, 255);
const Color correctAnswerColor(200, 255, 200);


struct Clue {
    std::string answer;
    std::string clue;
    int row;
    int col;
    bool across;
    int number;
};

//vector<Clue> clues = {
//{"CAT", " A small domesticated feline", 0, 0, true, 1},
//{"DOG", " A loyal animal", 1, 0, true, 2},
//{"BEE", " Insect that makes honey", 2, 1, true, 3},
//{"ANT", " Tiny hardworking insect", 3, 2, false, 4},
//{"SUN", " Shines during the day", 4, 3, true, 5}
//};



vector<Clue> clues;
bool loadCluesFromSQLServer() {
    SQLHENV hEnv = NULL;
    SQLHDBC hDbc = NULL;
    SQLHSTMT hStmt = NULL;
    SQLRETURN ret;

    // Allocate environment handle
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    if (!SQL_SUCCEEDED(ret)) {
        cerr << "Failed to allocate environment handle." << endl;
        return false;
    }

    // Set ODBC version
    ret = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (!SQL_SUCCEEDED(ret)) {
        cerr << "Failed to set ODBC version." << endl;
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return false;
    }

    // Allocate connection handle
    ret = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
    if (!SQL_SUCCEEDED(ret)) {
        cerr << "Failed to allocate connection handle." << endl;
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return false;
    }

    // Connection string (wide string)
    SQLWCHAR connStr[] = L"Driver={ODBC Driver 17 for SQL Server};Server=DESKTOP-E1HP2Q6\\SQLEXPRESS01;Database=timetable;Trusted_Connection=Yes;";

    // Output buffer for connection diagnostic message
    SQLWCHAR outStr[1024];
    SQLSMALLINT outStrLen;

    // Connect to DB using wide function
    ret = SQLDriverConnectW(hDbc, NULL, connStr, SQL_NTS, outStr, sizeof(outStr) / sizeof(SQLWCHAR), &outStrLen, SQL_DRIVER_COMPLETE);
    if (SQL_SUCCEEDED(ret)) {
        cout << "Connected to SQL Server successfully." << endl;
    }
    else {
        cerr << "Connection failed." << endl;
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return false;
    }

    // Allocate statement handle
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    if (!SQL_SUCCEEDED(ret)) {
        cerr << "Failed to allocate statement handle." << endl;
        SQLDisconnect(hDbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return false;
    }

    // SQL query - narrow string (ANSI)
    // Change this line:
   // string query = "SELECT ClueID, ClueText FROM Clues";

    // To this (assuming your table has these columns):
    string query = "select Answer, ClueText, RowPos, ColPos, IsAcross from Clues";

    // Execute query (ANSI version)
    try {
        ret = SQLExecDirectA(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
        if (!SQL_SUCCEEDED(ret)) {
            cerr << "Query execution failed." << endl;
            // handle cleanup
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
            SQLDisconnect(hDbc);
            SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
            SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
            return false;
        }
    }
    catch (const std::exception& ex) {
        cerr << "Exception caught during query execution: " << ex.what() << endl;
        // handle cleanup similarly here
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        SQLDisconnect(hDbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return false;
    }
    catch (...) {
        cerr << "Unknown exception caught during query execution." << endl;
        // handle cleanup similarly here
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        SQLDisconnect(hDbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return false;
    }


    // Buffers for fetching data
    char answer[100] = { 0 };
    char clueText[300] = { 0 };
    int row = 0, col = 0, acrossInt = 0, number = 0;

    clues.clear();

    // Fetch rows
    while (SQLFetch(hStmt) == SQL_SUCCESS) {
        SQLGetData(hStmt, 1, SQL_C_CHAR, answer, sizeof(answer), NULL);       // ClueID
        SQLGetData(hStmt, 2, SQL_C_CHAR, clueText, sizeof(clueText), NULL);   // ClueText
        SQLGetData(hStmt, 3, SQL_C_SLONG, &row, 0, NULL);                     // RowPos
        SQLGetData(hStmt, 4, SQL_C_SLONG, &col, 0, NULL);                     // ColPos
        SQLGetData(hStmt, 5, SQL_C_SLONG, &acrossInt, 0, NULL);              // IsAcross

        clues.push_back({ string(answer), string(clueText), row, col, (acrossInt == 1) });

        // Clear buffers for next fetch
        memset(answer, 0, sizeof(answer));
        memset(clueText, 0, sizeof(clueText));
    }

    // Cleanup
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    SQLDisconnect(hDbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

    return true;
}




class CrosswordGame {
private:
    RenderWindow window;
    Font font;
    vector<vector<string>> board;
    vector<vector<bool>> activeCells;
    vector<vector<int>> cellNumbers;
    vector<vector<bool>> correctCells;
    Clock gameClock;
    Time elapsedTime;
    bool gameWon;
    bool showReplay;
    Vector2i selectedCell;
    bool cellSelected;
    int currentClueHighlight;
    Texture backgroundTexture;
    Sprite backgroundSprite;


public:
    CrosswordGame() : window(VideoMode(900,600 ), "E6nhanced Crossword Puzzle"),
        gameWon(false), showReplay(false), selectedCell(-1, -1), cellSelected(false), currentClueHighlight(-1) {

        if (!font.loadFromFile("C:/Users/T L S/Downloads/LunaLogic/DSA PROJECT/assets/ARIAL.TTF")) {
            cerr << "Error loading font!" << endl;
        }

        initializeGame();
    }

    void initializeGame() {
        if (!loadCluesFromSQLServer()) {
            cerr << "Failed to load clues from database!" << endl;
            return;
        }

        // The rest of your existing code
        //board = vector<vector<string>>(boardSize, vector<string>(boardSize, ""));
        board = vector<vector<string>>(boardSize, vector<string>(boardSize, ""));
        activeCells = vector<vector<bool>>(boardSize, vector<bool>(boardSize, false));
        cellNumbers = vector<vector<int>>(boardSize, vector<int>(boardSize, 0));
        correctCells = vector<vector<bool>>(boardSize, vector<bool>(boardSize, false));

        if (!backgroundTexture.loadFromFile("C:/Users/T L S/Downloads/LunaLogic/DSA PROJECT/assets/pic2.jpg")) {
            cerr << "Error loading background image!" << endl;
        }
        backgroundSprite.setTexture(backgroundTexture);
        backgroundSprite.setScale(
            float(windowWidth) / backgroundTexture.getSize().x,
            float(windowHeight) / backgroundTexture.getSize().y
        );


        for (const auto& clue : clues) {
            for (int i = 0; i < clue.answer.size(); ++i) {
                int r = clue.row + (clue.across ? 0 : i);
                int c = clue.col + (clue.across ? i : 0);

                // Ensure we don't go out of bounds
                if (r < boardSize && c < boardSize) {
                    activeCells[r][c] = true;
                    if (i == 0) cellNumbers[r][c] = clue.number;
                }
            }
        }

        gameClock.restart();
        gameWon = false;
        showReplay = false;
        selectedCell = Vector2i(-1, -1);
        cellSelected = false;
        currentClueHighlight = -1;
    }

    void run() {
        while (window.isOpen()) {
            handleEvents();
            update();
            render();
        }
    }

private:
    void handleEvents() {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();
            else if (event.type == Event::KeyPressed)
                handleKeyInput(event.key.code);
            else if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left)
                handleMouseClick(Mouse::getPosition(window));
            else if (event.type == Event::TextEntered && !gameWon && cellSelected)
                handleTextInput(event.text.unicode);
        }
    }

    void handleKeyInput(Keyboard::Key key) {
        if (key == Keyboard::Escape)
            window.close();
        else if (key == Keyboard::R && gameWon)
            initializeGame();
        else if (key == Keyboard::Space)
            initializeGame();
        else if (key == Keyboard::H && !gameWon)
            giveHint();
        else if (cellSelected && !gameWon) {
            if (key == Keyboard::Up && selectedCell.y > 0)
                selectCell(selectedCell.x, selectedCell.y - 1);
            else if (key == Keyboard::Down && selectedCell.y < boardSize - 1)
                selectCell(selectedCell.x, selectedCell.y + 1);
            else if (key == Keyboard::Left && selectedCell.x > 0)
                selectCell(selectedCell.x - 1, selectedCell.y);
            else if (key == Keyboard::Right && selectedCell.x < boardSize - 1)
                selectCell(selectedCell.x + 1, selectedCell.y);
            else if (key == Keyboard::BackSpace || key == Keyboard::Delete)
                clearSelectedCell();
        }
    }

    void handleMouseClick(Vector2i pos) {
        int col = (pos.x - windowPadding) / cellSize;
        int row = (pos.y - windowPadding) / cellSize;
        if (row >= 0 && row < boardSize && col >= 0 && col < boardSize && activeCells[row][col])
            selectCell(col, row);
        else {
            selectedCell = { -1, -1 };
            cellSelected = false;
            currentClueHighlight = -1;
        }
    }

    void handleTextInput(Uint32 unicode) {
        if (isalpha(unicode)) {
            board[selectedCell.y][selectedCell.x] = toupper(static_cast<char>(unicode));
            checkForWin();
            moveToNextCell();
        }
    }

    void selectCell(int col, int row) {
        if (!activeCells[row][col]) return;
        selectedCell = { col, row };
        cellSelected = true;
        for (size_t i = 0; i < clues.size(); ++i) {
            const auto& clue = clues[i];
            if ((clue.across && row == clue.row && col >= clue.col && col < clue.col + clue.answer.size()) ||
                (!clue.across && col == clue.col && row >= clue.row && row < clue.row + clue.answer.size())) {
                currentClueHighlight = i;
                break;
            }
        }
    }

    void moveToNextCell() {
        if (selectedCell.x < boardSize - 1 && activeCells[selectedCell.y][selectedCell.x + 1])
            selectCell(selectedCell.x + 1, selectedCell.y);
        else if (selectedCell.y < boardSize - 1 && activeCells[selectedCell.y + 1][selectedCell.x])
            selectCell(selectedCell.x, selectedCell.y + 1);
    }

    void clearSelectedCell() {
        board[selectedCell.y][selectedCell.x] = "";
    }

    void giveHint() {
        for (const auto& clue : clues) {
            if ((clue.across && selectedCell.y == clue.row && selectedCell.x >= clue.col && selectedCell.x < clue.col + clue.answer.size()) ||
                (!clue.across && selectedCell.x == clue.col && selectedCell.y >= clue.row && selectedCell.y < clue.row + clue.answer.size())) {
                int i = clue.across ? selectedCell.x - clue.col : selectedCell.y - clue.row;
                board[selectedCell.y][selectedCell.x] = clue.answer[i];
                correctCells[selectedCell.y][selectedCell.x] = true;
                checkForWin();
                moveToNextCell();
                break;
            }
        }
    }

    void checkForWin() {
        gameWon = true;
        for (const auto& clue : clues) {
            for (int i = 0; i < clue.answer.size(); ++i) {
                int r = clue.row + (clue.across ? 0 : i);
                int c = clue.col + (clue.across ? i : 0);
                if (board[r][c] != string(1, clue.answer[i])) {
                    gameWon = false;
                    return;
                }
            }
        }
        showReplay = true;
        elapsedTime = gameClock.getElapsedTime();
    }

    void update() {
        if (!gameWon)
            elapsedTime = gameClock.getElapsedTime();
    }

    void render() {
        window.clear();
        window.draw(backgroundSprite); // Draw background first
        // window.clear(backgroundColor);
        drawBoard();
        drawClues();
        drawTimer();
        drawInstructions();
        if (gameWon)
            drawWinScreen();
        window.display();
    }

    void drawBoard() {
        for (int row = 0; row < boardSize; ++row) {
            for (int col = 0; col < boardSize; ++col) {
                RectangleShape cell(Vector2f(cellSize - 2, cellSize - 2));
                cell.setPosition(windowPadding + col * cellSize, windowPadding + row * cellSize);
                if (!activeCells[row][col])
                    cell.setFillColor(inactiveCellColor);
                else if (cellSelected && selectedCell == Vector2i(col, row))
                    cell.setFillColor(selectedCellColor);
                else if (correctCells[row][col])
                    cell.setFillColor(correctAnswerColor);
                else
                    cell.setFillColor(activeCellColor);
                cell.setOutlineThickness(1);
                cell.setOutlineColor(Color::Black);
                window.draw(cell);

                if (cellNumbers[row][col] > 0) {
                    Text number(to_string(cellNumbers[row][col]), font, 14);
                    number.setPosition(cell.getPosition().x + 2, cell.getPosition().y + 2);
                    number.setFillColor(Color::Black);
                    window.draw(number);
                }

                if (!board[row][col].empty()) {
                    Text letter(board[row][col], font, 36);
                    FloatRect bounds = letter.getLocalBounds();
                    letter.setPosition(cell.getPosition().x + (cellSize - bounds.width) / 2 - 8,
                        cell.getPosition().y + (cellSize - bounds.height) / 2 - 8);
                    letter.setFillColor(Color::Black);
                    window.draw(letter);
                }
            }
        }
    }

    void drawClues() {
        int x = windowPadding + boardSize * cellSize + 20;
        int y = windowPadding;

        //RectangleShape cluePanel(Vector2f(280, windowHeight - 2 * windowPadding));
        //cluePanel.setPosition(x - 10, y - 10);
        //cluePanel.setFillColor(Color(20, 20, 20, 180)); // Semi-transparent dark panel
        //window.draw(cluePanel);

        // Title "Clues"
        Text title("Clues", font, 26);
        title.setStyle(Text::Bold);
        title.setFillColor(Color::White);
        title.setPosition(x, y);
        window.draw(title);
        y += 40;

        // "ACROSS" Header
        Text across("ACROSS", font, 20);
        across.setStyle(Text::Bold);
        across.setFillColor(Color(200, 200, 255));
        across.setPosition(x, y);
        window.draw(across);
        y += 30;

        for (const auto& clue : clues)
            if (clue.across) drawClueText(clue, x, y);

        y += 20;

        // "DOWN" Header
        Text down("DOWN", font, 20);
        down.setStyle(Text::Bold);
        down.setFillColor(Color(200, 200, 255));
        down.setPosition(x, y);
        window.draw(down);
        y += 30;

        for (const auto& clue : clues)
            if (!clue.across) drawClueText(clue, x, y);
    }

    void drawClueText(const Clue& clue, int& x, int& y) {
        Text clueLine(to_string(clue.number) + ". " + clue.clue, font, 22);
        clueLine.setFillColor(Color(230, 230, 230));
        clueLine.setPosition(x, y);
        window.draw(clueLine);
        y += 24;
    }




    void drawTimer() {
        Text timer("Time: " + to_string((int)elapsedTime.asSeconds()) + "s", font, 22);
        timer.setPosition(windowPadding, 10);
        timer.setStyle(Text::Bold);
        timer.setFillColor(Color(1000, 1000, 1000));
        window.draw(timer);
    }


    void drawInstructions() {
        /*RectangleShape controlsBg(Vector2f(420, 50));
        controlsBg.setFillColor(Color(20, 20, 20, 180));
        controlsBg.setPosition(windowPadding, windowHeight - windowPadding - 50);
        window.draw(controlsBg);*/

        Text controls("Click or use arrow keys to navigate\nType to fill |  Space - New | R - Restart | ESC - Quit", font, 22);
        controls.setFillColor(Color::White);
        controls.setPosition(windowPadding + 10, windowHeight - windowPadding - 45);
        window.draw(controls);

    }


    void drawWinScreen() {
        RectangleShape overlay(Vector2f(windowWidth, windowHeight));
        overlay.setFillColor(Color(0, 0, 0, 160));
        window.draw(overlay);

        RectangleShape box(Vector2f(400, 200));
        box.setPosition(windowWidth / 2 - 200, windowHeight / 2 - 100);
        box.setFillColor(Color(60, 180, 75));
        box.setOutlineColor(Color::White);
        box.setOutlineThickness(4);
        window.draw(box);

        Text message("You Won!", font, 40);
        message.setStyle(Text::Bold);
        message.setFillColor(Color::White);
        FloatRect bounds = message.getLocalBounds();
        message.setPosition(windowWidth / 2 - bounds.width / 2, windowHeight / 2 - 80);
        window.draw(message);

        Text timeMsg("Time: " + to_string((int)elapsedTime.asSeconds()) + " seconds", font, 26);
        timeMsg.setFillColor(Color::White);
        bounds = timeMsg.getLocalBounds();
        timeMsg.setPosition(windowWidth / 2 - bounds.width / 2, windowHeight / 2 - 20);
        window.draw(timeMsg);

        Text replayMsg("Press 'R' to Replay or 'ESC' to Quit", font, 22);
        replayMsg.setFillColor(Color::White);
        bounds = replayMsg.getLocalBounds();
        replayMsg.setPosition(windowWidth / 2 - bounds.width / 2, windowHeight / 2 + 40);
        window.draw(replayMsg);
    }

};


