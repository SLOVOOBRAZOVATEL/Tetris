#include "tetris.h"

/**
 * @brief Инициализация состояния игры.
 *
 * Создает статическую структуру состояния игры и возвращает указатель на нее.
 *
 * @return Указатель на структуру Status_game.
 */
Status_game *initGameState(void) {
  static Status_game state;
  return &state;
}

/**
 * @brief Инициализация текущей информации о игре.
 *
 * Создает статическую структуру с информацией о текущем состоянии игры,
 * очках, уровне и т.п., и возвращает указатель на нее.
 *
 * @return Указатель на структуру GameInfo_t.
 */
GameInfo_t *initGameInfo(void) {
  static GameInfo_t game_info;
  return &game_info;  // Возврат указателя на текущую информацию о игре
}

/**
 * @brief Инициализация параметров игры.
 *
 * Создает статическую структуру Params_t и возвращает указатель на нее.
 *
 * @return Указатель на структуру Params_t.
 */
Params_t *initGameParam(void) {
  static Params_t param;
  return &param;
}

/**
 * @brief Инициализация игрового состояния и настроек.
 *
 * Эта функция создает и настраивает все необходимые структуры данных
 * для запуска новой игры: состояние, информацию о игре, параметры,
 * игровое поле, следующий блок, очки и уровень. Также запускается первая
 * фигура, и инициализируется генератор случайных чисел.
 */
void initGame(void) {
  Status_game *state = initGameState();
  GameInfo_t *game_info = initGameInfo();
  Params_t *param = initGameParam();

  param->game_info = game_info;
  param->game_info->field = initMatrix(FIELD_HEIGHT, FIELD_WIDTH);
  param->game_info->next = initMatrix(BLOCK, BLOCK);
  param->game_info->score = 0;
  param->game_info->high_score = getHighScore();
  param->game_info->level = 1;
  param->game_info->speed = 1000;
  param->game_info->pause = 0;
  param->fieldForFront = initMatrix(FIELD_HEIGHT, FIELD_WIDTH);

  spawnNewPiece(param, true);
  *state = Start_t;
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  srand((ts.tv_sec ^ ts.tv_nsec) + time(NULL));
}

/**
 * @brief Создает матрицу заданных размеров.
 *
 * Выделяет память под двумерный массив целых чисел размером row x col.
 *
 * @param row Количество строк.
 * @param col Количество столбцов.
 * @return Указатель на выделенную матрицу.
 */
int **initMatrix(int row, int col) {
  int **array = (int **)calloc(row, sizeof(int *));
  for (int i = 0; i < row; i++) {
    array[i] = (int *)calloc(col, sizeof(int));
  }
  return array;
}

/**
 * @brief Освобождает память, занятую матрицей.
 *
 * Освобождает память, выделенную под матрицу field размером row.
 *
 * @param field Указатель на матрицу.
 * @param row Количество строк.
 */
void freeMatrix(int **field, int row) {
  for (int i = 0; i < row; i++) {
    free(field[i]);
  }
  free(field);
}

/**
 * @brief Обновление состояния игры.
 *
 * Основная функция, управляющая состоянием игры, переключает состояния,
 * обрабатывает таймауты, движение фигур, проверяет условия окончания игры.
 * Метод конечного автомата.
 *
 * @return Структура с текущей информацией о состоянии игры (`GameInfo_t`).
 */
GameInfo_t updateCurrentState(void) {
  Status_game *state = initGameState();
  Params_t *param = initGameParam();
  int return_check = 1;

  if (param->game_info->pause == 1 || !param->start) {
    return_check = 0;
  }
  if (return_check) {
    struct timespec cur_time = {0};
    double drop_interval = 0;

    switch (*state) {
      case Start_t:
        if (param->start) *state = Spawn;
        break;
      case Spawn:
        if (spawnNewPiece(param, false)) {
          *state = Moving;
        } else {
          *state = GameOver;
        }
        break;
      case Moving:
        if (getTime(param, &drop_interval, &cur_time)) {
          *state = Shifting;
        }
        break;
      case Shifting:
        if (!moveFigureDown(param)) {
          *state = Attaching;
        } else {
          *state = Moving;
        }
        break;
      case Pause_t:
        break;
      case Attaching:
        fixFigure(param);
        updateCurrentGameInfo(param);
        *state = Spawn;
        break;
      case Terminate_t:
      case GameOver:
        param->game_info->pause = 2;
        addHighScore(param->game_info->score);
        break;
      default:
        break;
    }
  }
  return createGameInfo(*param, return_check);
}

