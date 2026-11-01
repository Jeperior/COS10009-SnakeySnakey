#include "include/raylib.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Define Constants
// Screen dimensions and grid settings
#define SCREEN_WIDTH 840 
#define SCREEN_HEIGHT 840
#define GRID_SIZE 40                                    // Size of each grid cell
#define GRID_WIDTH (SCREEN_WIDTH / GRID_SIZE)           // Number of columns in the grid
#define GRID_HEIGHT (SCREEN_HEIGHT / GRID_SIZE)         // Number of rows in the grid

// Snake size and spacing
#define GRID_SPACE 0                                    // Space between snake segments
#define MAX_SNAKE_LENGTH (GRID_WIDTH * GRID_HEIGHT)     // Maximum length of the snake

// Bomb Lifetime and spawn settings
#define BOMB_LIFETIME 7.0f                              // Initial bomb lifetime second 
#define MAX_BOMBS 10                                    // Maximum number of bombs that can be active at once
#define BOMBS_PER_SPAWN 2                               // Number of bombs to spawn at once for the first time

// Decoration counts
#define MUSHROOM_COUNT 5 
#define FLOWER_COUNT 8
#define FERN_COUNT 50
#define FLOWER2_COUNT 13

// Particle counts
#define MAX_PARTICLES 300

// Texture Assets
Texture2D texSnakeHead; 
Texture2D texSnakeHeadDead;
Texture2D texSnakeBody; 
Texture2D texFoodNormal;
Texture2D texFoodEpic;
Texture2D texFoodSuper;
Texture2D texBomb;
Texture2D texGrass;
Texture2D texMushroom; 
Texture2D texFlower;
Texture2D texFlower2;
Texture2D texFern;

// Sound Assets
Sound sndEat;
Sound sndGameOver;
Sound sndMenu;
Sound sndLevelUp;
Sound sndEnter; 
Sound sndBomb;

// Music Assets
Music musicMenu;
Music musicRunning;

// Arrays for decoration positions
int mushroomPositions[MUSHROOM_COUNT][2] = {           // Mushroom Coordinates 
    {3, 5},
    {7, 2},
    {12, 10},
    {15, 18},
    {20, 6}
};
int flowerPositions[FLOWER_COUNT][2] = {               // Flower Coordinates
    {2, 3},
    {5, 7},
    {8, 12},
    {10, 4},
    {13, 15},
    {16, 9},
    {18, 6},
    {20, 13}
};
int fernPositions[FERN_COUNT][2] = {                    // Fern Coordinates
    {1, 1}, {2, 4}, {3, 7}, {4, 10}, {5, 13},
    {6, 16}, {7, 19}, {8, 2}, {9, 5}, {10, 8},
    {11, 11}, {12, 14}, {13, 17}, {14, 20}, {15, 3},
    {16, 6}, {17, 9}, {18, 12}, {19, 15}, {20, 18},
    {1, 8}, {2, 11}, {3, 14}, {4, 17}, {5, 20},
    {6, 3}, {7, 6}, {8, 9}, {9, 12}, {10, 15},
    {11, 18}, {12, 1}, {13, 4}, {14, 7}, {15, 10},
    {16, 13}, {17, 16}, {18, 19}, {19, 2}, {20, 5},
    {1, 13}, {2, 16}, {3, 19}, {4, 2}, {5, 5},
    {6, 8}, {7, 11}, {8, 14}, {9, 17}, {10, 20}
};
int flower2Positions[FLOWER2_COUNT][2] = {              // Flower2 Coordinates
    {1, 2},
    {4, 8},
    {6, 15},
    {9, 3},
    {11, 12},
    {13, 7},
    {15, 18},
    {17, 5},
    {18, 10},
    {19, 14},
    {7, 19},
    {12, 17},
    {20, 4}
};

// Global Variables

float globalTime = 0.0f;                            // Tracks the total time since the game started
float timePlayed = 0.0f;                            // Time played in the current game session

// Best score and best time on main menu
int bestScore = 0;                                  // Best score achieved in the game
float bestTime = 0.0f;                              // Best time achieved in the game        

// Bomb 
int bombsToSpawn = 0;                               // Number of bombs to spawn 
float bombSpawnTimer = 0.0f;                        // Timer for bomb spawning
float bombSpawnInterval = 15.0f;                    // Initial interval for bomb spawning in seconds

// Define food lifetime
float foodTimer = 0.0f;                             // Timer for food lifetime
float foodLifetime = 5.0f;                          // food lasts 5 seconds

// Define level variables
int currentLevel = 1;                               // Current level of the game
int pointToNextLevel = 20;                          // Points needed to level up

