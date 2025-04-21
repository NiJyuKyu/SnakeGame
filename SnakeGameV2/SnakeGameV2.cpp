#include <iostream>
#include <conio.h>
#include <windows.h>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>
#include <iomanip>

using namespace std;

// Console color codes
enum Color {
    BLACK = 0,
    BLUE = 1,
    GREEN = 2,
    CYAN = 3,
    RED = 4,
    MAGENTA = 5,
    BROWN = 6,
    LIGHTGRAY = 7,
    DARKGRAY = 8,
    LIGHTBLUE = 9,
    LIGHTGREEN = 10,
    LIGHTCYAN = 11,
    LIGHTRED = 12,
    LIGHTMAGENTA = 13,
    YELLOW = 14,
    WHITE = 15
};

// Constants
const int WIDTH = 30;
const int HEIGHT = 20;
const char SNAKE_BODY = 'O';
const char SNAKE_HEAD = '@';
const char FOOD = '*';
const char EMPTY = ' ';
const char WALL_HORIZONTAL = '═';
const char WALL_VERTICAL = '║';
const char WALL_CORNER_TL = '╔';
const char WALL_CORNER_TR = '╗';
const char WALL_CORNER_BL = '╚';
const char WALL_CORNER_BR = '╝';

// Directions
enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };

// User structure for login system
struct User {
    string username;
    string password;
    int highScore;
};

// Snake segment structure
struct SnakeSegment {
    int x, y;
};

// Game state structure
struct GameState {
    bool gameOver;
    int score;
    Direction dir;
    vector<SnakeSegment> snake; // Dynamic data structure for snake body
    int foodX, foodY;
    User* currentUser; // Pointer to current user
    int speed; // Game speed (milliseconds between updates)
};

// Function prototypes
void SetConsoleColor(int textColor, int bgColor);
void CenterText(const string& text, int width, int textColor = WHITE, int bgColor = BLACK);
void DrawBox(int x, int y, int width, int height, int textColor = WHITE, int bgColor = BLACK);
void Setup(GameState* game);
void Draw(const GameState& game);
void Input(GameState* game);
void Logic(GameState* game);
void DrawMainMenu();
void DrawLoginMenu();
void DrawRegisterMenu();
bool Login(vector<User>& users, User** currentUser);
bool Register(vector<User>& users);
void SaveUsers(const vector<User>& users);
vector<User> LoadUsers();
void DisplayLeaderboard(const vector<User>& users);
void UpdateLeaderboard(vector<User>& users, User* currentUser, int score);
void DrawGameOver(int score, bool newHighScore);
void GotoXY(int x, int y);
void HideCursor();

int main() {
    // Set console title and size
    SetConsoleTitle(TEXT("Advanced Snake Game"));
    system("mode con: cols=80 lines=25");
    HideCursor();

    srand(static_cast<unsigned int>(time(0)));

    // Load users from file
    vector<User> users = LoadUsers();
    User* currentUser = nullptr;

    // Main menu
    int choice;
    bool loggedIn = false;

    do {
        DrawMainMenu();
        choice = _getch() - '0'; // Convert char to int

        switch (choice) {
        case 1: // Login
            loggedIn = Login(users, &currentUser);
            if (loggedIn) {
                system("cls");
                SetConsoleColor(LIGHTGREEN, BLACK);
                CenterText("Logged in as " + currentUser->username, 80);
                SetConsoleColor(WHITE, BLACK);
                Sleep(1500);
                choice = 0; // To start the game
            }
            break;
        case 2: // Register
            if (Register(users)) {
                system("cls");
                SetConsoleColor(LIGHTGREEN, BLACK);
                CenterText("Registration successful!", 80);
                SetConsoleColor(WHITE, BLACK);
                SaveUsers(users);
            }
            Sleep(1500);
            break;
        case 3: // View Leaderboard
            DisplayLeaderboard(users);
            break;
        case 4: // Play as Guest
            currentUser = nullptr;
            choice = 0; // To start the game
            break;
        case 5: // Exit
            system("cls");
            SetConsoleColor(YELLOW, BLACK);
            CenterText("Thanks for playing!", 80);
            SetConsoleColor(WHITE, BLACK);
            Sleep(1500);
            return 0;
        }

    } while (choice != 0);

    // Game initialization
    GameState game;
    game.currentUser = currentUser;
    Setup(&game);

    // Game loop
    while (!game.gameOver) {
        Draw(game);
        Input(&game);
        Logic(&game);
        Sleep(game.speed); // Game speed
    }

    // Game over
    bool newHighScore = false;
    if (currentUser != nullptr && game.score > currentUser->highScore) {
        currentUser->highScore = game.score;
        newHighScore = true;
        UpdateLeaderboard(users, currentUser, game.score);
        SaveUsers(users);
    }

    DrawGameOver(game.score, newHighScore);

    // Return to main menu
    _getch();
    main(); // Restart the program to show main menu again

    return 0;
}

