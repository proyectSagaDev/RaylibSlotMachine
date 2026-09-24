#include "raylib.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <thread>
#include <chrono>
#include <unordered_set> // ✅ Required for tracking unique fruits
#include <random>
#include <algorithm> // ✅ Required for std::shuffle

using namespace std;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int GRID_COLUMNS = 3;
const int GRID_ROWS = 3;
const int CELL_SIZE = 100;
const int SPIN_SPEED = 20;
const int offset = 3; // Thickness of the outline


Sound spinSound;
Sound winSound;
Sound buttonClickSound;
Music menuMusic;
bool soundMuted = false; // ✅ Default: sound is ON
Texture2D soundOnIcon;
Texture2D soundOffIcon;

// Generate a 2D vector filled with random texture indices
vector<vector<int>> GenerateRandomGrid(int textureCount) {  
    vector<vector<int>> grid(GRID_ROWS, vector<int>(GRID_COLUMNS));  
    random_device rd;  
    mt19937 rng(rd());  

    for (int col = 0; col < GRID_COLUMNS; col++) {  
        vector<int> availableFruits;  
        for (int i = 0; i < textureCount; i++) {  
            availableFruits.push_back(i); // ✅ Fill column with all unique fruits  
        }  

        shuffle(availableFruits.begin(), availableFruits.end(), rng); // ✅ Shuffle before assigning  

        for (int row = 0; row < GRID_ROWS; row++) {  
            grid[row][col] = availableFruits[row]; // ✅ Assign unique fruit for each row  
        }  
    } 

    // ✅ Add a controlled chance (~15%) to force a match in the middle row  
    if (rand() % 7 == 0) { // ✅ 1 in 7 chance of forcing a match  
        int matchFruit = rand() % textureCount;  
        grid[GRID_ROWS / 2][0] = matchFruit;  
        grid[GRID_ROWS / 2][1] = matchFruit;  
        grid[GRID_ROWS / 2][2] = matchFruit;  
    }  


    return grid;  
}

// Draw the grid with the current textures
void DrawGrid(const vector<vector<int>>& grid, const vector<Texture2D>& fruitTextures) {
    int columnSpacing = 40; // Space between columns
    float fruitWidth = 100;
    float fruitHeight = 80; // Set custom fruit height

    for (int row = 0; row < GRID_ROWS; row++) {
        for (int col = 0; col < GRID_COLUMNS; col++) {
            // Adjust grid position & spacing
            int x = (SCREEN_WIDTH / 2 + 102) - (CELL_SIZE * GRID_COLUMNS / 2) + col * (CELL_SIZE + columnSpacing);
            int y = (SCREEN_HEIGHT / 2 - 40) - (CELL_SIZE * GRID_ROWS / 2) + row * CELL_SIZE;

            // Draw grid background
            DrawRectangle(x, y, CELL_SIZE - 3, CELL_SIZE - 3, WHITE);

            // Define the destination rectangle for scaling fruit textures
            Rectangle destRec = {
                x + (CELL_SIZE - fruitWidth) / 2,  // Center horizontally
                y + (CELL_SIZE - fruitHeight) / 2, // Center vertically
                fruitWidth, 
                fruitHeight
            }; 
            Rectangle sourceRec = {0, 0, (float)fruitTextures[grid[row][col]].width, (float)fruitTextures[grid[row][col]].height};
            Vector2 origin = {0, 0};

            // Draw the fruit texture with scaling
            DrawTexturePro(fruitTextures[grid[row][col]], sourceRec, destRec, origin, 0.0f, WHITE);
        }
    }
}

// Check if a button is clicked
bool IsButtonClicked(Rectangle button, Vector2 mousePoint) {
    if (CheckCollisionPointRec(mousePoint, button) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        PlaySound(buttonClickSound); // 🎵 Play button click sound
        return true;
    }
    return false;
}

