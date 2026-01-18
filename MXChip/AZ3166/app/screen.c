/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "screen.h"

void screen_print(char* str, LINE_NUM line)
{
    ssd1306_SetCursor(2, line);
    ssd1306_WriteString(str, Font_11x18, White);
    ssd1306_UpdateScreen();
}

void screen_clear()
{
    ssd1306_Fill(Black);
    ssd1306_UpdateScreen();
}

void screen_printl(char* str, char* str1, LINE_NUM line, LINE_NUM line1)
{
    ssd1306_SetCursor(11, line);
    ssd1306_WriteString(str, Font_11x18, White);
    ssd1306_SetCursor(2, line1);
    ssd1306_WriteString(str1, Font_11x18, White);
    ssd1306_UpdateScreen();
}

void screen_printn(const char* str, unsigned int str_length, LINE_NUM line)
{
    ssd1306_SetCursor(2, line);

    for (unsigned int i = 0; i < str_length; ++i)
    {
        if (ssd1306_WriteChar(str[i], Font_11x18, White) != str[i])
        {
            return;
        }
    }

    ssd1306_UpdateScreen();
}

void draw_youtube_logo(LINE_NUM line0, LINE_NUM line1)
{
    // Define logo dimensions
    const uint8_t logo_width  = 60;
    const uint8_t logo_height = 32;

    // Calculate the x and y positions for centering the logo
    uint8_t x = (128 - logo_width) / 2; // 128 is the width of the SSD1306 display
    uint8_t y = (line0 + line1 - logo_height) / 2;

    // Draw the filled rectangle representing the YouTube logo
    for (uint8_t i = y; i < y + logo_height; i++)
    {
        for (uint8_t j = x; j < x + logo_width; j++)
        {
            ssd1306_DrawPixel(j, i, White);
        }
    }

    ssd1306_DrawRectangle(x, y, x + logo_width, y + logo_height, White);

    // Draw the play button (triangle)
    ssd1306_Line(x + 20, y + 8, x + 20, y + 24, Black);
    ssd1306_Line(x + 20, y + 8, x + 40, y + 16, Black);
    ssd1306_Line(x + 40, y + 16, x + 20, y + 24, Black);

    ssd1306_UpdateScreen();
}

void screen_draw_bitmap(const unsigned char* bitmap)
{
    ssd1306_Fill(Black);
    for (int y = 0; y < 64; y++)
    {
        for (int x = 0; x < 128; x++)
        {
            int byte_idx = y * 16 + (x / 8);
            int bit_idx  = 7 - (x % 8);
            if (bitmap[byte_idx] & (1 << bit_idx))
            {
                ssd1306_DrawPixel(x, y, White);
            }
        }
    }
    ssd1306_UpdateScreen();
}