#include <math.h>
#include <raylib.h>
#include <stdio.h>
#include <string.h>

#include "Canvas.h"
#include "Textbox.h"
#include "config.h"

void drawAlignedText(const char* text, int ypos, int fontsize, int aligment, Color c) {
  if (text == NULL) return;

  SetTextLineSpacing(fontsize);
  int measure = MeasureText(text, fontsize);
  int xpos = 0;
  if (aligment == 0) {
    xpos = GetScreenWidth() / 2 - measure / 2;
  } else if (aligment < 0) {
    xpos = GetScreenWidth() - measure + aligment;
  } else if (aligment > 0) {
    xpos = aligment;
  }

  if (ypos < 0) ypos = GetScreenHeight() + ypos - fontsize + 2;

  DrawTextEx(GetFontDefault(), text, (Vector2){xpos, ypos}, fontsize, fontsize / 10, c);
}

typedef struct ColorBlob {
  Color color;
  Rectangle rect;
  int priority;
  char key;
  char* name;
} ColorBlob;

typedef enum UiMode {
  UIMODE_NORMAL,
  UIMODE_COLOR_PALETTE,
  UIMODE_SAVE_AS,
  UIMODE_OPEN,
  UIMODE_HELP,
} UiMode;

typedef struct Editor {
  Canvas canvas;
  char* filename;
  UiMode mode;
  bool isUiHidden;
  bool clickableCursor;
  Textbox inputField;
  bool ioError;
  Rectangle buttonRect;
} Editor;

ColorBlob colorRom[] = {
    {{255, 255, 255, 255}, {0, 0, 0, 0}, 0, 'W', (char[]){"white"}},
    {{245, 245, 245, 255}, {0, 0, 0, 0}, 2, ' ', (char[]){"light\nwhite"}},
    {{200, 200, 200, 255}, {0, 0, 0, 0}, 0, ' ', (char[]){"light\ngray"}},
    {{130, 130, 130, 255}, {0, 0, 0, 0}, 1, ' ', (char[]){"gray"}},
    {{102, 102, 102, 255}, {0, 0, 0, 0}, 1, ' ', (char[]){"evil\ngray"}},
    {{80, 80, 80, 255}, {0, 0, 0, 0}, 0, 'D', (char[]){"dark\ngray"}},
    {{0, 0, 0, 255}, {0, 0, 0, 0}, 0, 'K', (char[]){"black"}},
    {{253, 249, 0, 255}, {0, 0, 0, 0}, 0, 'Y', (char[]){"yellow"}},
    {{255, 203, 0, 255}, {0, 0, 0, 0}, 1, ' ', (char[]){"gold"}},
    {{255, 161, 0, 255}, {0, 0, 0, 0}, 0, 'O', (char[]){"orange"}},
    {{255, 109, 194, 255}, {0, 0, 0, 0}, 0, ' ', (char[]){"pink"}},
    {{230, 41, 55, 255}, {0, 0, 0, 0}, 0, 'R', (char[]){"red"}},
    {{190, 33, 55, 255}, {0, 0, 0, 0}, 1, 'M', (char[]){"maroon"}},
    {{0, 228, 48, 255}, {0, 0, 0, 0}, 0, 'G', (char[]){"green"}},
    {{0, 158, 47, 255}, {0, 0, 0, 0}, 1, 'L', (char[]){"lime"}},
    {{0, 117, 44, 255}, {0, 0, 0, 0}, 0, ' ', (char[]){"dark\ngreen"}},
    {{102, 191, 255, 255}, {0, 0, 0, 0}, 0, 'S', (char[]){"skyblue"}},
    {{0, 121, 241, 255}, {0, 0, 0, 0}, 0, 'B', (char[]){"blue"}},
    {{0, 82, 172, 255}, {0, 0, 0, 0}, 1, ' ', (char[]){"dark\nblue"}},
    {{200, 122, 255, 255}, {0, 0, 0, 0}, 1, 'P', (char[]){"purple"}},
    {{135, 60, 190, 255}, {0, 0, 0, 0}, 0, 'V', (char[]){"violet"}},
    {{112, 31, 126, 255}, {0, 0, 0, 0}, 0, ' ', (char[]){"dark\npurple"}},
    {{211, 176, 131, 255}, {0, 0, 0, 0}, 0, ' ', (char[]){"beige"}},
    {{127, 106, 79, 255}, {0, 0, 0, 0}, 0, ' ', (char[]){"brown"}},
    {{76, 63, 47, 255}, {0, 0, 0, 0}, 1, ' ', (char[]){"dark\nbrown"}},
    {{255, 255, 128, 255}, {0, 0, 0, 0}, 0, ' ', (char[]){"pastel\nyellow"}},
    {{247, 168, 184, 255}, {0, 0, 0, 0}, 0, ' ', (char[]){"pastel\npink"}},
    {{128, 255, 158, 255}, {0, 0, 0, 0}, 0, ' ', (char[]){"pastel\ngreen"}},
    {{85, 205, 252, 255}, {0, 0, 0, 0}, 0, ' ', (char[]){"pastel\nblue"}},
    {{188, 179, 255, 255}, {0, 0, 0, 0}, 0, ' ', (char[]){"pastel\npurple"}},
    {{255, 198, 128, 255}, {0, 0, 0, 0}, 0, ' ', (char[]){"pastel\norange"}},
    {{255, 85, 0, 255}, {0, 0, 0, 0}, 2, ' ', (char[]){"bright\norange"}},
    {{179, 21, 100, 255}, {0, 0, 0, 0}, 2, ' ', (char[]){"dark\npink"}},
    {{162, 230, 27, 255}, {0, 0, 0, 0}, 2, 'C', (char[]){"grass\ngreen"}},
    {{0, 170, 204, 255}, {0, 0, 0, 0}, 2, 'T', (char[]){"teal"}},
    {{2, 220, 164, 255}, {0, 0, 0, 0}, 2, ' ', (char[]){"pale\ngreen"}},
    {{0, 0, 0, 0}, {0, 0, 0, 0}, 0, ' ', NULL},
};