// Structures and Enumerations
// Particle System
typedef struct {
    float x, y;                                     // Position of the particle
    float vx, vy;                                   // Velocity of the particle
    float life;                                     // Remaining life before fading out
    Color color;                                    // Color of the particle
    bool active;                                    // Whether the particle is active or not        
} Particle;
Particle particles[MAX_PARTICLES];                  // Array of particles

// Snake segment structure
typedef struct {
    int x;
    int y;
} SnakeSegment;
SnakeSegment snake[GRID_WIDTH * GRID_WIDTH];        // Array for snake segments
int snakeLength = 1;                                // Initial length of the snake

// Difficulty levels
typedef enum {
    DIFFICULTY_EASY,
    DIFFICULTY_MEDIUM,
    DIFFICULTY_HARD
} Difficulty;
Difficulty currentDifficulty = DIFFICULTY_MEDIUM;   // Initial difficulty

// Game state
typedef enum {
    GAME_MENU,
    GAME_RUNNING,
    GAME_OVER
} GameState;

// Direction
typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

// Food
typedef struct{
    int x;                                          // X position of the food
    int y;                                          // Y position of the food
    int scoreValue;                                 // Point value of the food
    Color color;                                    // Color for particle
    int type;                                       // 0: Normal, 1: Epic, 2: Super
} Food;
Food food;                                          // Current food on the grid

// Declare Bomb
typedef struct{
    int x;
    int y;
    bool active;                                    // Whether the bomb is active or not                    
    float timer;                                    // How long the bomb has been active
    float lifetime;                                 // How long the bomb lasts before it explodes
} Bomb;
Bomb bombs[MAX_BOMBS];                              // Array of bombs

// Snake Direction and speed when the game starts
Direction currentDirection = DIR_UP;                // Initial direction of the snake
float moveTimer = 0;                                // Timer for snake movement
float moveDelay = 0.15f;                            // Initial speed for snake when the game starts
GameState gameState = GAME_MENU;                    // Initial game state

// Function Prototypes
void update(void);                                  // Update game logic and handling input
void draw(void);                                    // Draw the game elements(GUI)
void spawnFood(void);                               // Spawn food at random position
void spawnBomb(void);                               // Spawn bomb at random position
bool checkSelfCollision(void);                      // Check if the snake collides with itself
bool checkBorderCollision(void);                    // Check if the snake collides with the border
void resetGame(void);                               // Reset the game state
void applyDifficultySettings(void);                 // Apply difficulty settings based on currentDifficulty

// Save best score and time to file
void SaveBestStats() {
    FILE *file = fopen("best_stats.txt", "w");      
    if (file) {
        fprintf(file, "%d\n%.2f\n", bestScore, bestTime);
        fclose(file);
    }
}

// Load best score and time from file
void LoadBestStats() {
    FILE *file = fopen("best_stats.txt", "r");
    if (file) {
        fscanf(file, "%d\n%f\n", &bestScore, &bestTime);
        fclose(file);
    }
}

// Difficulty settings
void applyDifficultySettings() {
    switch (currentDifficulty) {
        case DIFFICULTY_EASY:
            moveDelay = 0.2f;
            foodLifetime = 7.0f;
            bombSpawnInterval = 20.0f;
            break;
        case DIFFICULTY_MEDIUM:
            moveDelay = 0.15f;
            foodLifetime = 5.0f;
            bombSpawnInterval = 15.0f;
            break;
        case DIFFICULTY_HARD:
            moveDelay = 0.1f;
            foodLifetime = 3.0f;
            bombSpawnInterval = 10.0f;
            break;
    }
}

// Spawn Food
void spawnFood() {                               
    bool validPosition = false;
    // Repeat until a valid position is found
    while (!validPosition) {
        // Randomly generate food position within the grid
        food.x = GetRandomValue(0, GRID_WIDTH - 1);
        food.y = GetRandomValue(0, GRID_HEIGHT - 1);

        // Make sure food does not spawn on the snake
        validPosition = true;

        // Check if food position overlaps with the snake
        for (int i = 0; i < snakeLength; i++) {
            if (snake[i].x == food.x && snake[i].y == food.y) {

                // If overlapping with snake, find a new position
                validPosition = false;
                break;
            }
        }
    }

    // Randomly assign food type and color
    // Weighted random / Spawn Chance: Normal (85%), Epic (10%), Super (5%)
    int roll = GetRandomValue(1, 100);

    if (roll <= 85) {
        food.scoreValue = 1;            // Score value for Normal food
        food.color = RED;
        food.type = 0; 
    } else if (roll <= 95) {
        food.scoreValue = 3;            // Score value for Epic food
        food.color = GREEN;
        food.type = 1;
    } else {
        food.scoreValue = 5;            // Score value for Super food
        food.color = BLUE;
        food.type = 2;
    }
    foodTimer = 0.0f; // Reset food timer
}

