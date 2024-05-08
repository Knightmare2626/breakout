#include "raylib.h"
#include <stdio.h>

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
    int score;
    int timer;
};

const int numBlocksX = 18;
const int numBlocksY = 10;

void saveScore(struct GameState *gameState)
{
    FILE *g_score_ptr;
    g_score_ptr = fopen("scores.dat", "a");

    if (g_score_ptr == NULL)
    {
        printf("File couldn't be opened successfully :(\n");
        return;
    }

    if (gameState->score != 0)
        fprintf(g_score_ptr, "%d\n", gameState->score);

    fclose(g_score_ptr);
    gameState->score = 0;
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
    if (ball->Radius + ball->Centre.y > screenHeight)
    {
        (gameState->isGameOver) = true;
        ball->velocity.y = 0;
        ball->velocity.x = 0;
        ball->Centre.y = screenHeight - ball->Radius;
    }

    else if (ball->Centre.x + ball->Radius > playArea.width + 50)
        ball->velocity.x *= -1;

    else if (ball->Centre.x + ball->Radius < playArea.x + 20)
        ball->velocity.x *= -1;

    else if (ball->Centre.y + ball->Radius < playArea.y + 20)
        ball->velocity.y *= -1;

    if (CheckCollisionCircleRec(ball->Centre, ball->Radius, paddle->Rect))
    {
        ball->velocity.y *= -1;
        ball->Centre.y = paddle->Rect.y - ball->Radius;
    }
}

void updateBallPosition(struct Ball *ball, struct GameState *gameState)
{
    ball->Centre.y += ball->velocity.y;
    ball->Centre.x += ball->velocity.x;
}

void updatePaddle(struct Paddle *paddle, Rectangle playArea, struct GameState *gameState)
{
    // moves the paddle
    if (!gameState->isGameOver)
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
            saveScore(&gameState);
            if (IsAnyKeyPressed())
            {
                ResetBallPosition(&ball);
                gameState.isGameOver = false;
                setBallVelocity(&ball);
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

        // displays and centres "Score"
        char *ScoreText = TextFormat("Score: %d", gameState.score);
        Vector2 textSize = MeasureTextEx(GetFontDefault(), ScoreText, 30, 1);
        int posX = (screenWidth - textSize.x) / 2;
        int posY = 20;
        DrawText(ScoreText, posX, posY, 30, BLACK);

        // draws and updates the rectangles
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
        else
            gameState.score++;

        EndDrawing();
    }

    CloseWindow(); // Close window and OpenGL context

    return 0;
}