void drawColorBlob(ColorBlob cb) {
  if (cb.rect.height == 0 || cb.rect.width == 0) return;
  Color aspectColor = (cb.color.r == 0 && cb.color.g == 0 && cb.color.b == 0) ? WHITE : BLACK;

  bool isHighlighted = CheckCollisionPointRec(GetMousePosition(), cb.rect);
  DrawRectangleRec(cb.rect, Fade(cb.color, isHighlighted ? 0.6 : 1.0));
  DrawRectangleLinesEx(cb.rect, 6, Fade(aspectColor, 0.3));

  char* line2 = strstr(cb.name, "\n");
  if (line2) *line2 = '\0';
  DrawText(cb.name, cb.rect.x + 10, cb.rect.y + 10, 20, Fade(aspectColor, 0.5));
  if (line2) {
    *line2 = '\n';
    DrawText(line2 + 1, cb.rect.x + 10, cb.rect.y + 30, 20, Fade(aspectColor, 0.5));
  }

  const char* hex = TextFormat("#%02X%02X%02X", cb.color.r, cb.color.g, cb.color.b);
  DrawText(hex, cb.rect.x + 10, cb.rect.y + 80, 10, Fade(aspectColor, 0.5));
}

void layoutColorBlobs(ColorBlob* arr) {
  const int squareSideLength = 100;
  int blobsInRow = (GetScreenWidth() - 20) / (squareSideLength + 10);
  if (blobsInRow > 9) blobsInRow = 9;
  if (blobsInRow == 5) blobsInRow = 4;
  int margin = (GetScreenWidth() - blobsInRow * squareSideLength - (blobsInRow - 1) * 10) / 2;
  int priority = blobsInRow == 9 ? 3 : blobsInRow == 6 ? 1 : blobsInRow >= 7 ? 2 : 0;

  int j = 0;
  for (int i = 0; arr[i].name; i++) {
    arr[i].rect.x = margin + squareSideLength * (j % blobsInRow) + 10 * (j % blobsInRow);
    arr[i].rect.y = 80 + squareSideLength * (j / blobsInRow) + 10 * (j / blobsInRow);
    if (arr[i].priority <= priority && arr[i].rect.y + squareSideLength < GetScreenHeight()) {
      arr[i].rect.height = squareSideLength;
      arr[i].rect.width = squareSideLength;
      j++;
    } else {
      arr[i].rect.height = 0;
      arr[i].rect.width = 0;
    }
  }
}