// Draw button with appropriate texture based on mouse interaction
void DrawButton(Rectangle button, Vector2 mousePoint, Texture2D relaxedTexture, Texture2D pressedTexture) {
    Rectangle destRec = {button.x, button.y, button.width, button.height}; // Stretch to button size
    Rectangle sourceRec = {0, 0, (float)relaxedTexture.width, (float)relaxedTexture.height}; // Use full texture
    Vector2 origin = {0, 0}; // No rotation
    
    if (CheckCollisionPointRec(mousePoint, button) && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        DrawTexturePro(pressedTexture, sourceRec, destRec, origin, 0.0f, WHITE);
    } else {
        DrawTexturePro(relaxedTexture, sourceRec, destRec, origin, 0.0f, WHITE);
    }
}

// Menu screen with Play and Exit options
void MenuScreen(bool& startGame, const vector<Texture2D>& fruitTextures, 
    Texture2D startRelaxedButton, Texture2D startPressedButton, 
    Texture2D exitRelaxedButton, Texture2D exitPressedButton) {

    PlayMusicStream(menuMusic);
    menuMusic.looping = true; // ✅ Makes sure it repeats automatically

    int fruitSize = 80; // ✅ Define fruitSize before using it
    random_device rd;
    mt19937 rng(rd());
    vector<vector<int>> randomizedColumns(SCREEN_WIDTH / fruitSize); // ✅ Store shuffled fruits per column

    // ✅ Shuffle fruits uniquely for each column while ensuring no consecutive duplicates
    for (int col = 0; col < SCREEN_WIDTH / fruitSize; col++) {  
        vector<int> availableFruits;  
        for (size_t i = 0; i < fruitTextures.size(); i++) {  
            availableFruits.push_back(i);  
        }  
    
        shuffle(availableFruits.begin(), availableFruits.end(), rng); // ✅ Randomize fruit order
    
        vector<int> columnFruits;  
        columnFruits.push_back(availableFruits[0]); // ✅ Start with the first random fruit  
    
        // ✅ Ensure no adjacent fruits are identical  
        for (size_t i = 1; i < availableFruits.size(); i++) {  
            int nextFruit = availableFruits[i];  
    
            if (nextFruit == columnFruits.back()) {  
                swap(nextFruit, availableFruits[(i + 1) % availableFruits.size()]); // ✅ Swap to prevent stacking  
            }  
    
            columnFruits.push_back(nextFruit);  
        }  
    
        randomizedColumns[col] = columnFruits; // ✅ Store final separated fruits  
    }

    while (!WindowShouldClose() && !startGame) {
        Rectangle playButton = {SCREEN_WIDTH / 2 - 75, SCREEN_HEIGHT / 2 + 50, 150, 80};
        Rectangle exitButton = {SCREEN_WIDTH / 2 - 75, SCREEN_HEIGHT / 2 + 150, 150, 80};
        Rectangle muteButton = {SCREEN_WIDTH - 60, 20, 40, 40}; // ✅ Smaller mute button size
        Vector2 mousePoint = GetMousePosition();

        UpdateMusicStream(menuMusic); // ✅ Keeps the music playing smoothly

        if (IsButtonClicked(playButton, mousePoint)) {  
            StopMusicStream(menuMusic);  
            BeginDrawing();  
            ClearBackground(DARKGREEN);  
            DrawButton(playButton, mousePoint, startPressedButton, startPressedButton);  
            DrawButton(exitButton, mousePoint, exitRelaxedButton, exitPressedButton);  
            EndDrawing();  
            this_thread::sleep_for(chrono::milliseconds(200)); // ✅ Show pressed button momentarily  
            
            BeginDrawing();  
            ClearBackground(DARKGREEN);  
            DrawButton(playButton, mousePoint, startRelaxedButton, startPressedButton); // ✅ Show relaxed button before action  
            DrawButton(exitButton, mousePoint, exitRelaxedButton, exitPressedButton);  
            EndDrawing();  
            this_thread::sleep_for(chrono::milliseconds(200)); // ✅ Allow relaxed button state to show  
        
            startGame = true;  
            return;
        } if (IsButtonClicked(exitButton, mousePoint)) {  
            StopMusicStream(menuMusic);  
         
            BeginDrawing();  
            ClearBackground(DARKGREEN);  
            DrawButton(playButton, mousePoint, startRelaxedButton, startPressedButton);  
            DrawButton(exitButton, mousePoint, exitPressedButton, exitPressedButton);  
            EndDrawing();  
            this_thread::sleep_for(chrono::milliseconds(200));  
        
            BeginDrawing();  
            ClearBackground(DARKGREEN);  
            DrawButton(playButton, mousePoint, startRelaxedButton, startPressedButton);  
            DrawButton(exitButton, mousePoint, exitRelaxedButton, exitPressedButton);  
            EndDrawing();  
            this_thread::sleep_for(chrono::milliseconds(200));  
        
            CloseWindow(); // ✅ Action executes only after animation finishes  
            exit(0);  
        } else if (IsButtonClicked(muteButton, mousePoint)) {
            soundMuted = !soundMuted; // ✅ Toggle sound ON/OFF
            SetMasterVolume(soundMuted ? 0.0f : 0.4f);
        } else if (IsKeyPressed(KEY_M)) { // ✅ Toggle sound in the menu
            soundMuted = !soundMuted;
            SetMasterVolume(soundMuted ? 0.0f : 0.4f);
        }

        BeginDrawing();
        ClearBackground(DARKGREEN); // Set dark green background

        static float scrollOffset = 0;
        scrollOffset += 0.2f;
        if (scrollOffset >= SCREEN_HEIGHT) scrollOffset = 0;

        for (int col = 0; col < SCREEN_WIDTH; col += fruitSize) {
            for (int row = -SCREEN_HEIGHT; row < SCREEN_HEIGHT; row += fruitSize) {
                int fruitIndex = randomizedColumns[col / fruitSize][(row / fruitSize) % fruitTextures.size()]; // ✅ Uses pre-shuffled fruits per column
                DrawTexture(fruitTextures[fruitIndex], col, row + scrollOffset, WHITE);
            }
        }

        const int offset = 3;
        DrawText("Fruity", SCREEN_WIDTH / 2 - 140 - offset, SCREEN_HEIGHT / 3 - 30, 50, BLACK);
        DrawText("Fruity", SCREEN_WIDTH / 2 - 140 + offset, SCREEN_HEIGHT / 3 - 30, 50, BLACK);
        DrawText("Fruity", SCREEN_WIDTH / 2 - 140, SCREEN_HEIGHT / 3 - 30 - offset, 50, BLACK);
        DrawText("Fruity", SCREEN_WIDTH / 2 - 140, SCREEN_HEIGHT / 3 - 30 + offset, 50, BLACK);

        DrawText("Slot Machine", SCREEN_WIDTH / 2 - 160 - offset, SCREEN_HEIGHT / 3 + 30, 50, BLACK);
        DrawText("Slot Machine", SCREEN_WIDTH / 2 - 160 + offset, SCREEN_HEIGHT / 3 + 30, 50, BLACK);
        DrawText("Slot Machine", SCREEN_WIDTH / 2 - 160, SCREEN_HEIGHT / 3 + 30 - offset, 50, BLACK);
        DrawText("Slot Machine", SCREEN_WIDTH / 2 - 160, SCREEN_HEIGHT / 3 + 30 + offset, 50, BLACK);

        DrawText("Fruity", SCREEN_WIDTH / 2 - 140, SCREEN_HEIGHT / 3 - 30, 50, ORANGE);
        DrawText("Slot Machine", SCREEN_WIDTH / 2 - 160, SCREEN_HEIGHT / 3 + 30, 50, DARKPURPLE);

        DrawButton(playButton, mousePoint, startRelaxedButton, startPressedButton);
        DrawButton(exitButton, mousePoint, exitRelaxedButton, exitPressedButton);

        // ✅ Draw mute button icon dynamically (scaled properly)
        DrawTexturePro(soundMuted ? soundOffIcon : soundOnIcon, 
            {0, 0, (float)soundOffIcon.width, (float)soundOffIcon.height}, 
            {muteButton.x, muteButton.y, muteButton.width, muteButton.height}, 
            {0, 0}, 0.0f, WHITE);

        EndDrawing();
    }
}

