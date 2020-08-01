#ifndef __DG_DISPLAY__
#define __DG_DISPLAY__

#include <wchar.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

int dp_open(char *fb_dev);
void dp_close();

int dp_getScreenInfo();
void dp_ScreenInfo();
int dp_endDraw();
int dp_beginDraw();
int dp_refresh();
int dp_point(int color, int x, int y);
int dp_fillScreen(int color);
int dp_line(int color, int x1, int y1, int x2, int y2);
int dp_rectangle(int color, int x1, int y1, int x2, int y2);
int dp_circle(int color, int x, int y, int r);

// int dp_setFont(char *file, int width, int height);
// int dp_text(int color,int x, int y,wchar_t *text);
FT_Face dp_loadFont(char *fontFile);
void dp_freeFont(FT_Face font);
int dp_text(wchar_t *text, int color, int x, int y, FT_Face font, int fontsize);

// int dp_image(char* file);

int dp_image_bmp(int drawcolor, char *file, int x, int y);

// 绘制进度条
int dp_processbar_LR(int color, int x, int y, int len, float percent);
int dp_processbar_BT(int color, int x, int y, int len, float percent);
#endif