// Spawn Bomb randomly on the grid without overlapping with the snake or food
void spawnBomb() {
    // Loop through all bombs to find an inactive bomb slot
    for (int i = 0; i < MAX_BOMBS; i++) {
        if (!bombs[i].active) {
            bool validPosition = false;
            // Repeat until a valid position is found
            while (!validPosition) {
                // Randomly spawn bomb in positions within the grid
                bombs[i].x = GetRandomValue(0, GRID_WIDTH - 1);
                bombs[i].y = GetRandomValue(0, GRID_HEIGHT - 1);
                validPosition = true;

                // Check not overlapping snake
                for (int j = 0; j < snakeLength; j++) {
                    if (snake[j].x == bombs[i].x && snake[j].y == bombs[i].y) {
                        validPosition = false; // Overlaps with snake, try again
                        break;
                    }
                }

                // Check not overlapping food
                if (bombs[i].x == food.x && bombs[i].y == food.y) {
                    validPosition = false; // Overlaps with food, try again
                }

                // Check not overlapping other active bombs
                for (int j = 0; j < MAX_BOMBS; j++) {
                    if (i != j && bombs[j].active && bombs[i].x == bombs[j].x && bombs[i].y == bombs[j].y) {
                        validPosition = false; // Overlaps with another bomb, try again
                        break;
                    }
                }
            }
            // If a valid position is found, activate the bomb
            bombs[i].active = true;
            bombs[i].timer = 0.0f;
            bombs[i].lifetime = BOMB_LIFETIME;
            return; // Exit function
        }
    }
}

// Spawn crumbly particles at a given position and color
void spawnCrumblyParticles(float x, float y, Color color, int count) { // Use parameter for specific information
    // Loop through the particles array to find inactive particles
    for (int i = 0; i < MAX_PARTICLES && count > 0; i++) {
        if (!particles[i].active) {         // Use inactive particle slots
            particles[i].x = x;             // Starting X position
            particles[i].y = y;             // Starting Y position

            // 360 degrees making circle spread pattern
            float angle = GetRandomValue(0, 359) * (PI / 180.0f);

            // Set particle random speed (for a wider spread)
            float speed = GetRandomValue(20, 40) / 100.0f; 

            // Set particle velocity based on angle and speed
            particles[i].vx = cosf(angle) * speed;
            particles[i].vy = sinf(angle) * speed;

            particles[i].life = 1.0f;       // Particle life duration
            particles[i].color = color;     // Particle color following food color
            particles[i].active = true;     // Activate the particle
            count--;                        // Decrease count of particles to spawn
        }
    }
}

// Check for self-collision and border collision
// Checks if the snake's head collides with part of the body
bool checkSelfCollision() {
    // Start from 1 because the head is at index 0
    for (int i = 1; i < snakeLength; i++) {
        // Compare head position wich each body segment
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
            return true; // Collision detected
        }
    }
    return false; // No collision detected
}

// Check collisiong with border
bool checkBorderCollision() {
    // Check if the snake's head is outside the grid boundaries
    return (snake[0].x < 0 || snake[0].x >= GRID_WIDTH ||
            snake[0].y < 0 || snake[0].y >= GRID_HEIGHT);
}

// Reset the game state
void resetGame() {
    applyDifficultySettings();          // Apply difficulty settings
    snakeLength = 1;                    // Reset snake length to 1
    currentDirection = DIR_UP;          // Reset snake direction to up
    gameState = GAME_RUNNING;           // Set game state to running
    timePlayed = 0.0f;                  // Reset time played

    // Reset bomb and level
    for (int i = 0; i < MAX_BOMBS; i++) {
        bombs[i].active = false;
        bombs[i].timer = 0.0f;
    }
    currentLevel = 1;

    // Position the snake in the center of the screen and spawn food
    for (int i = 0; i < snakeLength; i++) {
        snake[i].x = GRID_WIDTH / 2 - i;
        snake[i].y = GRID_HEIGHT / 2;
    }
    spawnFood();
}

