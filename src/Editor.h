#pragma once
#include <raylib.h>

#include "Canvas.h"
#include "Textbox.h"

#define array_length(x) (sizeof(x)/sizeof(*x))

typedef enum UiMode {
  UIMODE_NORMAL,
  UIMODE_COLOR_PALETTE,
  UIMODE_SAVE_AS,
  UIMODE_OPEN,
  UIMODE_HELP,
} UiMode;

typedef enum ClickableZoneName {
  ZONE_COLOR_RECT = 0,
  ZONE_LINE_MODE,

  ZONE_MAX,
  ZONE_NONE = -1,
} ClickableZoneName;


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

  Rectangle clickableRects[ZONE_MAX];
  Texture icons[2];
} Editor;

void Editor$setMode(Editor* ed, UiMode mode);
void Editor$toggleUi(Editor* ed);
bool Editor$open(Editor* ed, char* filename);
void Editor$drawColorPicker(Editor* ed);
void Editor$drawUIBars(Editor* ed);
void Editor$Draw(Editor* ed);
void Editor$Update(Editor* ed);
void Editor$Delete(Editor* ed);
