#include "mlog.h"
#include "window.h"
#include "drv_spi_ili9488.h"

int row = MAX_LCD_ROW;
int col = MAX_LCD_COL;
int colWithNull = MAX_LCD_COL + 1;
int curEndLine = MAX_LCD_ROW - 1;
char mFramebuffer[MAX_LCD_ROW][MAX_LCD_COL + 1] = {0};


void increaseLineIndex() {
    curEndLine = (row + curEndLine + 1) % row;
}
int getCurEndLine() {
    return curEndLine;
}
int getCurTopLine() {
    return (row + curEndLine + 1) % row;
}

void addNewLine(char lineStr[]) {
    increaseLineIndex();
    for (int i = 0; i < col; i++)
    {
        char ch = lineStr[i];
        mFramebuffer[curEndLine][i] = ch;
        if (ch == '\0')
        {
            break;
        }
    }
    mFramebuffer[curEndLine][col] = '\0';
}

void freshLine() {
    lcd_fill(0, 0, 480, LOG_TEXT_END, WHITE);
    // lcd_clear(WHITE);
    lcd_set_color(WHITE, BLACK);
    int startLine = getCurTopLine();
    for (int i = 0; i < row; i++) {
        int printLine = (startLine + i) % row;
        char* chp = mFramebuffer[printLine];
        lcd_show_string(10, 15 + i * 24, 24, chp);
    }
}

void addNewLineLn(char lineStr[]) {
    addNewLine(lineStr);
    freshLine();
}