// Updates all game logic, including input, timers, collisions, and spawning
void update() {
    globalTime += GetFrameTime();
    
    // Main menu state
    if (gameState == GAME_MENU) {
        if (!IsMusicStreamPlaying(musicMenu)) {
        PlayMusicStream(musicMenu);
    }
        UpdateMusicStream(musicMenu);

        // Change difficulty using LEFT and RIGHT
        if (IsKeyPressed(KEY_LEFT)) {
            currentDifficulty = (currentDifficulty == 0) ? 2 : currentDifficulty - 1; // If difficulty is 0, set to 2, otherwise decrement by 1
            PlaySound(sndMenu); 
        } else if (IsKeyPressed(KEY_RIGHT)) {
            currentDifficulty = (currentDifficulty + 1) % 3; // Wrap around after last difficulty
            PlaySound(sndMenu); 
        }

        // Enter to start the game
        if (IsKeyPressed(KEY_ENTER)) {
            StopMusicStream(musicMenu); 
            resetGame();

            gameState = GAME_RUNNING;
            PlaySound(sndEnter); 
        }
        return;
    }

    // Game running state
    if (gameState == GAME_RUNNING) {
        timePlayed += GetFrameTime();
        if (!IsMusicStreamPlaying(musicRunning)) { 
            PlayMusicStream(musicRunning);
            SetMusicVolume(musicRunning, 0.5f);     // Set game volume
        }
        UpdateMusicStream(musicRunning);
        StopMusicStream(musicMenu);                 // Stop menu music if playing
    }

    // Update food timer and respawn if expired
    foodTimer += GetFrameTime();
    if (foodTimer >= foodLifetime) {                // If food lifetime has expired
        spawnFood();                                // Despawn and spawn new one
    }

    // Bomb logic: update timers and handle expiration
    for (int i = 0; i < MAX_BOMBS; i++) {
        if (bombs[i].active) {
            bombs[i].timer += GetFrameTime();
            if (bombs[i].timer >= bombs[i].lifetime) {
                bombs[i].active = false;
                PlaySound(sndBomb);     

                // Spawn crumbly particles at the bomb position
                spawnCrumblyParticles(
                bombs[i].x * GRID_SIZE + GRID_SIZE / 2,
                bombs[i].y * GRID_SIZE + GRID_SIZE / 2,
                ORANGE,80); 
            }
        }
    }

    // Spawn new bombs periodically
    bombSpawnTimer += GetFrameTime();
    if (bombSpawnTimer >= bombSpawnInterval) {
        bombsToSpawn = BOMBS_PER_SPAWN + (currentLevel - 1);        // Increase the number of bombs spawned as time/level increases
        if (bombsToSpawn > MAX_BOMBS) bombsToSpawn = MAX_BOMBS;

        for (int i = 0; i < bombsToSpawn; i++) {
            spawnBomb();
        }
        bombSpawnTimer = 0.0f;
    }

    // Press R to restart the game
    if (gameState == GAME_OVER) {
        if (IsKeyPressed(KEY_R)) {
            gameState = GAME_MENU;
            PlaySound(sndMenu); 
        }
        return;
    }

    // Control the snake with arrow keys or WASD
    if (IsKeyPressed(KEY_UP) && currentDirection != DIR_DOWN) currentDirection = DIR_UP;
    else if (IsKeyPressed(KEY_DOWN) && currentDirection != DIR_UP) currentDirection = DIR_DOWN;
    else if (IsKeyPressed(KEY_LEFT) && currentDirection != DIR_RIGHT) currentDirection = DIR_LEFT;
    else if (IsKeyPressed(KEY_RIGHT) && currentDirection != DIR_LEFT) currentDirection = DIR_RIGHT;

    if (IsKeyPressed(KEY_W) && currentDirection != DIR_DOWN) currentDirection = DIR_UP;
    else if (IsKeyPressed(KEY_S) && currentDirection != DIR_UP) currentDirection = DIR_DOWN;
    else if (IsKeyPressed(KEY_A) && currentDirection != DIR_RIGHT) currentDirection = DIR_LEFT;
    else if (IsKeyPressed(KEY_D) && currentDirection != DIR_LEFT) currentDirection = DIR_RIGHT;

    // Snake movement logic
    moveTimer += GetFrameTime();
    if (moveTimer >= moveDelay) {
        moveTimer = 0;

        // Move the snake segments from tail to head
        for (int i = snakeLength - 1; i > 0; i--) {
            snake[i] = snake[i - 1];
        }

        // Move the snake head in the current direction
        switch (currentDirection) {
            case DIR_UP: snake[0].y--; break;
            case DIR_DOWN: snake[0].y++; break;
            case DIR_LEFT: snake[0].x--; break;
            case DIR_RIGHT: snake[0].x++; break;
        }

        // Bomb collision check 
        for (int i = 0; i < MAX_BOMBS; i++) {
            if (bombs[i].active && snake[0].x == bombs[i].x && snake[0].y == bombs[i].y) { // If snake collides with an active bomb = game over
                gameState = GAME_OVER;
                PlaySound(sndGameOver);
                PlaySound(sndBomb);
                bombs[i].active = false; // Deactivate the bomb when snake collides with it

                if (snakeLength - 1 > bestScore) bestScore = snakeLength - 1;   // Check high score
                if (timePlayed > bestTime) bestTime = timePlayed;               // Check best time
                SaveBestStats();
                return;
            }
        }

        // Border or self collision
        if (checkBorderCollision() || checkSelfCollision()) {
            gameState = GAME_OVER;
            PlaySound(sndGameOver);

            // Clamp snake head position to stay within the grid when die
            if (snake[0].x < 0) snake[0].x = 0;
            if (snake[0].x >= GRID_WIDTH) snake[0].x = GRID_WIDTH - 1;
            if (snake[0].y < 0) snake[0].y = 0;
            if (snake[0].y >= GRID_HEIGHT) snake[0].y = GRID_HEIGHT - 1;

            if (snakeLength - 1 > bestScore) bestScore = snakeLength - 1;       // Check high score
            if (timePlayed > bestTime) bestTime = timePlayed;                   // Check best time
            SaveBestStats();
            return;
        }

        // Only runs if no collision happened!
        // Food collision check
        if (snake[0].x == food.x && snake[0].y == food.y) {
            PlaySound(sndEat);
            
            // Spawn crumbly particles when the food got eaten
            spawnCrumblyParticles(
                snake[0].x * GRID_SIZE + GRID_SIZE / 2,
                snake[0].y * GRID_SIZE + GRID_SIZE / 2,
                food.color, 18); // 18 crumbly particles

            // Grow the snake by the score value of the food
            for (int i = 0; i < food.scoreValue && snakeLength < MAX_SNAKE_LENGTH; i++) {
                snake[snakeLength] = snake[snakeLength - 1];   // New segment takes the position of the last segment
                snakeLength++;
            }
            spawnFood();

            // Check if the snake has reached the score needed to level up
            int score = snakeLength - 1;
            if (score >= currentLevel * pointToNextLevel) {
                currentLevel++;
                moveDelay *= 0.85f; // Speed up the snake by reducing the move delay every level up by 15%
                if (moveDelay < 0.05f) moveDelay = 0.05f;
                PlaySound(sndLevelUp);
            }       
        }
    }
    
    // Update all particles 
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            particles[i].x += particles[i].vx;              // Update X particle position 
            particles[i].y += particles[i].vy;              // Update Y particle position
            particles[i].life -= GetFrameTime();            // Decrease particle life
            if (particles[i].life <= 0) particles[i].active = false;
        }
    }
}