// Pause menu with Resume and Quit to Menu options
void PauseMenu(bool& paused, bool& quitToMainMenu, Texture2D resumeRelaxedButton, Texture2D resumePressedButton, 
    Texture2D menuRelaxedButton, Texture2D menuPressedButton) {
    while (!WindowShouldClose() && paused) {
        Rectangle resumeButton = {SCREEN_WIDTH / 2 - 220, SCREEN_HEIGHT / 2 - 60, 200, 100};
        Rectangle menuButton = {SCREEN_WIDTH / 2 + 20, SCREEN_HEIGHT / 2 - 60, 200, 100};
        Vector2 mousePoint = GetMousePosition();
        if (IsButtonClicked(resumeButton, mousePoint)) {  

            BeginDrawing();  
            ClearBackground(DARKGREEN);  
            DrawButton(resumeButton, mousePoint, resumePressedButton, resumePressedButton);  
            DrawButton(menuButton, mousePoint, menuRelaxedButton, menuPressedButton);  
            EndDrawing();  
            this_thread::sleep_for(chrono::milliseconds(200));  
        
            BeginDrawing();  
            ClearBackground(DARKGREEN);  
            DrawButton(resumeButton, mousePoint, resumeRelaxedButton, resumePressedButton);  
            DrawButton(menuButton, mousePoint, menuRelaxedButton, menuPressedButton);  
            EndDrawing();  
            this_thread::sleep_for(chrono::milliseconds(200));  
        
            paused = false; // ✅ Resume action executes only after animation finishes  
            return;  
        } else if (IsButtonClicked(menuButton, mousePoint)) {  

            BeginDrawing();  
            ClearBackground(DARKGREEN);  
            DrawButton(resumeButton, mousePoint, resumeRelaxedButton, resumePressedButton);  
            DrawButton(menuButton, mousePoint, menuPressedButton, menuPressedButton);  
            EndDrawing();  
            this_thread::sleep_for(chrono::milliseconds(200));  
         
            BeginDrawing();  
            ClearBackground(DARKGREEN);  
            DrawButton(resumeButton, mousePoint, resumeRelaxedButton, resumePressedButton);  
            DrawButton(menuButton, mousePoint, menuRelaxedButton, menuPressedButton);  
            EndDrawing();  
            this_thread::sleep_for(chrono::milliseconds(200));  
        
            quitToMainMenu = true; // ✅ Menu action executes only after animation finishes  
            paused = false;  
            return;  
        } else if (IsButtonClicked(menuButton, mousePoint)) {
            BeginDrawing();
            ClearBackground(DARKGREEN);
            DrawButton(resumeButton, mousePoint, resumeRelaxedButton, resumePressedButton);
            DrawButton(menuButton, mousePoint, menuRelaxedButton, menuPressedButton); // Show pressed state
            EndDrawing();

            this_thread::sleep_for(chrono::milliseconds(200)); // Short delay before quitting to menu
            quitToMainMenu = true;
            paused = false;
            return;
        }
        BeginDrawing();
        ClearBackground(DARKGREEN); // Set green background for pause menu

        DrawButton(resumeButton, mousePoint, resumeRelaxedButton, resumePressedButton);
        DrawButton(menuButton, mousePoint, menuRelaxedButton, menuPressedButton);

        EndDrawing();
        }
}