void drawFramedTextbox(Textbox* textbox) {
  int scrwidth = GetScreenWidth();
  int margin = scrwidth < 600 ? 50 : scrwidth > 1000 ? 200 : 100;
  int width = scrwidth - 2 * margin;
  int height = 50;
  int ypos = (GetScreenHeight() - height) / 2;
  DrawRectangleLinesEx((Rectangle){margin, ypos, width, height}, 4, DARKGRAY);

  int mesure = Textbox$measureText(textbox).x;
  textbox->pos = (Vector2){margin + 15, ypos + 8};
  if (mesure > width - 32) {
    int mesure2 = Textbox$partialMeasureText(textbox).x;
    int pos1 = mesure - (width - 32);
    int pos2 = mesure2 - (width - 32) / 2;
    textbox->pos.x -= fmaxf(0, fminf(pos1, pos2));
  }

  textbox->showCursor = true;
  BeginScissorMode(margin + 13, ypos, width - 26, height);
  Textbox$Draw(textbox);
  EndScissorMode();
}

void Editor$setMode(Editor* ed, UiMode mode) {
  if (ed->mode == mode) return;
  ed->mode = mode;

  if (ed->mode == UIMODE_NORMAL) {
    float prevx = ed->canvas.screenWidth;
    float prevy = ed->canvas.screenHeight;
    Canvas$updateSize(&ed->canvas);
    if (prevx != ed->canvas.screenWidth || prevy != ed->canvas.screenHeight) {
      Canvas$rescale(&ed->canvas);
    }
  } else if (ed->mode == UIMODE_COLOR_PALETTE) {
    layoutColorBlobs(colorRom);
  } else if (ed->mode == UIMODE_SAVE_AS || ed->mode == UIMODE_OPEN) {
    ed->ioError = false;

    Textbox$Delete(&ed->inputField);
    Textbox$init(&ed->inputField);
    ed->inputField.textColor = DARKGRAY;
    ed->inputField.fontSize = 3;
    ed->inputField.thickCursor = true;

    if (ed->filename && ed->mode == UIMODE_SAVE_AS) {
      Textbox$addText(&ed->inputField, ed->filename);
    } else if (ed->mode == UIMODE_SAVE_AS) {
      Textbox$addText(&ed->inputField, getenv("USERPROFILE"));
      Textbox$addText(&ed->inputField, getenv("HOME"));
      Textbox$addText(&ed->inputField, "/Downloads/screenshot.png");
    } else if (ed->mode == UIMODE_OPEN) {
      Textbox$addText(&ed->inputField, GetWorkingDirectory());
    }

    for (int i = 0; i < sb_count(ed->inputField.text); i++) {
      if (ed->inputField.text[i] == '\\') {
        ed->inputField.text[i] = '/';
      }
    }
  }
}

void Editor$toggleUi(Editor* ed) {
  ed->isUiHidden = !ed->isUiHidden;
  ed->canvas.marginTopLeft.y = ed->isUiHidden ? 0 : 20;
  ed->canvas.marginBottomRight.y = ed->isUiHidden ? 0 : 20;
  Canvas$reload(&ed->canvas, false);
}

bool Editor$open(Editor* ed, char* filename) {
  if (ed->canvas.marginTopLeft.y == 0 && !ed->isUiHidden) {
    ed->isUiHidden = !ed->isUiHidden;
    Editor$toggleUi(ed);
  }

  bool res = Canvas$loadImage(&ed->canvas, filename);
  if (res) ed->filename = filename;
  return res;
}

