//-------------------------------------------------------------------------------
// Copyright (c) 2020-2022 Fábio Pichler
/*-------------------------------------------------------------------------------

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

-------------------------------------------------------------------------------*/

#include "GameBoard.h"
#include "../base/SceneManager.h"
#include "../base/Button.h"
#include "../base/Texture.h"
#include "../base/Rectangle.h"
#include "../base/Box.h"

#include <stdlib.h>

#define ROWS 4
#define COLS 8

typedef struct GameEvent
{
    GameEventHandler function;
    void *userdata;
} GameEvent;

typedef struct Images
{
    const int id;
    const char * const image;
} Images;

const Images images[] = {
    {1, "images/beaming-face-with-smiling-eyes_1f601.png"},
    {2, "images/birthday-cake_1f382.png"},
    {3, "images/cat-face_1f431.png"},
    {4, "images/cherry-blossom_1f338.png"},
    {5, "images/dog-face_1f436.png"},
    {6, "images/doughnut_1f369.png"},
    {7, "images/floppy-disk_1f4be.png"},
    {8, "images/four-leaf-clover_1f340.png"},
    {9, "images/globe-showing-americas_1f30e.png"},
    {10, "images/guitar_1f3b8.png"},
    {11, "images/laptop_1f4bb.png"},
    {12, "images/red-heart_2764-fe0f.png"},
    {13, "images/ringed-planet_1fa90.png"},
    {14, "images/rocket_1f680.png"},
    {15, "images/violin_1f3bb.png"},
    {16, "images/watermelon_1f349.png"},
};

typedef struct TimerData
{
    GameBoard *self;
    BoardItem *last_item;
    BoardItem *current_item;
} TimerData;

struct GameBoard
{
    SceneManager *sceneManager;
    SDL_Renderer *renderer;
    Rectangle *background;

    Player player;
    Player gameResult;
    int round;

    bool blockedEvents;
    int lastItemId;
    int lastImageId;
    int count;
    int player1Count, player2Count;

    struct Board
    {
        SDL_Rect rect;
        int item_size;
        int space;
        BoardItem items[ROWS][COLS];
    } board;

    GameEvent gameEvent;
};

void GameBoard_SetupBoard(GameBoard * const self);
void GameBoard_OnItemPress(Button * const button, void *user);
void GameBoard_CallEventFunction(GameBoard * const self);
void GameBoard_FinalizeEventCallback(GameBoard * const self);
void GameBoard_Check(GameBoard * const self, BoardItem *current_item);
void GameBoard_CheckWinner(GameBoard * const self);
void GameBoard_AddTimer(GameBoard * const self, Uint32 interval, BoardItem *last_item, BoardItem *current_item,
                        SceneManager_TimerCallback callback);
void GameBoard_BlockEvents(GameBoard * const self);
void GameBoard_UnblockEvents(GameBoard * const self);

static void GetTimerData(void *userdata, GameBoard ** self, BoardItem **last_item, BoardItem **current_item);
static BoardItem *GetItem(BoardItem items[ROWS][COLS], int id);
static void SetupColors_Empty(Button *button);
static void SetupColors_FirstItemSelected(Button *current_button);
static void SetupColors_Correct(Button *last_button, Button *current_button);
static void SetupColors_CorrectAfterTimer(Button *last_button, Button *current_button);
static void SetupColors_Wrong(Button *last_button, Button *current_button);
static void SetupColors_WrongAfterTimer(Button *last_button, Button *current_button);

GameBoard *GameBoard_New(SDL_Renderer *renderer, SceneGameRect *sceneGameRect, SceneManager *sceneManager)
{
    GameBoard * const self = malloc(sizeof (GameBoard));

    const int board_size_x = 819;
    const int board_size_y = 407;
    int board_x = sceneGameRect->sidebar_w + ((sceneGameRect->content_w - board_size_x) / 2);
    int board_y = (sceneGameRect->window_h - board_size_y) / 2;

    self->board.item_size = 98;
    self->board.space = 5;
    self->board.rect = (SDL_Rect) {board_x, board_y, board_size_x, board_size_y};

    self->sceneManager = sceneManager;
    self->renderer = renderer;
    self->background = Rectangle_New(self->renderer, board_size_x, board_size_y);
    self->player = Player_1;
    self->gameResult = None;
    self->round = 0;
    self->gameEvent = (GameEvent) {NULL, NULL};

    self->blockedEvents = false;
    self->lastItemId = 0;
    self->lastImageId = 0;
    self->count = 0;
    self->player1Count = 0;
    self->player2Count = 0;

    Box_SetPosition(Rectangle_Box(self->background), self->board.rect.x, self->board.rect.y);
    Rectangle_SetColorRGBA(self->background, 180, 180, 180, 255);

    GameBoard_SetupBoard(self);

    return self;
}

void GameBoard_Delete(GameBoard * const self)
{
    if (!self)
        return;

    for (int row = 0; row < ROWS; ++row)
    {
        for (int col = 0; col < COLS; ++col)
        {
            BoardItem *item = &self->board.items[row][col];

            Button_Delete(item->button);
            Texture_Delete(item->texture);
        }
    }

    Rectangle_Delete(self->background);

    free(self);
}

