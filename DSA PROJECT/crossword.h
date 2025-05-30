#include<SFML\Graphics.hpp>
#include <windows.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

using namespace std;
using namespace sf;

// Constants for game configuration
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

// Database configuration
const wstring DB_CONNECTION_STRING = L"Driver={ODBC Driver 17 for SQL Server};Server=DESKTOP-E1HP2Q6\\SQLEXPRESS01;Database=timetable;Trusted_Connection=Yes;";

struct Clue {
    string answer;
    string clue;
    int row;
    int col;
    bool across;
    int number;
};

// Database connection wrapper class
class DatabaseConnection {
private:
    SQLHENV hEnv;
    SQLHDBC hDbc;
    bool connected;

public:
    DatabaseConnection() : hEnv(NULL), hDbc(NULL), connected(false) {
        initialize();
    }

    ~DatabaseConnection() {
        cleanup();
    }

    bool initialize() {
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
            cleanup();
            return false;
        }

        // Allocate connection handle
        ret = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
        if (!SQL_SUCCEEDED(ret)) {
            cerr << "Failed to allocate connection handle." << endl;
            cleanup();
            return false;
        }

        // Connect to database
        SQLWCHAR outStr[1024];
        SQLSMALLINT outStrLen;
        ret = SQLDriverConnectW(hDbc, NULL, (SQLWCHAR*)DB_CONNECTION_STRING.c_str(), SQL_NTS, 
                              outStr, sizeof(outStr) / sizeof(SQLWCHAR), &outStrLen, SQL_DRIVER_COMPLETE);
        
        if (SQL_SUCCEEDED(ret)) {
            connected = true;
            cout << "Connected to SQL Server successfully." << endl;
            return true;
        } else {
            cerr << "Connection failed." << endl;
            cleanup();
            return false;
        }
    }

    void cleanup() {
        if (hDbc) {
            if (connected) {
                SQLDisconnect(hDbc);
            }
            SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
            hDbc = NULL;
        }
        if (hEnv) {
            SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
            hEnv = NULL;
        }
        connected = false;
    }

    bool isConnected() const { return connected; }
    SQLHDBC getConnection() const { return hDbc; }
};

vector<Clue> loadCluesFromDatabase() {
    vector<Clue> loadedClues;
    DatabaseConnection db;

    if (!db.isConnected()) {
        cerr << "Failed to connect to database. Using default clues." << endl;
        // Fallback to default clues if database connection fails
        loadedClues = {
            {"CAT", "A small domesticated feline", 0, 0, true, 1},
            {"DOG", "A loyal animal", 1, 0, true, 2},
            {"BEE", "Insect that makes honey", 2, 1, true, 3},
            {"ANT", "Tiny hardworking insect", 3, 2, false, 4},
            {"SUN", "Shines during the day", 4, 3, true, 5}
        };
        cout << "Using " << loadedClues.size() << " default clues." << endl;
        return loadedClues;
    }

    SQLHSTMT hStmt = NULL;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, db.getConnection(), &hStmt);
    
    if (!SQL_SUCCEEDED(ret)) {
        cerr << "Failed to allocate statement handle." << endl;
        return loadedClues;
    }

    try {
        string query = "SELECT Answer, ClueText, RowPos, ColPos, IsAcross FROM Clues";
        ret = SQLExecDirectA(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
        
        if (!SQL_SUCCEEDED(ret)) {
            cerr << "Query execution failed." << endl;
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
            return loadedClues;
        }

        // Buffers for fetching data
        char answer[100] = { 0 };
        char clueText[300] = { 0 };
        int row = 0, col = 0, acrossInt = 0, number = 1;

        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_CHAR, answer, sizeof(answer), NULL);
            SQLGetData(hStmt, 2, SQL_C_CHAR, clueText, sizeof(clueText), NULL);
            SQLGetData(hStmt, 3, SQL_C_SLONG, &row, 0, NULL);
            SQLGetData(hStmt, 4, SQL_C_SLONG, &col, 0, NULL);
            SQLGetData(hStmt, 5, SQL_C_SLONG, &acrossInt, 0, NULL);

            loadedClues.push_back({string(answer), string(clueText), row, col, (acrossInt == 1), number++});

            memset(answer, 0, sizeof(answer));
            memset(clueText, 0, sizeof(clueText));
        }
    }
    catch (const exception& ex) {
        cerr << "Exception during database operation: " << ex.what() << endl;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return loadedClues;
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
    vector<Clue> clues;

public:
    CrosswordGame() : window(VideoMode(windowWidth, windowHeight), "Enhanced Crossword Puzzle"),
        gameWon(false), showReplay(false), selectedCell(-1, -1), cellSelected(false), currentClueHighlight(-1) {
        
        try {
            // Load assets with debug output
            cout << "Attempting to load font from: assets/ARIAL.TTF" << endl;
            if (!font.loadFromFile("assets/ARIAL.TTF")) {
                throw runtime_error("Failed to load font file");
            }
            cout << "Font loaded successfully" << endl;

            cout << "Attempting to load background from: assets/pic2.jpg" << endl;
            if (!backgroundTexture.loadFromFile("assets/pic2.jpg")) {
                throw runtime_error("Failed to load background image");
            }
            cout << "Background loaded successfully" << endl;

            backgroundSprite.setTexture(backgroundTexture);
            backgroundSprite.setScale(
                float(windowWidth) / backgroundTexture.getSize().x,
                float(windowHeight) / backgroundTexture.getSize().y
            );

            initializeGame();
            cout << "Game initialized with " << clues.size() << " clues" << endl;
        }
        catch (const exception& e) {
            cerr << "Error initializing game: " << e.what() << endl;
            window.close();
        }
    }

    void initializeGame() {
        cout << "Initializing game..." << endl;
        clues = loadCluesFromDatabase();
        cout << "Loaded " << clues.size() << " clues from database" << endl;
        
        // Initialize game board
        board = vector<vector<string>>(boardSize, vector<string>(boardSize, ""));
        activeCells = vector<vector<bool>>(boardSize, vector<bool>(boardSize, false));
        cellNumbers = vector<vector<int>>(boardSize, vector<int>(boardSize, 0));
        correctCells = vector<vector<bool>>(boardSize, vector<bool>(boardSize, false));

        // Set up active cells and numbers
        for (const auto& clue : clues) {
            cout << "Setting up clue: " << clue.answer << " at position (" << clue.row << "," << clue.col << ")" << endl;
            for (int i = 0; i < clue.answer.size(); ++i) {
                int r = clue.row + (clue.across ? 0 : i);
                int c = clue.col + (clue.across ? i : 0);

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
        cout << "Game initialization complete" << endl;
    }

    void drawTimer() {
        Text timer("Time: " + to_string((int)elapsedTime.asSeconds()) + "s", font, 22);
        timer.setPosition(windowPadding, 10);
        timer.setStyle(Text::Bold);
        timer.setFillColor(Color(255, 255, 255));  // Fixed color values to be within 0-255 range
        window.draw(timer);
    }
}; 