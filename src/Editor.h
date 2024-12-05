#pragma once
#include <raylib.h>

#include "Canvas.h"
#include "Textbox.h"

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

  float colorPickerPopupTimer;
} Editor;

void Editor$setMode(Editor* ed, UiMode mode);
void Editor$toggleUi(Editor* ed);
bool Editor$open(Editor* ed, char* filename);
void Editor$drawColorPicker(Editor* ed);
void Editor$drawUIBars(Editor* ed);
void Editor$Draw(Editor* ed);
void Editor$Update(Editor* ed);
void Editor$Delete(Editor* ed);