/**
 * @brief Создает актуальное состояние для отображения.
 *
 * Формирует структуру `GameInfo_t`, основываясь на текущем состоянии игрового
 * поля, фигуре, очках, уровне и т.п., для дальнейшей отрисовки.
 *
 * @param param Текущие параметры игры.
 * @param return_check Статус, указывающий, нужно ли обновлять состояние.
 * @return Обновленная структура `GameInfo_t`.
 */
GameInfo_t createGameInfo(const Params_t param, const int return_check) {
  if (!return_check) {
    GameInfo_t *game = param.game_info;
    return *game;
  }
  GameInfo_t game = {0};

  for (int i = 0; i < FIELD_HEIGHT; i++) {
    for (int j = 0; j < FIELD_WIDTH; j++) {
      param.fieldForFront[i][j] = param.game_info->field[i][j];
    }
  }

  overlayCurrentPiece(&param);
  game.score = param.game_info->score;
  game.high_score = param.game_info->high_score;
  game.level = param.game_info->level;
  game.speed = param.game_info->speed;
  game.pause = param.game_info->pause;
  game.field = param.fieldForFront;
  game.next = param.game_info->next;

  return game;
}

/**
 * @brief Генерирует новую фигуру.
 *
 * Создает новую фигуру из шаблонов фигур, устанавливает ее параметры,
 * проверяет возможность размещения, и обновляет состояние.
 *
 * @param param Указатель на параметры игры.
 * @param isInit Флаг, указывающий, инициализировать ли фигуру впервые.
 * @return Возвращает 1, если фигура успешно создана, иначе 0.
 */