void GameBoard_ProcessEvent(GameBoard * const self, const SDL_Event *event)
{
    for (int row = 0; row < ROWS; ++row)
        for (int col = 0; col < COLS; ++col)
            Button_ProcessEvent(self->board.items[row][col].button, event);
}

void GameBoard_Update(GameBoard * const self, double deltaTime)
{
}

void GameBoard_Draw(GameBoard * const self)
{
    Rectangle_Draw(self->background);

    for (int row = 0; row < ROWS; ++row)
        for (int col = 0; col < COLS; ++col)
            Button_Draw(self->board.items[row][col].button);
}

void GameBoard_SetGameEvent(GameBoard * const self, GameEventHandler callback, void *user)
{
    self->gameEvent.function = callback;
    self->gameEvent.userdata = user;
}

int GameBoard_GetCurrentPlayer(GameBoard * const self)
{
    return self->player;
}

int GameBoard_GetPlayer1Count(GameBoard * const self)
{
    return self->player1Count;
}

int GameBoard_GetPlayer2Count(GameBoard * const self)
{
    return self->player2Count;
}

int GameBoard_GetGameResult(GameBoard * const self)
{
    return self->gameResult;
}

static void shuffle(const Images **array, size_t n)
{
    if (n < 1)
        return;

    for (size_t i = 0; i < n - 1; i++)
    {
        size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
        const Images *t = array[j];
        array[j] = array[i];
        array[i] = t;
    }
}

void GameBoard_SetupBoard(GameBoard * const self)
{
    const Images *_images[ROWS * COLS];

    for (size_t i = 0, j = 0; i < (ROWS * COLS); ++i)
    {
        _images[i] = &images[j];

        if (i % 2 == 1)
            ++j;
    }

    shuffle(_images, ROWS * COLS);

    for (int row = 0, i = 0; row < ROWS; ++row)
    {
        for (int col = 0; col < COLS; ++col)
        {
            BoardItem *item = &self->board.items[row][col];
            const Images *image = _images[i];
            ++i;

            *item = (BoardItem) {
                .player = 0,
                .button = Button_New(self->renderer),
                .texture = Texture_New(self->renderer),
                .id = i,
                .image_id = image->id,
            };

            Texture_LoadImageFromFile(item->texture, image->image);

            Box_SetSize(Button_Box(item->button), self->board.item_size, self->board.item_size);
            Box_SetPosition(Button_Box(item->button),
                            self->board.rect.x + (col * self->board.item_size) + (col * self->board.space),
                            self->board.rect.y + (row * self->board.item_size) + (row * self->board.space));

            Button_SetOnPressEvent(item->button, GameBoard_OnItemPress, self);
            SetupColors_Empty(item->button);
        }
    }
}

void GameBoard_OnItemPress(Button * const button, void *user)
{
    GameBoard * const self = user;

    for (int row = 0; row < ROWS; ++row)
    {
        for (int col = 0; col < COLS; ++col)
        {
            BoardItem *item = &self->board.items[row][col];

            if (item->button == button)
            {
                GameBoard_Check(self, item);

                return;
            }
        }
    }
}

void GameBoard_CallEventFunction(GameBoard * const self)
{
    self->player = self->player == Player_1 ? Player_2 : Player_1;

    if (self->gameEvent.function)
        self->gameEvent.function(self, self->gameEvent.userdata);
}

void GameBoard_FinalizeEventCallback(GameBoard * const self)
{
    self->lastItemId = 0;
    self->lastImageId = 0;
    self->round++;

    GameBoard_UnblockEvents(self);
}

static void CorrectCallback(void * const manager, void *userdata)
{
    GameBoard *self;
    BoardItem *last_item, *current_item;
    GetTimerData(userdata, &self, &last_item, &current_item);

    if (self->player == Player_1)
        ++self->player1Count;
    else
        ++self->player2Count;

    ++self->count;
    last_item->player = self->player;
    current_item->player = self->player;

    GameBoard_CheckWinner(self);
    GameBoard_CallEventFunction(self);
    SetupColors_CorrectAfterTimer(last_item->button, current_item->button);
    GameBoard_FinalizeEventCallback(self);
}

static void WrongCallback(void * const manager, void *userdata)
{
    GameBoard *self;
    BoardItem *last_item, *current_item;
    GetTimerData(userdata, &self, &last_item, &current_item);

    GameBoard_CallEventFunction(self);
    Button_SetIcon(last_item->button, NULL);
    Button_SetIcon(current_item->button, NULL);

    last_item->player = None;
    current_item->player = None;

    SetupColors_WrongAfterTimer(last_item->button, current_item->button);
    GameBoard_FinalizeEventCallback(self);
}