void DrawPayoutGrid(const vector<Texture2D>& fruitTextures) {
    int startX = 50;
    int startY = 100;
    int spacingY = 50;
    
    vector<int> payouts = {150, 25, 500, 250, 10, 100, 50, 1000}; // Payouts for each fruit

    for (size_t i = 0; i < fruitTextures.size(); i++) {
        // Draw fruit image
        DrawTexture(fruitTextures[i], startX, startY + (i * spacingY), WHITE);
        
        // Display equal sign and payout amount
        DrawText(("= $" + to_string(payouts[i])).c_str(), startX + 50, startY + (i * spacingY) + 10, 20, YELLOW);
    }
}

void DrawGameTitle() {
    
    // 🔲 Draw black border behind text (offset in 8 directions)
    DrawText("Fruity", SCREEN_WIDTH / 2 + 60 - offset, 20, 50, BLACK);
    DrawText("Fruity", SCREEN_WIDTH / 2 + 60 + offset, 20, 50, BLACK);
    DrawText("Fruity", SCREEN_WIDTH / 2 + 60, 20 - offset, 50, BLACK);
    DrawText("Fruity", SCREEN_WIDTH / 2 + 60, 20 + offset, 50, BLACK);

    DrawText("Slot Machine", SCREEN_WIDTH / 2 - offset, 70, 50, BLACK);
    DrawText("Slot Machine", SCREEN_WIDTH / 2 + offset, 70, 50, BLACK);
    DrawText("Slot Machine", SCREEN_WIDTH / 2, 70 - offset, 50, BLACK);
    DrawText("Slot Machine", SCREEN_WIDTH / 2, 70 + offset, 50, BLACK);

    // 🎨 Now draw original title on top of the black border
    DrawText("Fruity", SCREEN_WIDTH / 2 + 60, 20, 50, ORANGE);
    DrawText("Slot Machine", SCREEN_WIDTH / 2, 70, 50, DARKPURPLE);
}