// Utility function to set console text and background colors
void SetConsoleColor(int textColor, int bgColor) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, (bgColor << 4) | textColor);
}

// Utility function to center text in console
void CenterText(const string& text, int width, int textColor, int bgColor) {
    SetConsoleColor(textColor, bgColor);
    int spaces = (width - text.length()) / 2;
    cout << string(spaces, ' ') << text << string(spaces, ' ');
    if ((width - text.length()) % 2 != 0) cout << " ";
    SetConsoleColor(WHITE, BLACK);
    cout << endl;
}

// Utility function to move cursor to specific coordinates
void GotoXY(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// Utility function to hide the cursor
void HideCursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &info);
}

// Draw a box with borders
void DrawBox(int x, int y, int width, int height, int textColor, int bgColor) {
    SetConsoleColor(textColor, bgColor);

    // Draw top border
    GotoXY(x, y);
    cout << WALL_CORNER_TL;
    for (int i = 0; i < width - 2; i++) cout << WALL_HORIZONTAL;
    cout << WALL_CORNER_TR;

    // Draw side borders
    for (int i = 1; i < height - 1; i++) {
        GotoXY(x, y + i);
        cout << WALL_VERTICAL;
        GotoXY(x + width - 1, y + i);
        cout << WALL_VERTICAL;
    }

    // Draw bottom border
    GotoXY(x, y + height - 1);
    cout << WALL_CORNER_BL;
    for (int i = 0; i < width - 2; i++) cout << WALL_HORIZONTAL;
    cout << WALL_CORNER_BR;

    SetConsoleColor(WHITE, BLACK);
}

// Draw the main menu
void DrawMainMenu() {
    system("cls");

    // Draw title
    int y = 3;
    SetConsoleColor(YELLOW, BLACK);
    GotoXY(28, y++);
    cout << " _____ _   _    _    _  _______";
    GotoXY(28, y++);
    cout << "/  ___| \\ | |  / \\  | |/ / ____|";
    GotoXY(28, y++);
    cout << "\\ `--.|  \\| | / _ \\ | ' /|  _|  ";
    GotoXY(28, y++);
    cout << " `--. \\ . ` |/ ___ \\|  < | |___ ";
    GotoXY(28, y++);
    cout << "/\\__/ / |\\  / /   \\ \\ . \\|  ___|";
    GotoXY(28, y++);
    cout << "\\____/\\_| \\_\\/     \\_\\_|\\_\\_____|";
    y += 2;

    // Draw menu box
    DrawBox(25, y, 30, 10, CYAN, BLACK);

    // Draw menu options
    SetConsoleColor(WHITE, BLACK);
    GotoXY(35, y + 2);
    cout << "1. Login";
    GotoXY(35, y + 3);
    cout << "2. Register";
    GotoXY(35, y + 4);
    cout << "3. Leaderboard";
    GotoXY(35, y + 5);
    cout << "4. Play as Guest";
    GotoXY(35, y + 6);
    cout << "5. Exit";

    GotoXY(30, y + 8);
    SetConsoleColor(LIGHTGRAY, BLACK);
    cout << "Select an option (1-5): ";
}

// Draw login menu
void DrawLoginMenu() {
    system("cls");

    int y = 5;
    SetConsoleColor(CYAN, BLACK);
    GotoXY(33, y++);
    cout << "LOGIN MENU";
    y++;

    DrawBox(25, y, 30, 8, BLUE, BLACK);

    SetConsoleColor(WHITE, BLACK);
    GotoXY(28, y + 2);
    cout << "Username: ";

    GotoXY(28, y + 4);
    cout << "Password: ";
}

// Draw register menu
void DrawRegisterMenu() {
    system("cls");

    int y = 5;
    SetConsoleColor(GREEN, BLACK);
    GotoXY(32, y++);
    cout << "REGISTER MENU";
    y++;

    DrawBox(25, y, 30, 8, GREEN, BLACK);

    SetConsoleColor(WHITE, BLACK);
    GotoXY(28, y + 2);
    cout << "New Username: ";

    GotoXY(28, y + 4);
    cout << "New Password: ";
}

