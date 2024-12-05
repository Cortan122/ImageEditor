#include "Textbox.h"

#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "resource_loader.h"
#include "stretchy_buffer.h"

double keyTypingRom[400];
const double keyRepeatDelay = .500;
const double keyRepeatRate = 1. / 31;

bool IsKeyTyped(int key) {
  // IsKeyDown on wsl always returns 1 for some reason
  // or is it...?
  // then if IsKeyDown() == 0 we know for sure the key isn't down (?)
  if (!IsKeyDown(key)) keyTypingRom[key] = 0;

  if (IsKeyPressed(key)) {
    keyTypingRom[key] = keyRepeatDelay;
    return true;
  } else if (IsKeyReleased(key)) {
    keyTypingRom[key] = 0;
  } else if (keyTypingRom[key]) {
    keyTypingRom[key] -= GetFrameTime();
    if (keyTypingRom[key] <= 0) {
      keyTypingRom[key] += keyRepeatRate;
      return true;
    }
  }

  return false;
}

void Textbox$init(Textbox* textbox) {
  sb_push(textbox->text, '\0');
  textbox->cursorPos = 0;
  textbox->pos = (Vector2){0, 0};

  textbox->cursorColor = DEFAULT_CURSOR_COLOR;
  textbox->textColor = DEFAULT_TEXT_COLOR;
  textbox->font = GetFont(1);
  textbox->fontSize = 2;
  textbox->showCursor = true;
  textbox->thickCursor = false;
}

// удаляет n байтов после курсора
void Textbox$_del(Textbox* textbox, int n) {
  int len = sb_count(textbox->text) - textbox->cursorPos;
  char* ptr = textbox->text + textbox->cursorPos;
  memmove(ptr, ptr + n, len - n);
  sb_add(textbox->text, -n);
}

// добавляет n байтов перед курсором и возвращает указатель на первый из них
char* Textbox$_ins(Textbox* textbox, int n) {
  int len = sb_count(textbox->text) - textbox->cursorPos;
  sb_add(textbox->text, n);
  char* ptr = textbox->text + textbox->cursorPos;
  memmove(ptr + n, ptr, len);

  // тут ещё может надо их както инициализировать
  memset(ptr, '?', n);  // todo: удалить эту строчку

  // или его менять ненадо
  textbox->cursorPos += n;
  return ptr;
}

void Textbox$moveCursor(Textbox* textbox, bool isbackward, bool isctrl) {
  int len = sb_count(textbox->text) - 1;
  int delta = isbackward ? -1 : 1;
  int i = textbox->cursorPos;

  while (isbackward ? (i != 0) : (i != len)) {
    i += delta;
    char byte = textbox->text[i];
    if ((byte & 0b11000000) != 0x80) {
      if (!isctrl) break;
      if (byte == ' ' || byte == '/' || byte == '.')
        break;  // todo: how the fuck does `ctrl + arrow keys` work exactly
    }
  }

  textbox->cursorPos = i;
}

void Textbox$backspace(Textbox* textbox, bool isbackward, bool isctrl) {
  int cp1 = textbox->cursorPos;
  Textbox$moveCursor(textbox, isbackward, isctrl);
  int cp2 = textbox->cursorPos;
  textbox->cursorPos = cp2 > cp1 ? cp1 : cp2;
  Textbox$_del(textbox, abs(cp1 - cp2));
}

void Textbox$addCharacter(Textbox* textbox, int utf32) {
  int len = 0;
  const char* utf8 = CodepointToUTF8(utf32, &len);
  memcpy(Textbox$_ins(textbox, len), utf8, len);
}

void Textbox$addText(Textbox* textbox, const char* text) {
  if (text == NULL) return;
  int len = strlen(text);
  memcpy(Textbox$_ins(textbox, len), text, len);
}

Vector2 Textbox$measureText(Textbox* textbox) {
  return MeasureTextEx(textbox->font, textbox->text, textbox->font.baseSize * textbox->fontSize,
                       textbox->fontSize);
}

