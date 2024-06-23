#pragma once
#include "Drawable.h"

typedef struct FloatingImage {
  Vector2 pos;
  Texture texture;
  bool hasFirstPoint;
  bool isDone;
  bool nearestNeighborToggle;
  float scale;

  // default value: DARKGREEN
  Color frameColor;
} FloatingImage;

void FloatingImage$Draw(FloatingImage* fi);
bool FloatingImage$Update(FloatingImage* fi);
void FloatingImage$Delete(FloatingImage* fi);
void FloatingImage$Move(FloatingImage* fi, Vector2 delta);
void FloatingImage$SetColor(FloatingImage* fi, Color color);
Drawable FloatingImage$New(char* name);
