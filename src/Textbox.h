#pragma once
#include <raylib.h>

#include "Drawable.h"

typedef struct Textbox {
  // текст у нас в utf8
  // поэтому важно различать байты и символы
  // символы тут примерно как слова
  // символ начиается с байта, для которого (b&0b11000000) != 0x80
  // PS: это не обычный char* а stretchy_buffer
  char* text;

  // позиция курсора включительная
  // тоесть он "показывает" на первый байт первого символа после него
  int cursorPos;

  // тут есть поля, которые нужны только для рисования Textbox-а
  // некоторые Textbox-ы могут быть невидимыми,
  // но нам особо неважно что мы тратим лишние 72 байта на их хранение
  Vector2 pos;
  // default value: RED
  Color cursorColor;
  // default value: GREEN
  Color textColor;

  // must be >= 1
  // default value: 2
  int fontSize;
  // default value: GetFont(1)
  Font font;
  // default value: true
  bool showCursor;
  // default value: false
  bool thickCursor;
} Textbox;

bool IsKeyTyped(int key);
void Textbox$init(Textbox* textbox);
void Textbox$_del(Textbox* textbox, int n);
char* Textbox$_ins(Textbox* textbox, int n);
void Textbox$moveCursor(Textbox* textbox, bool isbackward, bool isctrl);
void Textbox$backspace(Textbox* textbox, bool isbackward, bool isctrl);
void Textbox$addCharacter(Textbox* textbox, int utf32);
void Textbox$addText(Textbox* textbox, const char* text);
Vector2 Textbox$measureText(Textbox* textbox);
Vector2 Textbox$partialMeasureText(Textbox* textbox);
void Textbox$Draw(Textbox* textbox);
bool Textbox$Update(Textbox* textbox);
void Textbox$Delete(Textbox* textbox);
void Textbox$Move(Textbox* textbox, Vector2 delta);
void Textbox$SetColor(Textbox* textbox, Color color);
Drawable Textbox$New();
