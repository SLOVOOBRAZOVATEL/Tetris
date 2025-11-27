#include "main.h"

int main() {
  GameGUI gui = {0};
  initGUI(&gui);
  initGame();
  gameLoop(&gui);
  cleanGame();
  deinitGUI(&gui);
  return 0;
}

/**
 * @brief Инициализация графического интерфейса с помощью ncurses.
 *
 * Выполняет инициализацию библиотеки ncurses, настройку окон, цветов,
 * размеров и других параметров. Создает окна для поля, следующей фигуры
 * и информационной панели.
 *
 * @param gui Указатель на структуру, в которую сохраняются настройки и окна.
 */
void initGUI(GameGUI *gui) {
  initscr();
  nodelay(stdscr, TRUE);
  cbreak();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  gui->colors_supported = has_colors();
  if (gui->colors_supported) {
    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(4, COLOR_GREEN, COLOR_BLACK);
    init_pair(5, COLOR_RED, COLOR_BLACK);
    init_pair(6, COLOR_BLUE, COLOR_BLACK);
    init_pair(7, COLOR_WHITE, COLOR_BLACK);
  }

  gui->field_height = 22;
  gui->field_width = 12;

  gui->game_win = newwin(gui->field_height, gui->field_width, 1, 1);
  box(gui->game_win, 0, 0);
  wattron(gui->game_win, A_BOLD);

  gui->next_win = newwin(6, 7, 1, gui->field_width + 3);
  box(gui->next_win, 0, 0);

  gui->info_win = newwin(6, 24, 1, gui->field_width + 10);

  clear();
  refresh();
}

/**
 * @brief Основной цикл игры. Обрабатывает ввод, обновляет состояние
 * игры и вызывает отрисовку, пока не завершится.
 *
 * В цикле происходит:
 * - вызов processUserInput для чтения и обработки ввода
 * - вызов updateCurrentState для получения актуальных данных
 * - вызов renderGame для отображения текущего состояния
 * - задержка для регулировки скорости обновления
 *
 * Цикл завершает работу, когда display.pause становится равен 2,
 * что означает завершение игры.
 *
 * @param gui Указатель на структуру графического интерфейса.
 */
void gameLoop(GameGUI *gui) {
  int end = 0;
  while (end != 2) {
    processUserInput();
    GameInfo_t display = updateCurrentState();
    end = display.pause;
    renderGame(gui, display);
    napms(50);
  }
}

/**
 * @brief Обрабатывает пользовательский ввод, вызывая getch() и передавая
 * полученное действие в основную логику игры.
 *
 * Эта функция вызывается в основном цикле игры для чтения символа
 * с клавиатуры. Если пользователь что-то нажал (ch != ERR(nodelay())),
 * то преобразует символ в действие и вызывает userInput для обработки.
 */
void processUserInput(void) {
  int ch = getch();
  if (ch != ERR) {
    UserAction_t act = getAct(ch);
    bool hold = (act == Down) ? true : false;
    userInput(act, hold);
  }
}

/**
 * @brief Обрабатывает ввод пользователя, получая символ клавиши и
 * преобразует его в действие внутри игры.
 *
 * Эта функция читает символ с клавиатуры, используя getch(), и в зависимости
 * от символа определяет действие пользователя, например, старт, пауза,
 * перемещение фигур, или завершение игры. Возвращает перечисление типа
 * UserAction_t, которое далее используется для обработки логики игры.
 *
 * @param ch Символ, считанный с клавиатуры.
 * @return Значение типа UserAction_t, соответствующее действию пользователя.
 */
UserAction_t getAct(int ch) {
  UserAction_t action = -1;
  switch (ch) {
    case '\n':
      action = Start;
      break;
    case 'p':
    case 'P':
      action = Pause;
      break;
    case 'q':
    case 'Q':
      action = Terminate;
      break;
    case KEY_LEFT:
      action = Left;
      break;
    case KEY_RIGHT:
      action = Right;
      break;
    case KEY_UP:
      action = Up;
      break;
    case KEY_DOWN:
      action = Down;
      break;
    case ' ':
      action = Action;
      break;
    default:
      break;
  }
  return action;
}

/**
 * @brief Отрисовывает текущую игровую сцену, включая игровое поле, следующую
 * фигуру, информационную панель и состояние паузы или окончания игры.
 *
 * Эта функция отвечает за обновление графического отображения игры.
 * Вначале вызываются функции drawGameField, drawNextPiece, drawInfo для
 * отображения актуальных данных. Затем, если игра поставлена на паузу,
 * отображается сообщение "PAUSE" с инверсией цвета. Если игра завершена,
 * вызывается функция gameOverScreen для отображения экрана конца.
 *
 * @param gui Указатель на структуру, содержащую все окна и настройки графики.
 * @param display Структура, содержащая текущие данные состояния игры.
 */
void renderGame(GameGUI *gui, GameInfo_t display) {
  drawGameField(gui, display.field);
  drawNextPiece(gui, display.next);
  drawInfo(gui, display);
  if (display.pause == 1) {
    int rows, cols;
    getmaxyx(gui->game_win, rows, cols);
    const char *pauseMsg = "PAUSE";

    wattron(gui->game_win, A_BOLD | A_REVERSE);

    mvwprintw(gui->game_win, rows / 2, (cols - strlen(pauseMsg)) / 2, "%s",
              pauseMsg);

    wattroff(gui->game_win, A_BOLD | A_REVERSE);

    wrefresh(gui->game_win);
  }
  if (display.pause == 2) {
    gameOverScreen(display);
  }
}