// Spin reels column by column
void SpinReels(vector<vector<int>>& grid, const vector<Texture2D>& fruitTextures, Texture2D slotOverlay, int& playerBalance) {
    float fruitWidth = 100;  
    float fruitHeight = 80;  

    PlaySound(spinSound);  

    random_device rd;  
    mt19937 rng(rd());  

    vector<vector<int>> finalGrid = grid; // ✅ Store final fruit positions before spinning  

    // ✅ Assign unique fruits per column before spinning  
    for (int col = 0; col < GRID_COLUMNS; col++) {  
        vector<int> availableFruits;  
        for (size_t i = 0; i < fruitTextures.size(); i++) { // ✅ Fixes signed-unsigned comparison
            availableFruits.push_back(i);  
        }  

        shuffle(availableFruits.begin(), availableFruits.end(), rng);  

        for (int row = 0; row < GRID_ROWS; row++) {  
            finalGrid[row][col] = availableFruits[row]; // ✅ Set final fruit positions  
        }  
    }  

    // ✅ Animate each column separately  
    for (int col = 0; col < GRID_COLUMNS; col++) {  
        for (int step = 0; step < 3.5 * GRID_ROWS * CELL_SIZE; step += SPIN_SPEED) {  
            BeginDrawing();  
            ClearBackground(DARKGREEN);  

            for (int r = 0; r < GRID_ROWS; r++) {  
                for (int c = 0; c < GRID_COLUMNS; c++) {  
                    int x = (SCREEN_WIDTH / 2 + 102) - (CELL_SIZE * GRID_COLUMNS / 2) + c * (CELL_SIZE + 40);  
                    int y = (SCREEN_HEIGHT / 2 - 40) - (CELL_SIZE * GRID_ROWS / 2) + r * CELL_SIZE;  

                    // ✅ Only spin the current column  
                    if (c == col) {  
                        int spinProgress = step / CELL_SIZE;  
                        int fruitIndex = (spinProgress + r) % GRID_ROWS;  

                        // ✅ Slow down at the end, smoothly landing on the final fruit  
                        if (step >= (3.5 * GRID_ROWS * CELL_SIZE - SPIN_SPEED * GRID_ROWS)) {  
                            fruitIndex = r; // ✅ Force it to stop directly on the final fruit  
                        }  

                        Rectangle destRec = {x + (CELL_SIZE - fruitWidth) / 2, y + (CELL_SIZE - fruitHeight) / 2 + step % CELL_SIZE, fruitWidth, fruitHeight};  
                        Rectangle sourceRec = {0, 0, (float)fruitTextures[finalGrid[fruitIndex][c]].width, (float)fruitTextures[finalGrid[fruitIndex][c]].height};  
                        Vector2 origin = {0, 0};  

                        DrawRectangle(x, y + step % CELL_SIZE, CELL_SIZE - 5, CELL_SIZE - 5, WHITE);  
                        DrawTexturePro(fruitTextures[finalGrid[fruitIndex][c]], sourceRec, destRec, origin, 0.0f, WHITE);  
                    } else {  
                        // ✅ Keep static columns visible  
                        Rectangle destRec = {x + (CELL_SIZE - fruitWidth) / 2, y + (CELL_SIZE - fruitHeight) / 2, fruitWidth, fruitHeight};  
                        Rectangle sourceRec = {0, 0, (float)fruitTextures[grid[r][c]].width, (float)fruitTextures[grid[r][c]].height};  
                        Vector2 origin = {0, 0};  

                        DrawRectangle(x, y, CELL_SIZE - 5, CELL_SIZE - 5, WHITE);  
                        DrawTexturePro(fruitTextures[grid[r][c]], sourceRec, destRec, origin, 0.0f, WHITE);  
                    }  
                }  
            }  

            // ✅ Keep slot machine overlay visible  
            Rectangle overlayDest = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};  
            Rectangle overlaySource = {0, 0, (float)slotOverlay.width, (float)slotOverlay.height};  
            Vector2 overlayOrigin = {0, 0};  
            DrawTexturePro(slotOverlay, overlaySource, overlayDest, overlayOrigin, 0.0f, WHITE);  

            // ✅ Display balance dynamically  
            DrawText(("Balance: $" + to_string(playerBalance)).c_str(), 50, SCREEN_HEIGHT - 50, 20, YELLOW);  

            // ✅ Keep payout grid visible  
            DrawPayoutGrid(fruitTextures);  

            // ✅ Maintain game title visibility  
            DrawGameTitle();  

            Rectangle messageScreen = {SCREEN_WIDTH / 2 - 15, SCREEN_HEIGHT - 210, 300, 180}; // ✅ Centered & neat

             DrawRectangleRec(messageScreen, BLACK); // ✅ Background for the screen
             DrawRectangleLinesEx(messageScreen, 5, DARKGREEN); // ✅ Adds a border

            EndDrawing();  
        }  

        this_thread::sleep_for(chrono::milliseconds(500)); // ✅ Pause before moving to the next column  

        // ✅ Reveal final fruit positions for this column after spinning stops  
        for (int row = 0; row < GRID_ROWS; row++) {  
            grid[row][col] = finalGrid[row][col]; // ✅ Assign final fruits only when column stops  
        }  
    }  

    StopSound(spinSound);  
}