// Draw game over screen
void DrawGameOver(int score, bool newHighScore) {
    system("cls");

    int y = 5;
    SetConsoleColor(LIGHTRED, BLACK);
    GotoXY(26, y++);
    cout << "  _____          __  __ ______    ______      ________ _____  ";
    GotoXY(26, y++);
    cout << " / ____|   /\\   |  \\/  |  ____|  / __ \\ \\    / /  ____|  __ \\ ";
    GotoXY(26, y++);
    cout << "| |  __   /  \\  | \\  / | |__    | |  | \\ \\  / /| |__  | |__) |";
    GotoXY(26, y++);
    cout << "| | |_ | / /\\ \\ | |\\/| |  __|   | |  | |\\ \\/ / |  __| |  _  / ";
    GotoXY(26, y++);
    cout << "| |__| |/ ____ \\| |  | | |____  | |__| | \\  /  | |____| | \\ \\ ";
    GotoXY(26, y++);
    cout << " \\_____/_/    \\_\\_|  |_|______|  \\____/   \\/   |______|_|  \\_\\";
    y += 2;

    DrawBox(25, y, 30, 7, YELLOW, BLACK);

    SetConsoleColor(WHITE, BLACK);
    GotoXY(28, y + 2);
    cout << "Your Score: " << score;

    if (newHighScore) {
        GotoXY(28, y + 3);
        SetConsoleColor(LIGHTGREEN, BLACK);
        cout << "NEW HIGH SCORE!";
        SetConsoleColor(WHITE, BLACK);
    }

    GotoXY(28, y + 5);
    SetConsoleColor(LIGHTGRAY, BLACK);
    cout << "Press any key to continue...";
}

// Set up the initial game state
void Setup(GameState* game) {
    game->gameOver = false;
    game->dir = STOP;
    game->score = 0;
    game->speed = 150; // Initial game speed

    // Initialize snake with 3 segments
    game->snake.clear();
    SnakeSegment head;
    head.x = WIDTH / 2;
    head.y = HEIGHT / 2;
    game->snake.push_back(head);

    for (int i = 1; i < 3; i++) {
        SnakeSegment segment;
        segment.x = head.x - i;
        segment.y = head.y;
        game->snake.push_back(segment);
    }

    // Place food at random position
    game->foodX = rand() % (WIDTH - 4) + 2;
    game->foodY = rand() % (HEIGHT - 4) + 2;
}

// Draw the game board, snake, and food
void Draw(const GameState& game) {
    system("cls");

    // Draw title and info
    SetConsoleColor(YELLOW, BLACK);
    CenterText("SNAKE GAME", 80);

    string playerInfo = "Player: ";
    playerInfo += (game.currentUser ? game.currentUser->username : "Guest");
    playerInfo += " | Score: " + to_string(game.score);
    if (game.currentUser) {
        playerInfo += " | High Score: " + to_string(game.currentUser->highScore);
    }

    SetConsoleColor(CYAN, BLACK);
    CenterText(playerInfo, 80);
    cout << endl;

    // Create a 2D array to represent the game board
    char board[HEIGHT][WIDTH];

    // Initialize the board with empty spaces
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (y == 0 && x == 0)
                board[y][x] = WALL_CORNER_TL;
            else if (y == 0 && x == WIDTH - 1)
                board[y][x] = WALL_CORNER_TR;
            else if (y == HEIGHT - 1 && x == 0)
                board[y][x] = WALL_CORNER_BL;
            else if (y == HEIGHT - 1 && x == WIDTH - 1)
                board[y][x] = WALL_CORNER_BR;
            else if (y == 0 || y == HEIGHT - 1)
                board[y][x] = WALL_HORIZONTAL;
            else if (x == 0 || x == WIDTH - 1)
                board[y][x] = WALL_VERTICAL;
            else
                board[y][x] = EMPTY;
        }
    }

    // Add food to the board
    board[game.foodY][game.foodX] = FOOD;

    // Add snake to the board
    for (size_t i = 0; i < game.snake.size(); i++) {
        int x = game.snake[i].x;
        int y = game.snake[i].y;
        if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
            board[y][x] = (i == 0) ? SNAKE_HEAD : SNAKE_BODY;
        }
    }

    // Start the board at an offset to center it
    int offsetX = (80 - WIDTH * 2) / 2;
    int offsetY = 4;

    // Draw the board with colors
    for (int y = 0; y < HEIGHT; y++) {
        GotoXY(offsetX, offsetY + y);
        for (int x = 0; x < WIDTH; x++) {
            if (board[y][x] == WALL_HORIZONTAL || board[y][x] == WALL_VERTICAL ||
                board[y][x] == WALL_CORNER_TL || board[y][x] == WALL_CORNER_TR ||
                board[y][x] == WALL_CORNER_BL || board[y][x] == WALL_CORNER_BR) {
                SetConsoleColor(CYAN, BLACK);
            }
            else if (board[y][x] == SNAKE_HEAD) {
                SetConsoleColor(LIGHTGREEN, BLACK);
            }
            else if (board[y][x] == SNAKE_BODY) {
                SetConsoleColor(GREEN, BLACK);
            }
            else if (board[y][x] == FOOD) {
                SetConsoleColor(LIGHTRED, BLACK);
            }
            else {
                SetConsoleColor(WHITE, BLACK);
            }

            // Use double characters for better aspect ratio
            cout << board[y][x] << " ";
        }
    }

    // Draw controls at the bottom
    GotoXY(offsetX, offsetY + HEIGHT + 1);
    SetConsoleColor(WHITE, BLACK);
    cout << "Controls: W (Up), A (Left), S (Down), D (Right), X (Quit)";

    // Restore default color
    SetConsoleColor(WHITE, BLACK);
}