void Editor$Draw(Editor* ed) {
  ClearBackground(RAYWHITE);
  if (ed->mode == UIMODE_NORMAL) {
    Canvas$Draw(&ed->canvas);

    if (ed->canvas.marginTopLeft.y) {
      DrawText(TextFormat("%2i FPS", GetFPS()), 3, 1, 20, DARKGRAY);

      drawAlignedText("color:   ", 1, 20, -3, DARKGRAY);
      Rectangle colorRect = {GetScreenWidth() - 18, 2, 16, 16};
      DrawTexture(ed->canvas.transparencyTexture, colorRect.x, colorRect.y, DARKGRAY);
      DrawRectangleRec(colorRect, ed->canvas.color);

      const char* sizeIndicator =
          TextFormat("%dx%d %.0f%%", ed->canvas.texture.width, ed->canvas.texture.height, ed->canvas.scale * 100);
      drawAlignedText(sizeIndicator, -1, 20, -3, DARKGRAY);

      drawAlignedText(ed->filename ? GetFileName(ed->filename) : "No open file", -1, 20, 3, DARKGRAY);

      drawAlignedText(TextFormat("%dx%d", GetScreenWidth(), GetScreenHeight()), 1, 20, 0, DARKGRAY);

      if (ed->canvas.isActive) {
        drawAlignedText(sb_last(ed->canvas.drawables).name, -1, 20, 0, DARKGRAY);
      } else {
        drawAlignedText(TextFormat("%d things", sb_count(ed->canvas.drawables)), -1, 20, 0, DARKGRAY);
      }
    }
  } else if (ed->mode == UIMODE_COLOR_PALETTE) {
    drawAlignedText("Color palette", 20, 40, 0, DARKGRAY);

    for (int i = 0; colorRom[i].name; i++) {
      drawColorBlob(colorRom[i]);
    }
  } else if (ed->mode == UIMODE_SAVE_AS || ed->mode == UIMODE_OPEN) {
    drawAlignedText(ed->mode == UIMODE_SAVE_AS ? "Save as" : "Open file", 20, 40, 0, DARKGRAY);

    bool isHover = CheckCollisionPointRec(GetMousePosition(), ed->buttonRect);
    drawFramedTextbox(&ed->inputField);
    DrawRectangleRec(ed->buttonRect, isHover ? WHITE : LIGHTGRAY);
    DrawRectangleLinesEx(ed->buttonRect, 4, DARKGRAY);
    drawAlignedText("Done!", ed->buttonRect.y + 8, 40, 0, DARKGRAY);

    if (ed->ioError) {
      drawAlignedText("Can't open file", -20, 40, 0, DARKGRAY);
    }
  } else if (ed->mode == UIMODE_HELP) {
    drawAlignedText("--help", 20, 40, 0, DARKGRAY);
    int fontsize = 4;
    char* text =
        "wasd - pan image\n"
        "left click - draw lines\n"
        "right click - add text\n"
        "ctrl+c/v/s - copy/paste/save\n"
        "x - crop\n"
        "b - box\n"
        "c - color picker\n"
        "p - toggle pixelatedness\n"
        "ctrl+p - take screenshot";
    SetTextLineSpacing(10);  // i have no idea why this works
    Vector2 measure = MeasureTextEx(GetFontDefault(), text, 10 * fontsize, fontsize);
    drawAlignedText(text, -GetScreenHeight() / 2 - measure.y / 2 + 10 * fontsize, 10 * fontsize, 0, DARKGRAY);
  }
}