void CheckMiddleRowMatch(const vector<vector<int>>& grid, string& matchMessage, int& playerBalance) {
    int rowIndex = GRID_ROWS / 2; // Middle row
    int winningSymbol = grid[rowIndex][0];

    
    if (grid[rowIndex][0] == grid[rowIndex][1] && grid[rowIndex][1] == grid[rowIndex][2]) {

        PlaySound(winSound); // 🎵 Play win sound on match

        matchMessage = "You got \n\ta match!";

        // Reward based on fruit type
        if (winningSymbol == 7) playerBalance += 1000; // Seven
        else if (winningSymbol == 2) playerBalance += 500; // Bar
        else if (winningSymbol == 3) playerBalance += 250; // Bell
        else if (winningSymbol == 0) playerBalance += 150; // Cherry
        else if (winningSymbol == 5) playerBalance += 100; // Orange
        else if (winningSymbol == 6) playerBalance += 50;  // Pear
        else if (winningSymbol == 1) playerBalance += 25;  // Watermelon
        else if (winningSymbol == 4) playerBalance += 10;  // Coconut
    } else {
        matchMessage = "No match!\n\tAgain?";
    }

}

// Main function
int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Slot Machine");
    SetTargetFPS(60);
    SetExitKey(0); // Disable automatic ESC key closing

    InitAudioDevice(); // ✅ Initialize audio
    spinSound = LoadSound("sounds/spin.mp3");
    winSound = LoadSound("sounds/win.ogg");
    buttonClickSound = LoadSound("sounds/buttonClick.ogg");
    menuMusic = LoadMusicStream("sounds/menuMusic.wav");
    soundOnIcon = LoadTexture("images/soundOn.png");
    soundOffIcon = LoadTexture("images/soundOff.png");
    SetMasterVolume(0.4f); // ✅ Adjust volume (1.0 = full, 0.4 = lower)
    bool hasPlayed = false; // ✅ Tracks if player has started playing
    float faceTimer = 0;  // ✅ Tracks animation time
    int faceIndex = 0;    // ✅ Tracks which face is displayed

    //Overlay texture
    Texture2D slotOverlay = LoadTexture("images/pixel_art_slot_machine_-_machine_frame2.png");

    // Load fruit textures
    vector<string> textureFiles = {
        "cherry.png", "watermelon.png", "bar.png", "bell.png",
        "coconut.png", "orange.png", "pear.png", "seven.png"
    };
    
    vector<Texture2D> fruitTextures;
    for (const auto& file : textureFiles) {
        fruitTextures.push_back(LoadTexture(("images/" + file).c_str()));
    }

    // Load button textures
    Texture2D startRelaxedButton = LoadTexture("images/startRelaxedButton.png");
    Texture2D startPressedButton = LoadTexture("images/startPressedButton.png");
    Texture2D exitRelaxedButton = LoadTexture("images/exitRelaxedButton.png");
    Texture2D exitPressedButton = LoadTexture("images/exitPressedButton.png");
    Texture2D resumeRelaxedButton = LoadTexture("images/resumeRelaxedButton.png");
    Texture2D resumePressedButton = LoadTexture("images/resumePressedButton.png");
    Texture2D menuRelaxedButton = LoadTexture("images/menuRelaxedButton.png");
    Texture2D menuPressedButton = LoadTexture("images/menuPressedButton.png");
    Texture2D soundOnIcon = LoadTexture("images/sounds_on.png");
    Texture2D soundOffIcon = LoadTexture("images/sounds_off.png");
    Texture2D happyFace = LoadTexture("images/happyFace.png");  
    Texture2D smileyFace = LoadTexture("images/smileyFace.png");  
    Texture2D blinkingFace = LoadTexture("images/blinkingFace.png");  

    vector<vector<int>> grid = GenerateRandomGrid(fruitTextures.size());
    bool startGame = false;
    bool quitToMainMenu = false;
    int playerBalance = 100;
    string matchMessage = "";

    MenuScreen(startGame, fruitTextures, startRelaxedButton, startPressedButton, exitRelaxedButton, exitPressedButton);

    while (!WindowShouldClose() && startGame) {
        bool paused = false;

        if (IsKeyPressed(KEY_ESCAPE)) {
            paused = true;
            PauseMenu(paused, quitToMainMenu, resumeRelaxedButton, resumePressedButton, menuRelaxedButton, menuPressedButton);

            if (quitToMainMenu) {  
                startGame = false;  
                quitToMainMenu = false; // ✅ Reset flag  
                hasPlayed = false; // ✅ Reset smiley animation  
                faceIndex = 0; // ✅ Reset face animation sequence  
                faceTimer = 0; // ✅ Reset animation timer  
                matchMessage = ""; // ✅ Clear previous match message  
                grid = GenerateRandomGrid(fruitTextures.size()); // ✅ Reinitialize slot machine  
                MenuScreen(startGame, fruitTextures, startRelaxedButton, startPressedButton, exitRelaxedButton, exitPressedButton);  
            }
        }

        if (!paused && IsKeyPressed(KEY_E)) {  
            if (playerBalance >= 5) { // ✅ Only allow spinning if balance is at least $5  
                hasPlayed = true; // ✅ First spin removes the smiley  
                playerBalance -= 5; // ✅ Deduct $5 for playing  

                SpinReels(grid, fruitTextures, slotOverlay, playerBalance);  
                CheckMiddleRowMatch(grid, matchMessage, playerBalance); // ✅ Update balance if they win  
            } else {  
                matchMessage = "Coins \nRequired!"; // ✅ Show warning when funds are too low  
            }  
        }

        Rectangle destRec = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT}; // Full-screen overlay
        Rectangle sourceRec = {0, 0, (float)slotOverlay.width, (float)slotOverlay.height}; // Original image size
        Vector2 origin = {0, 0}; // No rotation

        if (!paused && IsKeyPressed(KEY_P)) {
            // Force middle row to match with a random fruit
            hasPlayed = true; // ✅ First spin removes the smiley  
            int forcedSymbol = GetRandomValue(0, fruitTextures.size() - 1);
            for (int col = 0; col < GRID_COLUMNS; col++) {
                grid[GRID_ROWS / 2][col] = forcedSymbol;
            }
            
            CheckMiddleRowMatch(grid, matchMessage, playerBalance); // Check match after forced spin
        }

        if (IsKeyPressed(KEY_M)) { // ✅ Toggle sound when "M" is pressed
            soundMuted = !soundMuted;
            SetMasterVolume(soundMuted ? 0.0f : 0.4f);
        }

        BeginDrawing();
        ClearBackground(DARKGREEN); // Set dark green background

        DrawGrid(grid, fruitTextures);
        DrawTexturePro(slotOverlay, sourceRec, destRec, origin, 0.0f, WHITE);

        // Draw game title on top of the overlay
        DrawGameTitle();

        DrawText("Press [E] to spin!", SCREEN_WIDTH / 2 + 50, SCREEN_HEIGHT - 230, 20, BLACK);
        DrawText("Press [ESC] for Pause Menu", SCREEN_WIDTH - 750, SCREEN_HEIGHT - 570, 11, LIGHTGRAY);

        // ✅ Balance is now drawn **only inside main()**, keeping it visible!
        DrawText(("Balance: $" + to_string(playerBalance)).c_str(), 50, SCREEN_HEIGHT - 50, 20, YELLOW);

        // ✅ Payout grid stays visible during gameplay & spinning
        DrawPayoutGrid(fruitTextures);

        Rectangle messageScreen = {SCREEN_WIDTH / 2 - 15, SCREEN_HEIGHT - 210, 300, 180}; // ✅ Centered & neat

        DrawRectangleRec(messageScreen, BLACK); // ✅ Background for the screen
        DrawRectangleLinesEx(messageScreen, 5, DARKGREEN); // ✅ Adds a border

        if (!hasPlayed) { // ✅ Animate face sequence before playing  
            faceTimer += GetFrameTime(); // ✅ Update animation timer  
        
            if (faceTimer > 0.8f) { // ✅ Switch face every 0.8 seconds  
                faceIndex = (faceIndex + 1) % 3; // ✅ Cycle through 3 faces  
                faceTimer = 0; // ✅ Reset timer  
            }  
        
            // ✅ Assign different sizes for each face
            Texture2D currentFace;
            float scaleFactor;
            
            if (faceIndex == 0) { 
                currentFace = happyFace; 
                scaleFactor = 0.4f; // ✅ Slightly smaller
            } else if (faceIndex == 1) { 
                currentFace = smileyFace; 
                scaleFactor = 0.3f; // ✅ Medium size
            } else { 
                currentFace = blinkingFace; 
                scaleFactor = 0.35f; // ✅ Slightly larger
            }
        
            Rectangle sourceRec = {0, 0, (float)currentFace.width, (float)currentFace.height};  
            Rectangle destRec = {messageScreen.x + messageScreen.width / 2 - (currentFace.width * scaleFactor) / 2 - 40,  
                                 messageScreen.y + messageScreen.height / 2 - (currentFace.height * scaleFactor) / 2 + 5,  
                                 currentFace.width * scaleFactor, currentFace.height * scaleFactor};  
        
            DrawTexturePro(currentFace, sourceRec, destRec, {0, 0}, 0.0f, WHITE);
        }
        
        if (!hasPlayed) { // ✅ Show only before first spin  
            DrawText("Good", messageScreen.x + messageScreen.width / 2 + 40, messageScreen.y + messageScreen.height / 2 - 10, 30, WHITE);
            DrawText("Luck!", messageScreen.x + messageScreen.width / 2 + 40, messageScreen.y + messageScreen.height / 2 + 25, 30, WHITE);
        }
        DrawText(matchMessage.c_str(), messageScreen.x + 55, messageScreen.y + 40, 45, WHITE); // ✅ Draw message inside

        EndDrawing();
    }

    // Unload textures
    for (auto& texture : fruitTextures) {
        UnloadTexture(texture);
    }
    UnloadTexture(startRelaxedButton);
    UnloadTexture(startPressedButton);
    UnloadTexture(exitRelaxedButton);
    UnloadTexture(exitPressedButton);
    UnloadTexture(resumeRelaxedButton);
    UnloadTexture(resumePressedButton);
    UnloadTexture(menuRelaxedButton);
    UnloadTexture(menuPressedButton);
    UnloadTexture(happyFace);  
    UnloadTexture(smileyFace);  
    UnloadTexture(blinkingFace);  

    UnloadSound(spinSound);
    UnloadSound(winSound);
    UnloadSound(buttonClickSound);
    UnloadTexture(soundOnIcon);
    UnloadTexture(soundOffIcon);
    CloseAudioDevice(); // ✅ Closes Raylib's audio system

    CloseWindow();

    return 0;
}