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

#include "Sidebar.h"
#include "../base/Texture.h"
#include "../base/Rectangle.h"
#include "../base/Box.h"

#include <malloc.h>
#include <stdio.h>

struct Sidebar
{
    SDL_Renderer *renderer;
    const SceneGameRect *sceneGameRect;
    int width;
    int player1_y;
    int player1Win_y;
    int player2_y;
    int player2Win_y;
    int tied_y;
    int tiedCount_y;

    Rectangle *background;
    Rectangle *verticalLine;
    Rectangle *horizontalLine1;
    Rectangle *horizontalLine2;
    SDL_Color textColor;

    Texture *player1Text;
    Texture *player1WinText;
    Texture *player2Text;
    Texture *player2WinText;
    Texture *tiedText;
    Texture *tiedCountText;
};

void Sidebar_SetupSizes(Sidebar * const self);
void Sidebar_CreateTextures(Sidebar * const self);
void Sidebar_UpdateTextRect(Sidebar * const self, Texture *texture, int y);
void Sidebar_UpdateText(Sidebar * const self, Texture *texture, int pos_y, int count);

Sidebar *Sidebar_New(SDL_Renderer *renderer, SceneGameRect *sceneGameRect)
{
    Sidebar * const self = malloc(sizeof (Sidebar));

    self->renderer = renderer;
    self->sceneGameRect = sceneGameRect;
    self->textColor = (SDL_Color) {120, 120, 120, 255};

    Sidebar_SetupSizes(self);
    Sidebar_CreateTextures(self);

    return self;
}

void Sidebar_Delete(Sidebar * const self)
{
    if (!self)
        return;

    Texture_Delete(self->player1Text);
    Texture_Delete(self->player1WinText);
    Texture_Delete(self->player2Text);
    Texture_Delete(self->player2WinText);
    Texture_Delete(self->tiedText);
    Texture_Delete(self->tiedCountText);

    free(self);
}

void Sidebar_Draw(Sidebar * const self)
{
    Rectangle_Draw(self->background);
    Rectangle_Draw(self->verticalLine);
    Rectangle_Draw(self->horizontalLine1);
    Rectangle_Draw(self->horizontalLine2);

    Texture_Draw(self->player1Text);
    Texture_Draw(self->player1WinText);
    Texture_Draw(self->player2Text);
    Texture_Draw(self->player2WinText);
    Texture_Draw(self->tiedText);
    Texture_Draw(self->tiedCountText);
}

void Sidebar_SetPlayer1WinText(Sidebar * const self, int count)
{
    Sidebar_UpdateText(self, self->player1WinText, self->player1Win_y, count);
}

void Sidebar_SetPlayer2WinText(Sidebar * const self, int count)
{
    Sidebar_UpdateText(self, self->player2WinText, self->player2Win_y, count);
}

void Sidebar_SetTiedCountText(Sidebar * const self, int count)
{
    Sidebar_UpdateText(self, self->tiedCountText, self->tiedCount_y, count);
}

void Sidebar_SetupSizes(Sidebar * const self)
{
    const int border_w = 2;
    const int title_margin = 30;
    const int number_margin = 95;
    const int second_block = self->sceneGameRect->sidebar_h / 3;
    const int third_block = second_block * 2;

    self->width = self->sceneGameRect->sidebar_w - border_w;

    self->player1_y = title_margin;
    self->player1Win_y = number_margin;
    self->player2_y = second_block + title_margin;
    self->player2Win_y = second_block + number_margin;
    self->tied_y = third_block + title_margin;
    self->tiedCount_y = third_block + number_margin;

    self->background = Rectangle_New(self->renderer, self->width, self->sceneGameRect->sidebar_h);
    self->verticalLine = Rectangle_New(self->renderer, border_w, self->sceneGameRect->sidebar_h);
    self->horizontalLine1 = Rectangle_New(self->renderer, self->width, border_w);
    self->horizontalLine2 = Rectangle_New(self->renderer, self->width, border_w);

    Rectangle_SetColorRGBA(self->background, 240, 240, 240, 255);
    Rectangle_SetColorRGBA(self->verticalLine, 180, 180, 180, 255);
    Rectangle_SetColorRGBA(self->horizontalLine1, 180, 180, 180, 255);
    Rectangle_SetColorRGBA(self->horizontalLine2, 180, 180, 180, 255);

    Box_SetPosition(Rectangle_Box(self->verticalLine), self->width, 0);
    Box_SetPosition(Rectangle_Box(self->horizontalLine1), 0, second_block - border_w);
    Box_SetPosition(Rectangle_Box(self->horizontalLine2), 0, third_block - border_w);
}

void Sidebar_CreateTextures(Sidebar * const self)
{
    self->player1Text = Texture_New(self->renderer);
    self->player1WinText = Texture_New(self->renderer);
    self->player2Text = Texture_New(self->renderer);
    self->player2WinText = Texture_New(self->renderer);
    self->tiedText = Texture_New(self->renderer);
    self->tiedCountText = Texture_New(self->renderer);

    Texture_SetText(self->player1Text, "Vitórias do jogador 1");
    Texture_SetText(self->player1WinText, "0");
    Texture_SetText(self->player2Text, "Vitórias do jogador 2");
    Texture_SetText(self->player2WinText, "0");
    Texture_SetText(self->tiedText, "Total de empates");
    Texture_SetText(self->tiedCountText, "0");

    Texture_SetTextSize(self->player1Text, 16);
    Texture_SetTextSize(self->player1WinText, 40);
    Texture_SetTextSize(self->player2Text, 16);
    Texture_SetTextSize(self->player2WinText, 40);
    Texture_SetTextSize(self->tiedText, 16);
    Texture_SetTextSize(self->tiedCountText, 40);

    Texture_SetTextColor(self->player1Text, &self->textColor);
    Texture_SetTextColor(self->player1WinText, &self->textColor);
    Texture_SetTextColor(self->player2Text, &self->textColor);
    Texture_SetTextColor(self->player2WinText, &self->textColor);
    Texture_SetTextColor(self->tiedText, &self->textColor);
    Texture_SetTextColor(self->tiedCountText, &self->textColor);

    Texture_MakeText(self->player1Text);
    Texture_MakeText(self->player1WinText);
    Texture_MakeText(self->player2Text);
    Texture_MakeText(self->player2WinText);
    Texture_MakeText(self->tiedText);
    Texture_MakeText(self->tiedCountText);

    Sidebar_UpdateTextRect(self, self->player1Text, self->player1_y);
    Sidebar_UpdateTextRect(self, self->player1WinText, self->player1Win_y);
    Sidebar_UpdateTextRect(self, self->player2Text, self->player2_y);
    Sidebar_UpdateTextRect(self, self->player2WinText, self->player2Win_y);
    Sidebar_UpdateTextRect(self, self->tiedText, self->tied_y);
    Sidebar_UpdateTextRect(self, self->tiedCountText, self->tiedCount_y);
}

void Sidebar_UpdateTextRect(Sidebar * const self, Texture *texture, int y)
{
    (void)self;

    int w = Texture_GetWidth(texture);
    int h = Texture_GetHeight(texture);

    Box_SetSize(Texture_Box(texture), w, h);
    Box_SetPosition(Texture_Box(texture), (self->width - w) / 2, y);
}

void Sidebar_UpdateText(Sidebar * const self, Texture *texture, int pos_y, int count)
{
    char text[6];

    snprintf(text, sizeof (text), "%d", count);
    Texture_SetText(texture, text);
    Texture_MakeText(texture);
    Sidebar_UpdateTextRect(self, texture, pos_y);
}