int spawnNewPiece(Params_t *param, bool isInit) {
  int return_value = 1;
  static const int pieces[7][BLOCK][BLOCK] = {
      {{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}},
      {{1, 1, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
      {{0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
      {{1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
      {{0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
      {{0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
      {{1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}};

  if (isInit) {
    int indexNext = rand() % 7;

    copyPieceShape(param->nextPiece.shape, pieces[indexNext]);

    param->nextPiece.type = indexNext;
    param->nextPiece.rotation = 0;
    param->nextPiece.posX = (FIELD_WIDTH / 2) - (BLOCK / 2);
    param->nextPiece.posY = 0;

    for (int i = 0; i < BLOCK; i++) {
      for (int j = 0; j < BLOCK; j++) {
        param->game_info->next[i][j] =
            param->nextPiece.shape[i][j] ? (param->nextPiece.type + 1) : 0;
      }
    }
  } else {
    copyPieceShape(param->currentPiece.shape, param->nextPiece.shape);
    param->currentPiece.type = param->nextPiece.type;
    param->currentPiece.rotation = 0;
    param->currentPiece.posX = (FIELD_WIDTH / 2) - (BLOCK / 2);
    param->currentPiece.posY = 0;

    if (canPlacePiece(param)) {
      int index = rand() % 7;

      copyPieceShape(param->nextPiece.shape, pieces[index]);

      param->nextPiece.type = index;
      param->nextPiece.rotation = 0;
      param->nextPiece.posX = (FIELD_WIDTH / 2) - (BLOCK / 2);
      param->nextPiece.posY = 0;
      for (int i = 0; i < BLOCK; i++) {
        for (int j = 0; j < BLOCK; j++) {
          param->game_info->next[i][j] =
              param->nextPiece.shape[i][j] ? (param->nextPiece.type + 1) : 0;
        }
      }
    } else
      return_value = 0;
  }
  return return_value;
}

/**
 * @brief Копирует форму фигуры.
 *
 * Копирует массив формы фигуры из `src` в `dest`.
 *
 * @param dest Массив назначения.
 * @param src Массив источника.
 */
void copyPieceShape(int dest[BLOCK][BLOCK], const int src[BLOCK][BLOCK]) {
  for (int i = 0; i < BLOCK; i++) {
    for (int j = 0; j < BLOCK; j++) {
      dest[i][j] = src[i][j];
    }
  }
}

/**
 * @brief Проверяет, можно ли разместить фигуру.
 *
 * Проверяет, не выходит ли фигура за границы поля и не пересекается ли с уже
 * занятыми клетками.
 *
 * @param param Указатель на параметры игры.
 * @return 1, если фигуру можно разместить, иначе 0.
 */
int canPlacePiece(Params_t *param) {
  int return_value = 1;
  for (int i = 0; i < BLOCK; i++) {
    for (int j = 0; j < BLOCK; j++) {
      if (param->currentPiece.shape[i][j]) {
        int x = param->currentPiece.posX + j;
        int y = param->currentPiece.posY + i;

        if (x < 0 || x >= FIELD_WIDTH || y < 0 || y >= FIELD_HEIGHT ||
            param->game_info->field[y][x])
          return_value = 0;
      }
    }
  }
  return return_value;
}

/**
 * @brief Фиксирует фигуру на поле.
 *
 * Вносит текущую фигуру в игровое поле, обновляя матрицу поля.
 *
 * @param param Указатель на параметры игры.
 */
void fixFigure(Params_t *param) {
  for (int i = 0; i < BLOCK; i++) {
    for (int j = 0; j < BLOCK; j++) {
      if (param->currentPiece.shape[i][j]) {
        int x = param->currentPiece.posX + j;
        int y = param->currentPiece.posY + i;

        if (x >= 0 && x < FIELD_WIDTH && y >= 0 && y < FIELD_HEIGHT) {
          param->game_info->field[y][x] = param->currentPiece.type + 1;
        }
      }
    }
  }
}

/**
 * @brief Обновляет информацию о текущем состоянии поля.
 *
 * Проверяет заполненные линии, очищает их, сдвигает верхние строки вниз,
 * обновляет счет и уровень.
 *
 * @param param Указатель на параметры игры.
 */
void updateCurrentGameInfo(Params_t *param) {
  int line_full = 0;
  for (int y = FIELD_HEIGHT - 1; y >= 0; y--) {
    int full_check = 0;
    for (int x = 0; x < FIELD_WIDTH; x++) {
      if (param->game_info->field[y][x]) {
        full_check++;
      }
    }
    if (full_check == FIELD_WIDTH) {
      line_full++;
      cleanStr(param, y);
      copyStr(param, y);
      y++;
    }
  }
  if (line_full) {
    updateInfo(param, line_full);
  }
}

/**
 * @brief Удаляет заполненную строку.
 *
 * Обнуляет строку `y` в игровом поле.
 *
 * @param param Указатель на параметры игры.
 * @param y Индекс строки для очистки.
 */
void cleanStr(Params_t *param, int y) {
  for (int x = 0; x < FIELD_WIDTH; x++) {
    param->game_info->field[y][x] = 0;
  }
}

/**
 * @brief Сдвигает строки вверх.
 *
 * Сдвигает все строки выше строки `y` вниз, очищая верхнюю строку.
 *
 * @param param Указатель на параметры игры.
 * @param y Индекс строки, начиная с которой происходит сдвиг.
 */
void copyStr(Params_t *param, int y) {
  for (int row = y; row > 0; row--) {
    for (int col = 0; col < FIELD_WIDTH; col++) {
      param->game_info->field[row][col] = param->game_info->field[row - 1][col];
    }
  }
  for (int col = 0; col < FIELD_WIDTH; col++) {
    param->game_info->field[0][col] = 0;
  }
}

/**
 * @brief Обновляет счет и уровень при очистке линий.
 *
 * В зависимости от количества очищенных линий, увеличивает счет,
 * обновляет уровень и скорость падения фигур.
 *
 * @param param Указатель на параметры игры.
 * @param line_full Количество очищенных линий.
 */
void updateInfo(Params_t *param, int line_full) {
  int newScore = 0;
  switch (line_full) {
    case 1:
      newScore = 100;
      break;
    case 2:
      newScore = 300;
      break;
    case 3:
      newScore = 700;
      break;
    case 4:
      newScore = 1500;
      break;
    default:
      newScore = 1500;
      break;
  }
  param->game_info->score += newScore;

  int newLevel = param->game_info->score / 600 + 1;
  if (newLevel > 10) newLevel = 10;

  if (newLevel != param->game_info->level) {
    param->game_info->level = newLevel;
    param->game_info->speed = 1000 - (newLevel - 1) * 100;
    if (param->game_info->speed < 100) param->game_info->speed = 100;
  }
}

/**
 * @brief Показывает текущую фигуру на игровом поле.
 *
 * Эта функция добавляет текущую активную фигуру в массив `fieldForFront`,
 * чтобы её было видно на экране. Она размещает фигуру в текущем положении.
 *
 * @param param Указатель на параметры игры.
 */
void overlayCurrentPiece(const Params_t *param) {
  const Piece *p = &param->currentPiece;

  for (int i = 0; i < BLOCK; i++) {
    for (int j = 0; j < BLOCK; j++) {
      if (p->shape[i][j]) {
        int x = p->posX + j;
        int y = p->posY + i;

        if (x >= 0 && x < FIELD_WIDTH && y >= 0 && y < FIELD_HEIGHT) {
          param->fieldForFront[y][x] = p->type + 1;
        }
      }
    }
  }
}

/**
 * @brief Проверяет таймер на падение фигуры.
 *
 * Определяет, пора ли опустить фигуру в соответствии с текущей скоростью.
 *
 * @param param Указатель на параметры игры.
 * @param drop_interval Указатель на переменную для времени падения.
 * @param cur_time Структура времени.
 * @return 1, если фигура должна опуститься, иначе 0.
 */
int getTime(Params_t *param, double *drop_interval, struct timespec *cur_time) {
  int return_value = 0;
  clock_gettime(CLOCK_MONOTONIC, cur_time);
  *drop_interval = (cur_time->tv_sec - param->drop_time.tv_sec) * 1000 +
                   (cur_time->tv_nsec - param->drop_time.tv_nsec) / 1000000;
  if (*drop_interval >= param->game_info->speed) {
    param->drop_time = *cur_time;
    return_value = 1;
  }
  return return_value;
}

/**
 * @brief Сдвигает фигуру вниз.
 *
 * Проверяет, можно ли сдвинуть фигуру вниз, и осуществляет сдвиг.
 *
 * @param param Указатель на параметры игры.
 * @return 1, если удалось сдвинуть, иначе 0.
 */
int moveFigureDown(Params_t *param) {
  Piece *cur = &param->currentPiece;
  int newPosY = cur->posY + 1;
  int return_value = 1;
  for (int i = 0; i < BLOCK; i++) {
    for (int j = 0; j < BLOCK; j++) {
      if (cur->shape[i][j]) {
        int x = cur->posX + j;
        int y = newPosY + i;

        if (y >= FIELD_HEIGHT || param->game_info->field[y][x]) {
          return_value = 0;
        }
      }
    }
  }

  if (return_value != 0) cur->posY = newPosY;
  return return_value;
}

/**
 * @brief Сдвигает фигуру влево.
 *
 * Проверяет возможность сдвига и осуществляет его.
 *
 * @param param Указатель на параметры игры.
 * @return 1, если сдвиг возможен, иначе 0.
 */
int moveFigureLeft(Params_t *param) {
  Piece *cur = &param->currentPiece;
  int newPosX = cur->posX - 1;
  int return_value = 1;

  for (int i = 0; i < BLOCK; i++) {
    for (int j = 0; j < BLOCK; j++) {
      if (cur->shape[i][j]) {
        int x = newPosX + j;
        int y = cur->posY + i;

        if (x < 0 || param->game_info->field[y][x]) return_value = 0;
      }
    }
  }

  if (return_value != 0) cur->posX = newPosX;
  return return_value;
}

/**
 * @brief Сдвигает фигуру вправо.
 *
 * Проверяет возможность сдвига и осуществляет его.
 *
 * @param param Указатель на параметры игры.
 * @return 1, если сдвиг возможен, иначе 0.
 */
int moveFigureRight(Params_t *param) {
  Piece *cur = &param->currentPiece;
  int newPosX = cur->posX + 1;
  int return_value = 1;

  for (int i = 0; i < BLOCK; i++) {
    for (int j = 0; j < BLOCK; j++) {
      if (cur->shape[i][j]) {
        int x = newPosX + j;
        int y = cur->posY + i;

        if (x >= FIELD_WIDTH || param->game_info->field[y][x]) return_value = 0;
      }
    }
  }

  if (return_value != 0) cur->posX = newPosX;
  return return_value;
}

/**
 * @brief Поворачивает фигуру.
 *
 * Выполняет вращение фигуры, проверяет столкновения и корректирует позицию.
 *
 * @param param Указатель на параметры игры.
 * @return 1, если поворот выполнен успешно, иначе 0.
 */
int rotatePiece(Params_t *param) {
  if (!canRotate(param)) return 0;
  if (param->currentPiece.type == 1) return 1;

  int return_value = 1;
  int oldShape[BLOCK][BLOCK];
  copyPieceShape(oldShape, param->currentPiece.shape);
  int oldPosX = param->currentPiece.posX;
  int oldPosY = param->currentPiece.posY;

  rotateShape(param->currentPiece.shape);

  int oldMinX, oldMinY, newMinX, newMinY;
  findMinXY(oldShape, &oldMinX, &oldMinY);
  findMinXY(param->currentPiece.shape, &newMinX, &newMinY);

  int offsetX = oldMinX - newMinX;
  int offsetY = oldMinY - newMinY;

  param->currentPiece.posX = oldPosX + offsetX;
  param->currentPiece.posY = oldPosY + offsetY;

  adjustPositionForLine(param);

  if (!canPlacePiece(param)) {
    copyPieceShape(param->currentPiece.shape, oldShape);
    param->currentPiece.posX = oldPosX;
    param->currentPiece.posY = oldPosY;
    return_value = 0;
  } else {
    param->currentPiece.rotation = (param->currentPiece.rotation + 1) % 4;
  }

  return return_value;
}

/**
 * @brief Проверяет возможность поворота.
 *
 * Проверяет, можно ли повернуть фигуру без столкновений или выхода за границы.
 *
 * @param param Указатель на параметры игры.
 * @return 1, если поворот возможен, иначе 0.
 */
int canRotate(Params_t *param) {
  int tmp[BLOCK][BLOCK];
  Piece *cur = &param->currentPiece;
  int return_value = 1;

  for (int i = 0; i < BLOCK; i++) {
    for (int j = 0; j < BLOCK; j++) {
      tmp[j][BLOCK - 1 - i] = cur->shape[i][j];
    }
  }

  for (int i = 0; i < BLOCK; i++) {
    for (int j = 0; j < BLOCK; j++) {
      if (tmp[i][j]) {
        int x = cur->posX + j;
        int y = cur->posY + i;

        if (x < 0 || x >= FIELD_WIDTH || y < 0 || y >= FIELD_HEIGHT ||
            param->game_info->field[y][x])
          return_value = 0;
      }
    }
  }
  return return_value;
}

// Вспомогательная функция для нахождения минимальных X и Y в форме
/**
 * @brief Находит минимальные координаты X и Y, в которых есть части формы.
 * @param shape Матрица формы.
 * @param minX Указатель на переменную для хранения минимального X.
 * @param minY Указатель на переменную для хранения минимального Y.
 */
void findMinXY(const int shape[BLOCK][BLOCK], int *minX, int *minY) {
  *minX = BLOCK;
  *minY = BLOCK;
  for (int i = 0; i < BLOCK; i++) {
    for (int j = 0; j < BLOCK; j++) {
      if (shape[i][j]) {
        if (j < *minX) *minX = j;
        if (i < *minY) *minY = i;
      }
    }
  }
}

// Вспомогательная функция для корректировки позиции палки при вращении
/**
 * @brief Корректирует позицию фигуры типа "палка" после вращения.
 * @param param Указатель на параметры игры, содержащие текущую фигуру.
 */
void adjustPositionForLine(Params_t *param) {
  if (param->currentPiece.type == 0) {  // тип 0 — палка
    if (param->currentPiece.rotation % 2 == 0) {
      param->currentPiece.posX += 1;
      param->currentPiece.posY -= 1;
    } else {
      param->currentPiece.posX -= 1;
      param->currentPiece.posY += 1;
    }
  }
}

/**
 * @brief Поворачивает матрицу фигуры на 90 градусов по часовой.
 *
 * Меняет местами элементы матрицы, реализуя поворот.
 *
 * @param shape Массив формы фигуры.
 */
void rotateShape(int shape[BLOCK][BLOCK]) {
  int temp[BLOCK][BLOCK];

  for (int i = 0; i < BLOCK; i++) {
    for (int j = 0; j < BLOCK; j++) {
      temp[j][BLOCK - 1 - i] = shape[i][j];
    }
  }

  for (int i = 0; i < BLOCK; i++) {
    for (int j = 0; j < BLOCK; j++) {
      shape[i][j] = temp[i][j];
    }
  }
}

/**
 * @brief Обрабатывает пользовательский ввод.
 *
 * В зависимости от действия, управляет движением и вращением фигуры,
 * а также паузой, стартом, остановкой.
 *
 * @param action Тип действия пользователя.
 * @param hold Флаг удержания клавиши (для "свободного" падения).
 */
void userInput(UserAction_t action, bool hold) {
  Params_t *param = initGameParam();
  Status_game *state = initGameState();

  if (!hold) {
    switch (action) {
      case Start:
        if (*state == Start_t || *state == GameOver) {
          param->start = true;
          *state = Spawn;
        }
        break;
      case Pause:
        if (*state == Pause_t) {
          *state = Moving;
          param->game_info->pause = 0;
        } else if (*state == Moving) {
          *state = Pause_t;
          param->game_info->pause = 1;
        }
        break;
      case Terminate:
        if ((param->game_info->score) > (param->game_info->high_score)) {
          addHighScore(param->game_info->score);
        }
        if (param->game_info->pause == 1) {
          param->game_info->pause = 2;
        }
        *state = GameOver;
        break;
      case Left:
        if (*state == Moving && !param->game_info->pause) {
          moveFigureLeft(param);
        }
        break;
      case Right:
        if (*state == Moving && !param->game_info->pause) {
          moveFigureRight(param);
        }
        break;
      case Up:
        break;
      case Down:
        break;
      case Action:
        if (*state == Moving && !param->game_info->pause) {
          rotatePiece(param);
        }
      default:
        break;
    }
  } else {
    if (*state == Moving && !param->game_info->pause) {
      while (moveFigureDown(param)) {
      }
    }
  }
}

/**
 * @brief Получает текущий рекорд из файла.
 *
 * Читает рекорд из файла "highscore.txt".
 *
 * @return Значение рекорда.
 */
int getHighScore(void) {
  int return_value;
  FILE *file = fopen("highscore.txt", "r");
  if (file == NULL) {
    return_value = 0;
  } else {
    int highScore = 0;
    fscanf(file, "%d", &highScore);
    return_value = highScore;
    fclose(file);
  }
  
  return return_value;
}

/**
 * @brief Обновляет рекорд в файле, если текущий счет больше.
 *
 * Записывает новый рекорд в файл, если текущий счет превышает предыдущий.
 *
 * @param newScore Текущий счет.
 * @return 1, если рекорд обновлен, иначе 0.
 */
int addHighScore(int newScore) {
  int return_value = 1;
  int currentHigh = getHighScore();
  if (newScore <= currentHigh) {
    return 0;
  }
  FILE *file = fopen("highscore.txt", "w");
  if (file == NULL) {
    return_value = 0;
  } else {
    fprintf(file, "%d", newScore);
    fclose(file);
  }
  return return_value;
}

/**
 * @brief Очищает текущую игру.
 *
 * Освобождает выделенную память и сбрасывает состояние игры.
 */
void cleanGame(void) {
  Params_t *param = initGameParam();
  if (param && param->game_info) {
    if (param->game_info->field) {
      freeMatrix(param->game_info->field, FIELD_HEIGHT);
      param->game_info->field = NULL;
    }
    if (param->game_info->next) {
      freeMatrix(param->game_info->next, BLOCK);
      param->game_info->next = NULL;
    }
    if (param->fieldForFront) {
      freeMatrix(param->fieldForFront, FIELD_HEIGHT);
      param->fieldForFront = NULL;
    }
  }
}
