#include "raylib.h"
#include <stdlib.h>

const int FPS = 60;

const int screenWidth = 1280;
const int screenHeight = 720;

struct Ball
{
    Vector2 Centre;
    float Radius;
    Color Color;
    Vector2 velocity;
};

struct Paddle
{
    Rectangle Rect;
    Color Color;
};

struct GameState
{
    bool isGameOver;
    bool isPaused;
    int score;
    int timer;
};

const int numBlocksX = 18;
const int numBlocksY = 10;

void screenTimer(struct GameState *gameState)
{
    if (gameState->timer > FPS * 4)
        DrawText("4", screenWidth / 2, screenHeight / 2, 100, ORANGE);
    else if (gameState->timer > FPS * 3)
        DrawText("3", screenWidth / 2, screenHeight / 2, 100, ORANGE);
    else if (gameState->timer > FPS * 2)
        DrawText("2", screenWidth / 2, screenHeight / 2, 100, ORANGE);
    else if (gameState->timer > FPS)
        DrawText("1", screenWidth / 2, screenHeight / 2, 100, ORANGE);
    else
        gameState->isPaused = true; // Set isPaused to true when the timer reaches zero
    gameState->timer--; // Decrement timer
}

bool IsAnyKeyPressed()
{
    for (int key = 0; key < 512; key++)
    {
        if (IsKeyPressed(key))
        {
            return true;
        }
    }
    return false;
}

void setBallVelocity(struct Ball *ball)
{
    ball->velocity.x = rand() % 5 + 2;
    ball->velocity.y = rand() % 5 - 7;
}

void ResetBallPosition(struct Ball *ball)
{
    ball->Centre.x = screenWidth / 2;
    ball->Centre.y = screenHeight / 2 + 150;
}

void checkBallPlayareaCollision(struct Ball *ball, struct Paddle *paddle, struct GameState *gameState, Rectangle playArea)
{
    // stops the ball from going off screen
    if (ball->Radius + ball->Centre.y > screenHeight && !gameState->isPaused)
    {
        (gameState->isGameOver) = true;
        ball->velocity.y = 0;
        ball->velocity.x = 0;
        ball->Centre.y = screenHeight - ball->Radius;
    }
    else if (ball->Centre.x + ball->Radius > playArea.width + 50 && !gameState->isPaused)
    {
        ball->velocity.x *= -1;
        // Ball.Centre.x = playArea.width - Ball.Radius;
    }
    else if (ball->Centre.x + ball->Radius < playArea.x + 20 && !gameState->isPaused)
    {
        ball->velocity.x *= -1;
        // Ball.Centre.x = Ball.Radius + playArea.x;
    }
    else if (ball->Centre.y + ball->Radius < playArea.y + 20 && !gameState->isPaused)
    {
        ball->velocity.y *= -1;
        // Ball.Centre.y = Ball.Radius + playArea.y;
    }

    if (CheckCollisionCircleRec(ball->Centre, ball->Radius, paddle->Rect))
    {
        ball->velocity.y *= -1;
        ball->Centre.y = paddle->Rect.y - ball->Radius;
    }
}

void updateBallPosition(struct Ball *ball, struct GameState *gameState)
{
    if (!gameState->isPaused)
    {
        ball->Centre.y += ball->velocity.y;
        ball->Centre.x += ball->velocity.x;
    }
}

void updatePaddle(struct Paddle *paddle, Rectangle playArea, struct GameState *gameState)
{
    // moves the paddle
    if (!gameState->isPaused && !gameState->isGameOver)
    {
        if (IsKeyDown(KEY_LEFT) && (paddle->Rect.x > playArea.x))
            paddle->Rect.x -= 10;
        if (IsKeyDown(KEY_RIGHT) && (paddle->Rect.x + paddle->Rect.width < playArea.width + 50))
            paddle->Rect.x += 10;
    }
}

void updateAndRegisterBallBlockCollision(struct Ball *ball, bool blocksHit[numBlocksX][numBlocksY], struct GameState *gameState)
{
    for (int i = 0; i < numBlocksX; i++)
    {
        for (int j = 0; j < numBlocksY; j++)
        {
            Rectangle block = {100 + i * 60, 100 + j * 30, 50, 20};

            if (!blocksHit[i][j] && CheckCollisionCircleRec(ball->Centre, ball->Radius, block))
            {
                (gameState->score) += 60;
                ball->velocity.y *= -1;
                blocksHit[i][j] = true;
            }
        }
    }
}

int main(void)
{
    InitWindow(screenWidth, screenHeight, "Breakout");

    Rectangle playArea = {50, 50, screenWidth - 100, screenHeight + 20};

    bool blocksHit[numBlocksX][numBlocksY];

    struct Ball ball;
    ball.Centre.x = screenWidth / 2;
    ball.Centre.y = screenHeight / 2 + 150;
    ball.Radius = 9.0;
    ball.Color = RED;
    setBallVelocity(&ball);

    struct Paddle paddle;
    paddle.Rect.x = ball.Centre.x - 20;
    paddle.Rect.y = 570;
    paddle.Rect.width = 80;
    paddle.Rect.height = 10;
    paddle.Color = BLACK;

    struct GameState gameState;
    gameState.isGameOver = false;
    gameState.isPaused = false;
    gameState.score = 0;
    gameState.timer = FPS * 5;

    SetTargetFPS(FPS);

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // UPDATES
        updateBallPosition(&ball, &gameState);

        /* CONDITIONS */
        if (gameState.isGameOver)
        {
            gameState.score = 0;
            if (IsAnyKeyPressed())
            {
                gameState.isGameOver = false;
                ResetBallPosition(&ball);
                setBallVelocity(&ball);
                gameState.isPaused = true;
                screenTimer(&gameState);
            }
        }

        checkBallPlayareaCollision(&ball, &paddle, &gameState, playArea);
        updatePaddle(&paddle, playArea, &gameState);
        updateAndRegisterBallBlockCollision(&ball, blocksHit, &gameState);

        // DRAWINGS
        BeginDrawing();

        if (!gameState.isGameOver)
            ClearBackground(RAYWHITE);

        DrawRectangleLinesEx(playArea, 5, DARKBROWN);
        DrawCircle(ball.Centre.x, ball.Centre.y, ball.Radius, ball.Color);
        DrawRectangleRec(paddle.Rect, paddle.Color);
        DrawText(TextFormat("Score : %d", gameState.score), screenWidth / 2, 10, 20, BLACK);

        for (int i = 0; i < numBlocksX; i++)
        {
            for (int j = 0; j < numBlocksY; j++)
            {
                // BlockStartingX + (i/j) * Block width
                Rectangle block = {100 + i * 60, 100 + j * 30, 50, 20};

                if (!blocksHit[i][j])
                {
                    DrawRectangleRec(block, BLACK);
                }
            }
        }

        if (gameState.isGameOver)
        {
            ClearBackground(LIGHTGRAY);
            DrawText("YOU LOST! ", screenWidth / 2 - 300, paddle.Rect.y, 100, RED);
            DrawText("Press any key to restart! ", screenWidth / 2 - 330, screenHeight / 2 + 100, 50, WHITE);
        }

        EndDrawing();

        gameState.score++;
    }

    CloseWindow(); // Close window and OpenGL context

    return 0;
}