void GameBoard_Check(GameBoard * const self, BoardItem *current_item)
{
    if (self->blockedEvents || self->gameResult != None)
        return;

    GameBoard_BlockEvents(self);

    if (self->lastImageId == 0)
    {
        if (current_item->player != None)
            return GameBoard_UnblockEvents(self);

        self->lastItemId = current_item->id;
        self->lastImageId = current_item->image_id;

        Button_SetIcon(current_item->button, current_item->texture);
        SetupColors_FirstItemSelected(current_item->button);
        GameBoard_UnblockEvents(self);

        return;
    }

    BoardItem *last_item = GetItem(self->board.items, self->lastItemId);

    if (current_item->player != None || current_item->id == last_item->id)
        return GameBoard_UnblockEvents(self);

    if (self->lastImageId == current_item->image_id)
    {
        Button_SetIcon(current_item->button, current_item->texture);
        SetupColors_Correct(last_item->button, current_item->button);
        GameBoard_AddTimer(self, 500, last_item, current_item, CorrectCallback);
    }
    else
    {
        Button_SetIcon(current_item->button, current_item->texture);
        SetupColors_Wrong(last_item->button, current_item->button);
        GameBoard_AddTimer(self, 1000, last_item, current_item, WrongCallback);
    }
}

void GameBoard_CheckWinner(GameBoard * const self)
{
    if (self->count == 16)
    {
        if (self->player1Count > self->player2Count)
            self->gameResult = Player_1;

        else if (self->player1Count < self->player2Count)
            self->gameResult = Player_2;

        else
            self->gameResult = Tied;
    }
    else
    {
        self->gameResult = None;
    }
}

void GameBoard_AddTimer(GameBoard * const self, Uint32 interval, BoardItem *last_item, BoardItem *current_item,
                        SceneManager_TimerCallback callback)
{
    TimerData *data = malloc(sizeof (TimerData));

    data->self = self;
    data->last_item = last_item;
    data->current_item = current_item;

    SceneManager_AddTimer(self->sceneManager, interval, callback, data);
}

void GetTimerData(void *userdata, GameBoard **self, BoardItem **last_item, BoardItem **current_item)
{
    TimerData *data = userdata;

    *self = data->self;
    *last_item = data->last_item;
    *current_item = data->current_item;

    free(data);
}

BoardItem *GetItem(BoardItem items[ROWS][COLS], int id)
{
    for (int row = 0; row < ROWS; ++row)
    {
        for (int col = 0; col < COLS; ++col)
        {
            BoardItem *item = &items[row][col];

            if (item->id == id)
                return item;
        }
    }

    return NULL;
}

void GameBoard_BlockEvents(GameBoard * const self)
{
    self->blockedEvents = true;
}

void GameBoard_UnblockEvents(GameBoard * const self)
{
    self->blockedEvents = false;
}

void SetupColors_Empty(Button *button)
{
    Button_SetBackgroundColorRGB(button, 240, 240, 240);
    Button_SetBackgroundHoverColorRGB(button, 255, 255, 255);
    Button_SetBackgroundPressedColorRGB(button, 200, 200, 200);
}

void SetupColors_FirstItemSelected(Button *button)
{
    Button_SetBackgroundColorRGB(button, 210, 240, 240);
    Button_SetBackgroundHoverColorRGB(button, 210, 240, 240);
    Button_SetBackgroundPressedColorRGB(button, 210, 240, 240);
}

void SetupColors_Correct(Button *last_button, Button *current_button)
{
    Button_SetBackgroundColorRGB(last_button, 220, 255, 220);
    Button_SetBackgroundHoverColorRGB(last_button, 220, 255, 220);
    Button_SetBackgroundPressedColorRGB(last_button, 220, 255, 220);

    Button_SetBackgroundColorRGB(current_button, 220, 255, 220);
    Button_SetBackgroundHoverColorRGB(current_button, 220, 255, 220);
    Button_SetBackgroundPressedColorRGB(current_button, 220, 255, 220);
}

void SetupColors_CorrectAfterTimer(Button *last_button, Button *current_button)
{
    Button_SetBackgroundColorRGB(last_button, 240, 240, 240);
    Button_SetBackgroundHoverColorRGB(last_button, 240, 240, 240);
    Button_SetBackgroundPressedColorRGB(last_button, 240, 240, 240);

    Button_SetBackgroundColorRGB(current_button, 240, 240, 240);
    Button_SetBackgroundHoverColorRGB(current_button, 240, 240, 240);
    Button_SetBackgroundPressedColorRGB(current_button, 240, 240, 240);
}

void SetupColors_Wrong(Button *last_button, Button *current_button)
{
    Button_SetBackgroundColorRGB(last_button, 255, 220, 220);
    Button_SetBackgroundHoverColorRGB(last_button, 255, 220, 220);
    Button_SetBackgroundPressedColorRGB(last_button, 255, 220, 220);

    Button_SetBackgroundColorRGB(current_button, 255, 220, 220);
    Button_SetBackgroundHoverColorRGB(current_button, 255, 220, 220);
    Button_SetBackgroundPressedColorRGB(current_button, 255, 220, 220);
}

void SetupColors_WrongAfterTimer(Button *last_button, Button *current_button)
{
    SetupColors_Empty(last_button);
    SetupColors_Empty(current_button);
}