// Draws / Render the game elements including the snake, food, bombs, and particles
void draw() {
    // MAIN MENU
    if (gameState == GAME_MENU) {
        ClearBackground((Color){6, 8, 18, 255});

        // Define char pointers for text
        const char* title = "SNAKEY SNAKEY";
        const char* instruction = "Press ENTER to Start";
        const char* bestScoreText = TextFormat("Best Score: %d", bestScore);
        const char* bestTimeText = TextFormat("Longest Time: %02d:%02d", (int)bestTime / 60, (int)bestTime % 60);

        // Define integers for text width
        int instWidth = MeasureText(instruction, 30);
        int scoreWidth = MeasureText(bestScoreText, 25);
        int timeWidth = MeasureText(bestTimeText, 25);

        // Animated snake on menu
        int snakeMenuLength = 8;                            // Length of the snake in the menu
        float snakeMenuBaseX = SCREEN_WIDTH / 2 - 120;      // Base X position for the snake
        float snakeMenuBaseY = SCREEN_HEIGHT / 2 - 130;     // Base Y position for the snake
        float snakeMenuSpacing = 32;                        // Spacing between snake segments in px

        // Draw body segments first 
        for (int i = 1; i < snakeMenuLength; i++) {
            float t = globalTime * 2.0f + i * 0.5f;                                 // Time offset for each segment (2.0f speed) (0.5f animation)
            float offsetY = 18 * sinf(t);                                           // Each segment goes vertically +-18 pixels and smoothly with sinf()
            int sx = (int)(snakeMenuBaseX + i * snakeMenuSpacing);                  // X position based on segment index and spacing (sx screen coordinate)
            int sy = (int)(snakeMenuBaseY + offsetY);                               // Y position based on base Y and animation offset (sy screen coordinate)
            DrawTextureEx(texSnakeBody, (Vector2){sx, sy}, 0.0f, 0.8f, WHITE);      // Draw snake body segment at 80% scale
        }

        // Draw head last so it's on top
        float t = globalTime * 2.0f;                                                // Time for head animation (2.0f speed)
        float offsetY = 18 * sinf(t);                                               // Head goes vertically +-18 pixels and smoothly with sinf()    
        int sx = (int)(snakeMenuBaseX);                                             // X position for the snake head (base X)                           
        int sy = (int)(snakeMenuBaseY + offsetY);                                   // Y position for the snake head (base Y + offsetY)
        DrawTextureEx(texSnakeHead, (Vector2){sx, sy}, 0.0f, 0.8f, WHITE);          // Draw snake head at 80% scale
        
        // Draw text in the center of the screen
        // Pulsing effect for the title
        float pulse = 1.0f + 0.08f * sinf(globalTime * 3.0f);                       // Adjust 0.08f for pulse strength, 3.0f for speed
        int pulsingFontSize = (int)(60 * pulse);
        int pulsingTitleWidth = MeasureText(title, pulsingFontSize);                
        DrawText(title, SCREEN_WIDTH / 2 - pulsingTitleWidth / 2, SCREEN_HEIGHT / 2 - 60, pulsingFontSize, GREEN);
        DrawText(instruction, SCREEN_WIDTH / 2 - instWidth / 2, SCREEN_HEIGHT / 2 + 10, 30, WHITE);
        DrawText(bestScoreText, SCREEN_WIDTH / 2 - scoreWidth / 2, SCREEN_HEIGHT / 2 + 60, 25, YELLOW);
        DrawText(bestTimeText, SCREEN_WIDTH / 2 - timeWidth / 2, SCREEN_HEIGHT / 2 + 100, 25, ORANGE);

        const char* difficultyLabel = "Difficulty: ";
        const char* difficultyText =
            (currentDifficulty == DIFFICULTY_EASY) ? "EASY" :
            (currentDifficulty == DIFFICULTY_MEDIUM) ? "MEDIUM" : "HARD";
        const char* difficultyInstruction = "Use LEFT/RIGHT to change difficulty";
        const char* escInstruction = "Press ESC to Exit"; 

        int diffLabelWidth = MeasureText(difficultyLabel, 25);      // Measuretext to perfectly center the text
        int diffTextWidth = MeasureText(difficultyText, 25);
        int diffInstWidth = MeasureText(difficultyInstruction, 25);
        int escInstWidth = MeasureText(escInstruction, 25); 
        
        DrawText(difficultyLabel, SCREEN_WIDTH / 2 - (diffLabelWidth + diffTextWidth) / 2, SCREEN_HEIGHT / 2 + 140, 25, LIGHTGRAY);
        DrawText(difficultyText, SCREEN_WIDTH / 2 - (diffLabelWidth + diffTextWidth) / 2 + diffLabelWidth, SCREEN_HEIGHT / 2 + 140, 25, SKYBLUE);       // +difflabelwidth to position difficulty text next to label
        DrawText(difficultyInstruction, SCREEN_WIDTH / 2 - diffInstWidth / 2, SCREEN_HEIGHT / 2 + 180, 25, LIGHTGRAY);
        DrawText(escInstruction, SCREEN_WIDTH / 2 - escInstWidth / 2, SCREEN_HEIGHT / 2 + 220, 25, RED); 
        return;
    }

    // GAME RUNNING
    ClearBackground(DARKGREEN);

    // Draw grass texture as background
    for (int y = 0; y < SCREEN_HEIGHT; y += texGrass.height) {
        for (int x = 0; x < SCREEN_WIDTH; x += texGrass.width) {
            DrawTexture(texGrass, x, y, WHITE);
        }
    }

    // Draw Fern
    for (int i = 0; i < FERN_COUNT; i++) {
        int fx = fernPositions[i][0] * GRID_SIZE;       //[0] is x position, [1] is y position
        int fy = fernPositions[i][1] * GRID_SIZE;
        DrawTexture(texFern, fx, fy, WHITE);
    }

    // Draw Flower
    for (int i = 0; i < FLOWER_COUNT; i++) {
        int fx = flowerPositions[i][0] * GRID_SIZE;
        int fy = flowerPositions[i][1] * GRID_SIZE;
        DrawTexture(texFlower, fx, fy, WHITE);
    }
    // Draw Flower2
    for (int i = 0; i < FLOWER2_COUNT; i++) {
        int fx = flower2Positions[i][0] * GRID_SIZE;
        int fy = flower2Positions[i][1] * GRID_SIZE;
        DrawTexture(texFlower2, fx, fy, WHITE);
    }

    // Draw Mushroom
    for (int i = 0; i < MUSHROOM_COUNT; i++) {
        int fx = mushroomPositions[i][0] * GRID_SIZE;
        int fy = mushroomPositions[i][1] * GRID_SIZE;
        DrawTexture(texMushroom, fx, fy, WHITE);
    }
   
    // Draw grid lines
    // Draw vertical grid lines across the screen 
    for (int x = 0; x <= SCREEN_WIDTH; x += GRID_SIZE) 
        DrawLine(x, 0, x, SCREEN_HEIGHT, (Color){102, 102, 0, 120});
    
    // Draw horizontal grid lines across the screen
    for (int y = 0; y <= SCREEN_HEIGHT; y += GRID_SIZE)
        DrawLine(0, y, SCREEN_WIDTH, y, (Color){102, 102, 0, 120}); 

    // Draw vertical gradient overlay (from transparent at top to semi-transparent at bottom)
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        float t = (float)y / SCREEN_HEIGHT;
        Color overlay = (Color){
            0, 0, 0,
            (unsigned char)(80 * t)                     // Unsigned Char (RGB) = Alpha goes from 0 (Top) to 80 (Bottom)
        };
        DrawRectangle(0, y, SCREEN_WIDTH, 1, overlay);  // Draw a 1px tall rectangle for each row
    }

    // Draw snake body first
    for (int i = 1; i < snakeLength; i++) {
        int px = snake[i].x * GRID_SIZE;
        int py = snake[i].y * GRID_SIZE;
        Texture2D tex = texSnakeBody;
        if (gameState == GAME_OVER) {
            DrawTextureEx(tex, (Vector2){px, py}, 0.0f, (float)GRID_SIZE / tex.width, WHITE);
        } else {
            float pulse = 1.0f + 0.05f * sinf(globalTime * 8 + i * 0.5f);               // Pulse effect for the snake body, 8 for speed, 0.5f for offset
            float drawSize = GRID_SIZE * pulse;                                         // Calculate the size of the body segment based on pulse                
            float offset = (GRID_SIZE - drawSize) / 2;
            float wiggle = 3.0f * sinf(globalTime * 8 + i);                             // Wiggle effect for the snake body, 8 for speed, i for offset
            DrawTextureEx(tex, (Vector2){px + offset + wiggle, py + offset}, 0.0f, drawSize / tex.width, WHITE);
        }
    }

    // Draw snake head last so it appears on top of the body
    int px = snake[0].x * GRID_SIZE;                                                // Pixel coordinates X for the snake head
    int py = snake[0].y * GRID_SIZE;                                                // Pixel coordinates Y for the snake head
    Texture2D tex = (gameState == GAME_OVER) ? texSnakeHeadDead : texSnakeHead;     // Use dead head texture if game is over
    DrawTextureEx(tex, (Vector2){px, py}, 0.0f, (float)GRID_SIZE / tex.width, WHITE);
    
    // DRAW FOOD AND PULSE ANIMATION
    float scale = 1.0f + 0.1f * sinf(globalTime * 5); // Pulses between 0.9 and 1.1
    float size = (GRID_SIZE - GRID_SPACE) * scale;
    float offset = ((GRID_SIZE - GRID_SPACE) - size) / 2;

    int fx = food.x * GRID_SIZE + GRID_SPACE / 2 + offset;  // Center food in the grid cell
    int fy = food.y * GRID_SIZE + GRID_SPACE / 2 + offset;  // Center food in the grid cell 

    // Select the correct texture based on food type
    Texture2D foodTex;
    switch (food.type) {
        case 0: foodTex = texFoodNormal; break;
        case 1: foodTex = texFoodEpic; break;
        case 2: foodTex = texFoodSuper; break;
        default: foodTex = texFoodNormal; break;
    }

    // Flashing effect if food is about to expire
    float timeLeft = foodLifetime - foodTimer;
    Color tint = WHITE;
    if (timeLeft < 1.5f) {
        float flashSpeed = 10.0f + (2.0f - timeLeft) * 30.0f;           // Speed up the flash as time runs out
        int flash = ((int)(globalTime * flashSpeed)) % 2;               // Blinking effect
        tint = flash ? WHITE : (Color){255,255,255,128};                // semi-transparent flash
    }

    // Draw the food texture with pulse and flash
    DrawTextureEx(foodTex, (Vector2){fx, fy}, 0.0f, size / foodTex.width, tint);

    // Draw bombs and pulse animation
    for (int i = 0; i < MAX_BOMBS; i++) {
        if (bombs[i].active) {
            float scale = 1.0f + 0.15f * sinf(globalTime * 6);              // Pulses between 0.85 and 1.15 , 6 for speed
            float size = (GRID_SIZE - GRID_SPACE) * scale;
            float offset = ((GRID_SIZE - GRID_SPACE) - size) / 2;

            int bx = bombs[i].x * GRID_SIZE + GRID_SPACE / 2 + offset;
            int by = bombs[i].y * GRID_SIZE + GRID_SPACE / 2 + offset;

            // Flashing effect if bomb is about to expire
            float timeLeft = bombs[i].lifetime - bombs[i].timer;
            Color bombTint = WHITE;
            if (timeLeft < 2.0f && ((int)(globalTime * 10) % 2 == 0)) {     // Less than 2 seconds left, flash every 0.1 seconds
                bombTint = ORANGE;
            }
            DrawTextureEx(texBomb, (Vector2){bx, by}, 0.0f, size / texBomb.width, bombTint);
        }
    }

    // Draw timer 
    DrawText(TextFormat("Time: %02d:%02d", (int)timePlayed / 60, (int)timePlayed % 60), 10, 30, 20, WHITE);

    // Draw level
    DrawText(TextFormat("Level: %d", currentLevel), 10, 50, 20, WHITE);
 
    // GAME OVER
    if (gameState == GAME_OVER) {

        // Define char pointers for text  
        const char* gameOverText = "GAME OVER";
        const char* restartText = "Press R to Return to Menu";
        const char* scoreText = TextFormat("SCORE: %d", snakeLength - 1);
        const char* timeText = TextFormat("Time: %02d:%02d", (int)timePlayed / 60, (int)timePlayed % 60);
        
        // Define integers for text width
        int gameOverWidth = MeasureText(gameOverText, 60);
        int restartWidth = MeasureText(restartText, 30);
        int scoreWidth = MeasureText(scoreText, 40);
        int timeTextWidth = MeasureText(timeText, 30);

        // Draw text in the center of the screen
        DrawText(gameOverText, SCREEN_WIDTH/2 - gameOverWidth/2, SCREEN_HEIGHT/2 - 60, 60, RED);
        DrawText(scoreText, SCREEN_WIDTH/2 - scoreWidth/2, SCREEN_HEIGHT/2, 40, WHITE);
        DrawText(restartText, SCREEN_WIDTH/2 - restartWidth/2, SCREEN_HEIGHT/2 + 50, 30, WHITE);
        DrawText(timeText, SCREEN_WIDTH / 2 - timeTextWidth / 2, SCREEN_HEIGHT / 2 + 90, 30, WHITE);
    } else {
        DrawText(TextFormat("Score: %d", snakeLength - 1), 10, 10, 20, WHITE);
    }

    // Draw particles (put this before the final closing brace of draw())
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            Color color = particles[i].color;
            color.a = (unsigned char)(255 * (particles[i].life / 1.0f));      // Fade out particles based on their life (Unsigned Char = Alpha)
            DrawCircle((int)particles[i].x, (int)particles[i].y, 2, color);
        }
    }
    // Draw FPS
    DrawText(TextFormat("FPS: %d", GetFPS()), 10, SCREEN_HEIGHT - 30, 20, WHITE);
}

