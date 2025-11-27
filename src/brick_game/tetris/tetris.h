#ifndef TETRIS_H
#define TETRIS_H

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#endif

#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FIELD_HEIGHT 20  // Высота поля
#define FIELD_WIDTH 10   // Ширина поля
#define BLOCK 4          // Размер матрицы для фигур

/**
 * @brief Перечисление действий пользователя.
 */
typedef enum {
  Start,      // \n
  Pause,      // p
  Terminate,  // q
  Left,
  Right,
  Up,  // не используется
  Down,
  Action  // ' '
} UserAction_t;

/**
 * @brief Перечисление, описывающее возможные состояния игры.
 */
typedef enum {
  Start_t,
  Spawn,
  Moving,
  Shifting,
  Pause_t,
  Attaching,
  Terminate_t,
  GameOver
} Status_game;

/**
 * @brief Структура с текущим состоянием игры.
 */
typedef struct GameInfo_t {
  int **field;
  int **next;
  int score;
  int high_score;
  int level;
  int speed;
  int pause;
} GameInfo_t;

/**
 * @brief Структура, описывающая игровые фигуры.
 */
typedef struct {
  int shape[BLOCK][BLOCK];  // Матрица формы фигуры:
  int posX;  // Позиция фигуры по оси X на игровом поле
  int posY;  // Позиция фигуры по оси Y на игровом поле
  int type;      // Тип фигуры (для цвета)
  int rotation;  // Текущий поворот
} Piece;

/**
 * @brief Структура, содержащая все вспомогательные параметры игры.
 */
typedef struct Params_t {
  GameInfo_t *game_info;  // Копия структуры с текущим состоянием
  struct timespec
      drop_time;  // Структура под отслеживания периода падения фигур
  double drop_interval;  // Переменная для хранения разницы во времени
  bool start;  // Флаг на начало игрового состояния
  Piece currentPiece;   // Структура текущей фигуры
  Piece nextPiece;      // Структура следующей фигуры
  int **fieldForFront;  // Матрица с полем которое учавствует в передаче
                        // состояния из логики
} Params_t;

Status_game *initGameState(void);
GameInfo_t *initGameInfo(void);
Params_t *initGameParam(void);
void initGame(void);

int **initMatrix(int row, int col);
void freeMatrix(int **field, int row);

void userInput(UserAction_t action, bool hold);
GameInfo_t updateCurrentState();
int getTime(Params_t *param, double *drop_interval,
            struct timespec *cur_time);  //
GameInfo_t createGameInfo(const Params_t param, const int return_check);
void overlayCurrentPiece(const Params_t *param);

int spawnNewPiece(Params_t *param, bool isInit);
void copyPieceShape(int dest[BLOCK][BLOCK], const int src[BLOCK][BLOCK]);
int canPlacePiece(Params_t *param);

void fixFigure(Params_t *param);
int moveFigureDown(Params_t *param);
int moveFigureLeft(Params_t *param);
int moveFigureRight(Params_t *param);

int rotatePiece(Params_t *param);
int canRotate(Params_t *param);
void findMinXY(const int shape[BLOCK][BLOCK], int *minX, int *minY);
void adjustPositionForLine(Params_t *param);
void rotateShape(int shape[BLOCK][BLOCK]);

void updateCurrentGameInfo(Params_t *param);
void cleanStr(Params_t *param, int y);
void copyStr(Params_t *param, int y);
void updateInfo(Params_t *param, int line_full);

int getHighScore(void);
int addHighScore(int newScore);
void cleanGame(void);

#endif