// Process user input
void Input(GameState* game) {
    if (_kbhit()) {
        switch (_getch()) {
        case 'a':
        case 'A':
            if (game->dir != RIGHT) game->dir = LEFT;
            break;
        case 'd':
        case 'D':
            if (game->dir != LEFT) game->dir = RIGHT;
            break;
        case 'w':
        case 'W':
            if (game->dir != DOWN) game->dir = UP;
            break;
        case 's':
        case 'S':
            if (game->dir != UP) game->dir = DOWN;
            break;
        case 'x':
        case 'X':
            game->gameOver = true;
            break;
        }
    }
}

// Update game logic
void Logic(GameState* game) {
    // If the game hasn't started yet, don't update
    if (game->dir == STOP) return;

    // Remember previous position of snake segments
    vector<SnakeSegment> prevPositions = game->snake;

    // Move the head
    switch (game->dir) {
    case LEFT:
        game->snake[0].x--;
        break;
    case RIGHT:
        game->snake[0].x++;
        break;
    case UP:
        game->snake[0].y--;
        break;
    case DOWN:
        game->snake[0].y++;
        break;
    default:
        break;
    }

    // Move the rest of the snake
    for (size_t i = 1; i < game->snake.size(); i++) {
        game->snake[i] = prevPositions[i - 1];
    }

    // Check for collisions with walls
    if (game->snake[0].x <= 0 || game->snake[0].x >= WIDTH - 1 ||
        game->snake[0].y <= 0 || game->snake[0].y >= HEIGHT - 1) {
        game->gameOver = true;
        return;
    }

    // Check for collisions with self
    for (size_t i = 1; i < game->snake.size(); i++) {
        if (game->snake[0].x == game->snake[i].x && game->snake[0].y == game->snake[i].y) {
            game->gameOver = true;
            return;
        }
    }

    // Check if food is eaten
    if (game->snake[0].x == game->foodX && game->snake[0].y == game->foodY) {
        // Increase score
        game->score += 10;

        // Add new segment to snake
        SnakeSegment newSegment = game->snake.back();
        game->snake.push_back(newSegment);

        // Generate new food
        bool validPosition;
        do {
            validPosition = true;
            game->foodX = rand() % (WIDTH - 4) + 2;
            game->foodY = rand() % (HEIGHT - 4) + 2;

            // Make sure food doesn't spawn on snake
            for (const auto& segment : game->snake) {
                if (game->foodX == segment.x && game->foodY == segment.y) {
                    validPosition = false;
                    break;
                }
            }
        } while (!validPosition);

        // Increase game speed slightly with each food eaten (up to a limit)
        if (game->speed > 50) {
            game->speed -= 5;
        }
    }
}