/**
 * @brief Отображает экран состояния "Game Over" в центре окна.
 *
 * Создает временное окно с рамкой, выводит сообщение "Game Over",
 * счет и уровень, а также инструкцию "Press any key". После этого
 * ждет нажатия любой клавиши и удаляет окно.
 *
 * @param display Структура с данными о текущем состоянии игры (счет, уровень и
 * др.).
 */
void gameOverScreen(GameInfo_t display) {
  int height = 15, width = 50;
  int start_y = (LINES - height) / 2;
  int start_x = (COLS - width) / 2;

  WINDOW *game_over_win = newwin(height, width, start_y, start_x);
  box(game_over_win, 0, 0);

  wattron(game_over_win, A_REVERSE);
  mvwprintw(game_over_win, 2, (width - 12) / 2, "Game Over");
  wattroff(game_over_win, A_REVERSE);

  mvwprintw(game_over_win, 4, 6, "Score: %d", display.score);
  mvwprintw(game_over_win, 6, 6, "Level: %d", display.level);

  mvwprintw(game_over_win, 10, (width - 16) / 2, "Press any key");

  wrefresh(game_over_win);
  wgetch(game_over_win);
  delwin(game_over_win);
}

/**
 * @brief Отрисовывает следующую фигуру в окне "Next".
 *
 * Эта функция очищает окно, рисует рамку и отображает текущую
 * следующую фигуру в виде матрицы 4x4. Каждая ячейка матрицы
 * отображается блоком, цвет блока зависит от значения в матрице.
 *
 * @param gui Указатель на структуру графического интерфейса, содержащую
 *            окна и настройки цветовой поддержки.
 * @param next Указатель на двумерный массив 4x4, содержащий текущий
 *             вариант следующей фигуры. Значения > 0 соответствуют
 *             цветам блока.
 */
void drawNextPiece(GameGUI *gui, int **next) {
  wclear(gui->next_win);
  box(gui->next_win, 0, 0);
  mvwprintw(gui->next_win, 0, 1, "Next:");

  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (next[y][x] > 0) {
        if (gui->colors_supported) {
          wattron(gui->next_win, COLOR_PAIR(next[y][x]));
        }
        mvwaddch(gui->next_win, 2 + y, 2 + x, ACS_BLOCK);
        if (gui->colors_supported) {
          wattroff(gui->next_win, COLOR_PAIR(next[y][x]));
        }
      }
    }
  }

  wrefresh(gui->next_win);
}

/**
 * @brief Отрисовывает игровое поле.
 *
 * Эта функция очищает окно поля, рисует границы и отображает
 * текущие занятые ячейки поля в виде блоков. Каждая ячейка
 * отображается цветом, если поддерживается цвет.
 *
 * @param gui Указатель на структуру графического интерфейса.
 * @param field Двумерный массив размером 20x10, содержащий состояние
 *              игрового поля. Значения > 0 соответствуют цветам блока.
 */
void drawGameField(GameGUI *gui, int **field) {
  wclear(gui->game_win);
  box(gui->game_win, 0, 0);
  for (int y = 0; y < 20; y++) {
    for (int x = 0; x < 10; x++) {
      if (field[y][x]) {
        if (gui->colors_supported) {
          wattron(gui->game_win, COLOR_PAIR(field[y][x]));
        }
        mvwaddch(gui->game_win, y + 1, x + 1, ACS_BLOCK);
        if (gui->colors_supported) {
          wattroff(gui->game_win, COLOR_PAIR(field[y][x]));
        }
      }
    }
  }
  wattron(gui->game_win, A_BOLD);
  wrefresh(gui->game_win);
}

/**
 * @brief Отрисовывает информационную панель с текущими данными.
 *
 * Эта функция отображает счет, рекорд, уровень, скорость, а также
 * состояние паузы или конца игры. В случае паузы или окончания
 * игры выводит соответствующие сообщения.
 *
 * @param gui Указатель на структуру графического интерфейса.
 * @param display Структура, содержащая текущие игровые данные, такие как
 *                счет, рекорд, уровень, скорость и состояние паузы.
 */
void drawInfo(GameGUI *gui, GameInfo_t display) {
  wclear(gui->info_win);
  box(gui->info_win, 0, 0);
  mvwprintw(gui->info_win, 1, 1, "Score: %d", display.score);
  mvwprintw(gui->info_win, 2, 1, "High Score: %d", display.high_score);
  mvwprintw(gui->info_win, 3, 1, "Level: %d", display.level);
  mvwprintw(gui->info_win, 4, 1, "Speed: %d", display.speed);
  if (display.pause == 1) {
    mvwprintw(gui->info_win, 6, 1, "PAUSE");
  } else if (display.pause == 2) {
    mvwprintw(gui->info_win, 6, 1, "GAME OVER");
    mvwprintw(gui->info_win, 7, 1, "Press Q to Exit");
    if (display.score > display.high_score) {
      mvwprintw(gui->info_win, 8, 1, "New high score!");
    }
  }
  wrefresh(gui->info_win);
}

/**
 * @brief Освобождает ресурсы, удаляет окна и завершает работу ncurses.
 *
 * Перед завершением программы вызывается для освобождения ресурсов,
 * удаления окон и вызова endwin() для корректного завершения работы ncurses.
 *
 * @param gui Указатель на структуру графического интерфейса.
 */
void deinitGUI(GameGUI *gui) {
  if (gui->game_win) delwin(gui->game_win);
  if (gui->next_win) delwin(gui->next_win);
  if (gui->info_win) delwin(gui->info_win);
  endwin();
}