Vector2 Textbox$partialMeasureText(Textbox* textbox) {
  char temp = textbox->text[textbox->cursorPos];
  textbox->text[textbox->cursorPos] = '\0';
  Vector2 res = Textbox$measureText(textbox);
  textbox->text[textbox->cursorPos] = temp;
  return res;
}

void Textbox$Draw(Textbox* textbox) {
  Vector2 pos = textbox->pos;
  Font font = textbox->font;
  int fontScale = textbox->fontSize;

  char temp = textbox->text[textbox->cursorPos];
  textbox->text[textbox->cursorPos] = '\0';
  Vector2 res = DrawTextScaled(font, textbox->text, pos, fontScale, textbox->textColor);
  textbox->text[textbox->cursorPos] = temp;

  if (textbox->showCursor) {
    int cursorWidth = fontScale;
    double prevDiff = fabs(fontScale / (double)cursorWidth - 2);
    while (!textbox->thickCursor) {
      cursorWidth -= 2;
      double newDiff = fabs(fontScale / (double)cursorWidth - 2);
      if (newDiff > prevDiff) {
        cursorWidth += 2;
        break;
      }
    }
    int delta = (fontScale - cursorWidth) / 2;  // тут всё должно делится парвильно
    res.x += delta;
    DrawRectangleV(res, (Vector2){cursorWidth, font.recs[0].height * fontScale}, textbox->cursorColor);
    res.x += fontScale - delta;
  } else {
    res.x += fontScale;
  }

  DrawTextScaled(font, textbox->text + textbox->cursorPos, res, fontScale, textbox->textColor);
}

bool Textbox$Update(Textbox* textbox) {
  int utf32;
  while (utf32 = GetCharPressed()) {
    Textbox$addCharacter(textbox, utf32);
  }

  bool isctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
  if (IsKeyTyped(KEY_LEFT)) Textbox$moveCursor(textbox, true, isctrl);
  if (IsKeyTyped(KEY_RIGHT)) Textbox$moveCursor(textbox, false, isctrl);
  if (IsKeyTyped(KEY_BACKSPACE)) Textbox$backspace(textbox, true, isctrl);
  if (IsKeyTyped(KEY_DELETE)) Textbox$backspace(textbox, false, isctrl);

  if (IsKeyTyped(KEY_V) && isctrl) {
    const char* str = GetClipboardText();
    // todo: handle \n and \r
    if (str != NULL) Textbox$addText(textbox, str);
  }

  if (isctrl) {
    if (IsKeyTyped(KEY_EQUAL) || IsKeyTyped(KEY_KP_ADD)) textbox->fontSize++;
    if (textbox->fontSize > 1 && (IsKeyTyped(KEY_MINUS) || IsKeyTyped(KEY_KP_SUBTRACT))) textbox->fontSize--;
  }

  if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    textbox->showCursor = false;
    return false;
  }

  if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
    textbox->pos = GetMousePosition();
    textbox->pos.x = roundf(textbox->pos.x);
    textbox->pos.y = roundf(textbox->pos.y);
  }

  if (IsKeyPressed(KEY_HOME)) textbox->cursorPos = 0;
  if (IsKeyPressed(KEY_END)) textbox->cursorPos = sb_count(textbox->text) - 1;

  return true;
}

void Textbox$Delete(Textbox* textbox) {
  sb_free(textbox->text);
  textbox->text = NULL;
}

void Textbox$Move(Textbox* textbox, Vector2 delta) {
  textbox->pos = Vector2Add(textbox->pos, delta);
  textbox->pos.x = roundf(textbox->pos.x);
  textbox->pos.y = roundf(textbox->pos.y);
}

void Textbox$SetColor(Textbox* textbox, Color color) {
  textbox->textColor = color;
}

Drawable Textbox$New() {
  Textbox* self = calloc(1, sizeof(Textbox));
  Textbox$init(self);
  return DRAWABLE(self, Textbox);
}
