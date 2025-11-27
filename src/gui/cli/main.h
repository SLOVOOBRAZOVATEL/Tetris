#ifndef MAIN_H
#define MAIN_H

#include "../../brick_game/tetris/tetris.h"

/**
 * @brief Управляет интерфейсом игры, включая окна и
 * отображение информации о размере поля и поддержку цветов.
 */
typedef struct {
  WINDOW *game_win;       // Окно игрового поля
  WINDOW *next_win;       // Окно следующей фигуры
  WINDOW *info_win;       // Окно для счёта
  int field_height;       // Высота поля
  int field_width;        // Ширина поля
  bool colors_supported;  // Флаг поддержки цветов
} GameGUI;

UserAction_t getAct(int ch);
void processUserInput(void);
void renderGame(GameGUI *gui, GameInfo_t display);
void gameOverScreen(GameInfo_t display);
void gameLoop(GameGUI *gui);
void initGUI(GameGUI *gui);
void drawGameField(GameGUI *gui, int **field);  // Изменено на int **
void drawNextPiece(GameGUI *gui, int **next);   // Изменено на int **
void drawInfo(GameGUI *gui, GameInfo_t display);
void deinitGUI(GameGUI *gui);  // Очистка и выход из ncurses

#endif