int main() {
    // Initialize the window and set target FPS
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Snakey Snakey");
    SetTargetFPS(144);
    LoadBestStats(); // Load best score and time

    texSnakeHead = LoadTexture("assets/snakehead.png");
    texSnakeBody = LoadTexture("assets/snakebody.png");
    texFoodNormal = LoadTexture("assets/apple1.png");
    texFoodEpic = LoadTexture("assets/apple2.png");
    texFoodSuper = LoadTexture("assets/easter.png");
    texBomb = LoadTexture("assets/bomb.png");
    texGrass = LoadTexture("assets/grass.jpg");
    texMushroom = LoadTexture("assets/mushroom.png");
    texFlower = LoadTexture("assets/flower.png");
    texFlower2 = LoadTexture("assets/flower2.png");
    texFern = LoadTexture("assets/fern.png");
    texSnakeHeadDead = LoadTexture("assets/snakedead.png");
    
    InitAudioDevice(); // Initialize audio device
    sndEat = LoadSound("assets/eat.wav");
    sndGameOver = LoadSound("assets/gameover.wav");
    sndMenu = LoadSound("assets/menu.wav");
    sndLevelUp= LoadSound("assets/level.wav");
    sndEnter = LoadSound("assets/enter.wav");
    sndBomb = LoadSound("assets/bomb.wav");
    musicMenu = LoadMusicStream("assets/musicmenu.wav");
    musicRunning = LoadMusicStream("assets/gamerunning.wav");

    // Initialize the snake in the center of the screen
    for (int i = 0; i < snakeLength; i++) {
        snake[i].x = GRID_WIDTH / 2 - i;
        snake[i].y = GRID_HEIGHT / 2;
    }

    // Initialize the food
    spawnFood();

    while (!WindowShouldClose()) {
        BeginDrawing();
        update();
        draw();
        EndDrawing();
    }

    // Unload textures and sounds
    UnloadTexture(texSnakeHead);
    UnloadTexture(texSnakeBody);
    UnloadTexture(texFoodNormal);
    UnloadTexture(texFoodEpic);
    UnloadTexture(texFoodSuper);
    UnloadTexture(texBomb);
    UnloadTexture(texGrass);
    UnloadTexture(texMushroom);
    UnloadTexture(texFlower);
    UnloadTexture(texFlower2);
    UnloadTexture(texFern);
    UnloadTexture(texSnakeHeadDead);

    UnloadSound(sndEat);
    UnloadSound(sndGameOver);
    UnloadSound(sndMenu);
    UnloadSound(sndLevelUp);
    UnloadSound(sndEnter);
    UnloadSound(sndBomb);
    UnloadMusicStream(musicMenu);
    UnloadMusicStream(musicRunning);
  
    CloseAudioDevice(); // Close audio device

    CloseWindow();
    return 0;
}
