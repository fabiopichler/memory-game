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

#include "Header.h"
#include "../base/Texture.h"
#include "../base/Rectangle.h"
#include "../base/Box.h"
#include "GameBoard.h"

#include <malloc.h>
#include <math.h>

struct Header
{
    int space;
    int margin;
    float line_p1_x;
    float line_p2_x;

    SDL_Renderer *renderer;
    SceneGameRect *sceneGameRect;

    Rectangle *background1;
    Rectangle *background2;
    Rectangle *line;

    Player currentPlayer;
    Player gameResult;

    Texture *result;
    Texture *player1;
    Texture *player2;
};

void Header_CreateBackgrounds(Header * const self);
void Header_CreateResultText(Header * const self);
void Header_CreatePlayer1Text(Header * const self);
void Header_CreatePlayer2Text(Header * const self);
void Header_SetupResultText(Header * const self);
void Header_SetupPlayer1Text(Header * const self);
void Header_SetupPlayer2Text(Header * const self);

Header *Header_New(SDL_Renderer *renderer, SceneGameRect *sceneGameRect)
{
    Header * const self = malloc(sizeof (Header));

    self->space = 6;
    self->margin = 20;

    const float w = 110;
    const float x = sceneGameRect->sidebar_w + ((sceneGameRect->content_w - w) / 2);
    self->line_p1_x = x - 75.f;
    self->line_p2_x = x + 75.f;

    self->renderer = renderer;
    self->sceneGameRect = sceneGameRect;
    self->currentPlayer = Player_1;
    self->gameResult = None;

    self->line = Rectangle_New(self->renderer, w, 4.f);
    Box_SetPosition(Rectangle_Box(self->line), self->line_p1_x, 62.f);
    Rectangle_SetColorRGBA(self->line, 80, 150, 220, 255);

    Header_CreateBackgrounds(self);
    Header_CreateResultText(self);
    Header_CreatePlayer1Text(self);
    Header_CreatePlayer2Text(self);

    return self;
}

void Header_Delete(Header * const self)
{
    if (!self)
        return;

    Rectangle_Delete(self->line);
    Rectangle_Delete(self->background1);
    Rectangle_Delete(self->background2);

    Texture_Delete(self->result);
    Texture_Delete(self->player1);
    Texture_Delete(self->player2);

    free(self);
}

void Header_ProcessEvent(Header * const self, const SDL_Event *event)
{
    (void)self;(void)event;
}

void Header_Update(Header * const self, double deltaTime)
{
    Box * const lineBox = Rectangle_Box(self->line);
    const float x = Box_X(lineBox);

    if (self->currentPlayer == Player_1 && x >= self->line_p1_x)
        Box_SetX(lineBox, fmax(x - (800.0 * deltaTime), self->line_p1_x));

    else if (self->currentPlayer == Player_2 && x <= self->line_p2_x)
        Box_SetX(lineBox, fmin(x + (800.0 * deltaTime), self->line_p2_x));
}

void Header_Draw(Header * const self)
{
    if (self->gameResult == None)
    {
        Rectangle_Draw(self->line);

        Texture_Draw(self->player1);
        Texture_Draw(self->player2);
    }
    else
    {
        Rectangle_Draw(self->background1);
        Rectangle_Draw(self->background2);

        Texture_Draw(self->result);
    }
}

void Header_SetCurrentPlayer(Header * const self, Player currentPlayer, Player gameResult)
{
    self->currentPlayer = currentPlayer;
    self->gameResult = gameResult;

    if (self->gameResult == Player_1)
        Texture_SetText(self->result, "Vitória do jogador 1");

    else if (self->gameResult == Player_2)
        Texture_SetText(self->result, "Vitória do jogador 2");

    else if (self->gameResult == Tied)
        Texture_SetText(self->result, "Deu empate!");

    if (self->gameResult != None)
    {
        Texture_MakeText(self->result);
        Header_SetupResultText(self);
    }
}

void Header_CreateResultText(Header * const self)
{
    self->result = Texture_New(self->renderer);

    Texture_SetText(self->result, "...");
    Texture_SetTextSize(self->result, 24);
    Texture_SetTextColorRGB(self->result, 80, 140, 200);
    Texture_MakeText(self->result);

    Header_SetupResultText(self);
}

void Header_CreateBackgrounds(Header * const self)
{
    int w = 302, h = 58;
    int x = self->sceneGameRect->sidebar_w + ((self->sceneGameRect->content_w - w) / 2);
    int y = 18;

    self->background1 = Rectangle_New(self->renderer, w, h);
    Box_SetPosition(Rectangle_Box(self->background1), x, y);
    Rectangle_SetColorRGBA(self->background1, 80, 150, 220, 255);

    w -= 2; h -= 2; x -= 2; y -= 2;

    self->background2 = Rectangle_New(self->renderer, w, h);
    Box_SetPosition(Rectangle_Box(self->background2), x, y);
    Rectangle_SetColorRGBA(self->background2, 220, 240, 255, 255);
}

void Header_CreatePlayer1Text(Header * const self)
{
    self->player1 = Texture_New(self->renderer);

    Texture_SetText(self->player1, "Jogador 1");
    Texture_SetTextSize(self->player1, 20);
    Texture_SetTextColorRGB(self->player1, 120, 120, 120);
    Texture_MakeText(self->player1);

    Header_SetupPlayer1Text(self);
}

void Header_CreatePlayer2Text(Header * const self)
{
    self->player2 = Texture_New(self->renderer);

    Texture_SetText(self->player2, "Jogador 2");
    Texture_SetTextSize(self->player2, 20);
    Texture_SetTextColorRGB(self->player2, 120, 120, 120);
    Texture_MakeText(self->player2);

    Header_SetupPlayer2Text(self);
}

void Header_SetupResultText(Header * const self)
{
    int w = Texture_GetWidth(self->result);
    int h = Texture_GetHeight(self->result);

    Box_SetSize(Texture_Box(self->result), w, h);
    Box_SetPosition(Texture_Box(self->result), self->sceneGameRect->sidebar_w + ((self->sceneGameRect->content_w - w) / 2), 26);
}

void Header_SetupPlayer1Text(Header * const self)
{
    int text_w = Texture_GetWidth(self->player1);
    int text_h = Texture_GetHeight(self->player1);

    int text_x = (self->sceneGameRect->sidebar_w + ((self->sceneGameRect->content_w - text_w) / 2)) - (text_w / 2) - self->margin - self->space;

    Box_SetSize(Texture_Box(self->player1), text_w, text_h);
    Box_SetPosition(Texture_Box(self->player1), text_x, 30);
}

void Header_SetupPlayer2Text(Header * const self)
{
    int text_w = Texture_GetWidth(self->player2);
    int text_h = Texture_GetHeight(self->player2);

    int text_x = (self->sceneGameRect->sidebar_w + ((self->sceneGameRect->content_w - text_w) / 2)) + (text_w / 2) + self->margin + self->space;

    Box_SetSize(Texture_Box(self->player2), text_w, text_h);
    Box_SetPosition(Texture_Box(self->player2), text_x, 30);
}