void Editor$Update(Editor* ed) {
  bool canCheckHelp = true;
  bool oldClickableCursor = ed->clickableCursor;
  ed->clickableCursor = false;
  bool isctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
  bool isshift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
  if (IsKeyPressed(KEY_F2)) Editor$toggleUi(ed);

  if (isctrl && IsKeyPressed(KEY_S)) {
    if (isshift || ed->filename == NULL) {
      Editor$setMode(ed, UIMODE_SAVE_AS);
    } else {
      Canvas$copy(&ed->canvas, ed->filename);
    }
  }

  if (isctrl && IsKeyPressed(KEY_O)) {
    Editor$setMode(ed, UIMODE_OPEN);
  }

  if (ed->mode == UIMODE_NORMAL) {
    Rectangle colorRect = {GetScreenWidth() - 18, 2, 16, 16};
    if (CheckCollisionPointRec(GetMousePosition(), colorRect)) {
      ed->clickableCursor = true;
      SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) Editor$setMode(ed, UIMODE_COLOR_PALETTE);
    }

    if (!isctrl && IsKeyPressed(KEY_C) && !ed->canvas.isActive) Editor$setMode(ed, UIMODE_COLOR_PALETTE);

    if (ed->mode == UIMODE_NORMAL) Canvas$Update(&ed->canvas);
  } else if (ed->mode == UIMODE_COLOR_PALETTE) {
    if (IsWindowResized()) layoutColorBlobs(colorRom);

    if (IsKeyPressed(KEY_ESCAPE)) Editor$setMode(ed, UIMODE_NORMAL);

    for (int i = 0; colorRom[i].name; i++) {
      if (!CheckCollisionPointRec(GetMousePosition(), colorRom[i].rect)) continue;
      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Canvas$SetColor(&ed->canvas, colorRom[i].color);
        Editor$setMode(ed, UIMODE_NORMAL);
      }
      ed->clickableCursor = true;
      SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
      break;
    }

    for (int i = 0; colorRom[i].name; i++) {
      if (colorRom[i].key == ' ') continue;
      if (!IsKeyPressed(colorRom[i].key)) continue;

      Canvas$SetColor(&ed->canvas, colorRom[i].color);
      Editor$setMode(ed, UIMODE_NORMAL);
      break;
    }
  } else if (ed->mode == UIMODE_SAVE_AS || ed->mode == UIMODE_OPEN) {
    if (IsKeyPressed(KEY_ESCAPE)) Editor$setMode(ed, UIMODE_NORMAL);

    ed->buttonRect = (Rectangle){GetScreenWidth() / 2 - 150 / 2, GetScreenHeight() / 2 + 30, 150, 50};
    bool isHover = CheckCollisionPointRec(GetMousePosition(), ed->buttonRect);
    bool isClick = isHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    if (IsKeyPressed(KEY_ENTER) || isClick) {
      bool res = false;
      if (ed->mode == UIMODE_OPEN) res = Editor$open(ed, ed->inputField.text);
      if (ed->mode == UIMODE_SAVE_AS) res = Canvas$copy(&ed->canvas, ed->inputField.text);

      if (res) {
        ed->filename = ed->inputField.text;
        Editor$setMode(ed, UIMODE_NORMAL);
      } else {
        ed->ioError = true;
      }
    }

    Textbox$Update(&ed->inputField);
  } else if (ed->mode == UIMODE_HELP) {
    bool click = IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsMouseButtonPressed(MOUSE_RIGHT_BUTTON);
    bool keys = IsKeyPressed(KEY_F1) || IsKeyPressed(KEY_H);
    int key = GetKeyPressed();
    if (click || key && key != KEY_F1 && key != KEY_H || keys) {
      Editor$setMode(ed, UIMODE_NORMAL);
      canCheckHelp = false;
    }
  }

  if (!ed->canvas.isActive && ed->mode == UIMODE_NORMAL && IsKeyPressed(KEY_H) || IsKeyPressed(KEY_F1)) {
    if (canCheckHelp) Editor$setMode(ed, UIMODE_HELP);
  }

  if (!ed->clickableCursor && oldClickableCursor) {
    SetMouseCursor(MOUSE_CURSOR_DEFAULT);
  }
}

int main(int argc, char** argv) {
  const int screenWidth = DEFAULT_SCREENWIDTH;
  const int screenHeight = DEFAULT_SCREENHEIGHT;
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
  InitWindow(screenWidth, screenHeight, WINDOW_TITLE);
  SetWindowMinSize(MINSIZE_WIDTH, MINSIZE_HEIGHT);
  SetTargetFPS(60);
  SetExitKey(0);
  EnableEventWaiting();  // now the fps counter is scuffed

  Editor theEditor = {0};
  Editor$open(&theEditor, argv[1]);

  int i = 0;
  while (!WindowShouldClose()) {
    BeginDrawing();

    Editor$Update(&theEditor);
    Editor$Draw(&theEditor);

    EndDrawing();
  }
  Canvas$copy(&theEditor.canvas, NULL);
  Canvas$Delete(&theEditor.canvas);

  CloseWindow();
  return 0;
}