// Handle user login
bool Login(vector<User>& users, User** currentUser) {
    string username, password;

    DrawLoginMenu();

    GotoXY(38, 9);
    cin >> username;

    GotoXY(38, 11);
    // Simple masking of password
    password = "";
    char ch;
    while ((ch = _getch()) != 13) { // 13 is Enter key
        if (ch == 8) { // Backspace
            if (!password.empty()) {
                password.pop_back();
                cout << "\b \b";
            }
        }
        else {
            password += ch;
            cout << "*";
        }
    }

    for (User& user : users) {
        if (user.username == username && user.password == password) {
            *currentUser = &user;
            return true;
        }
    }

    system("cls");
    SetConsoleColor(LIGHTRED, BLACK);
    CenterText("Invalid username or password.", 80);
    SetConsoleColor(WHITE, BLACK);
    Sleep(1500);
    return false;
}

// Handle user registration
bool Register(vector<User>& users) {
    User newUser;

    DrawRegisterMenu();

    GotoXY(42, 9);
    cin >> newUser.username;

    // Check if username already exists
    for (const User& user : users) {
        if (user.username == newUser.username) {
            system("cls");
            SetConsoleColor(LIGHTRED, BLACK);
            CenterText("Username already exists.", 80);
            SetConsoleColor(WHITE, BLACK);
            Sleep(1500);
            return false;
        }
    }

    GotoXY(42, 11);
    // Simple masking of password
    newUser.password = "";
    char ch;
    while ((ch = _getch()) != 13) { // 13 is Enter key
        if (ch == 8) { // Backspace
            if (!newUser.password.empty()) {
                newUser.password.pop_back();
                cout << "\b \b";
            }
        }
        else {
            newUser.password += ch;
            cout << "*";
        }
    }

    newUser.highScore = 0;
    users.push_back(newUser);

    return true;
}

// Save users to file
void SaveUsers(const vector<User>& users) {
    ofstream file("users.txt");

    if (!file.is_open()) {
        system("cls");
        SetConsoleColor(LIGHTRED, BLACK);
        CenterText("Error opening file for saving users.", 80);
        SetConsoleColor(WHITE, BLACK);
        Sleep(1500);
        return;
    }

    for (const User& user : users) {
        file << user.username << " " << user.password << " " << user.highScore << endl;
    }

    file.close();
}

// Load users from file
vector<User> LoadUsers() {
    vector<User> users;
    ifstream file("users.txt");

    if (!file.is_open()) {
        return users; // Return empty vector if file doesn't exist
    }

    User user;
    while (file >> user.username >> user.password >> user.highScore) {
        users.push_back(user);
    }

    file.close();
    return users;
}

// Display leaderboard
void DisplayLeaderboard(const vector<User>& users) {
    system("cls");

    int y = 3;
    SetConsoleColor(YELLOW, BLACK);
    GotoXY(32, y++);
    cout << "LEADERBOARD";
    y++;

    // Create a copy of users for sorting
    vector<User> sortedUsers = users;

    // Sort users by high score in descending order
    sort(sortedUsers.begin(), sortedUsers.end(),
        [](const User& a, const User& b) {
            return a.highScore > b.highScore;
        });

    // Draw leaderboard box
    DrawBox(20, y, 40, 15, CYAN, BLACK);

    // Draw header
    GotoXY(22, y + 1);
    SetConsoleColor(LIGHTGREEN, BLACK);
    cout << left << setw(5) << "Rank" << setw(20) << "Username" << "High Score";
    SetConsoleColor(WHITE, BLACK);

    // Draw separator
    GotoXY(22, y + 2);
    cout << string(36, '-');

    // Draw entries
    int rank = 1;
    for (size_t i = 0; i < sortedUsers.size() && i < 10; i++) {
        GotoXY(22, y + 3 + i);
        if (i < 3) SetConsoleColor(YELLOW, BLACK); // Highlight top 3
        else SetConsoleColor(WHITE, BLACK);

        cout << left << setw(5) << rank++
            << setw(20) << sortedUsers[i].username
            << sortedUsers[i].highScore;
    }

    // If no users yet
    if (sortedUsers.empty()) {
        GotoXY(28, y + 7);
        SetConsoleColor(LIGHTGRAY, BLACK);
        cout << "No records yet!";
    }

    // Footer
    SetConsoleColor(WHITE, BLACK);
    GotoXY(25, y + 13);
    cout << "Press any key to return...";

    _getch();
}

// Update leaderboard with new score
void UpdateLeaderboard(vector<User>& users, User* currentUser, int score) {
    if (score > currentUser->highScore) {
        currentUser->highScore = score;
    }
}