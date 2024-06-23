#pragma once
#include <raylib.h>

#define DRAWABLE(self, type)                                                              \
  (Drawable) {                                                                            \
    self, #type, (UpdateMethod)type##$Update, (Method)type##$Draw, (Method)type##$Delete, \
        (MoveMethod)type##$Move, (ColorMethod)type##$SetColor,                            \
  }

typedef void (*Method)(void*);
typedef void (*MoveMethod)(void*, Vector2);
typedef void (*ColorMethod)(void*, Color);
typedef bool (*UpdateMethod)(void*);

typedef struct Drawable {
  void* self;
  char* name;
  UpdateMethod update;
  Method draw;
  Method delete;
  MoveMethod move;
  ColorMethod setColor;
} Drawable;

bool Drawable$Update(Drawable* drawable);
void Drawable$Draw(Drawable* drawable);
void Drawable$Delete(Drawable* drawable);
void Drawable$Move(Drawable* drawable, Vector2 delta);
void Drawable$SetColor(Drawable* drawable, Color color);
