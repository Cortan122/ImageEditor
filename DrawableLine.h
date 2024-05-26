#pragma once
#include <raylib.h>

#include "Drawable.h"

typedef struct DrawableLine {
  // pointer type: stretchy buffer, NULL if empty
  Vector2* line;

  // pointer type: malloced buffer, NULL if dirty
  Vector2* triangleStrip;
  int triangleStripLength;

  // default value: 10
  float error;
  // default value: 2
  float thickness;

  // must be >= 2
  // default value: 30
  int subdivisions;

  // default value: RED
  Color color;
} DrawableLine;

void DrawableLine$_init(DrawableLine* dl);
void DrawableLine$add(DrawableLine* dl, Vector2 point);
void DrawableLine$Delete(DrawableLine* dl);
Vector2* DrawableLine$getTriangleStrip(DrawableLine* dl);
void DrawableLine$drawDebug(DrawableLine* dl);
void DrawableLine$Draw(DrawableLine* dl);
bool DrawableLine$Update(DrawableLine* dl);
bool DrawableLine$setOptions(DrawableLine* dl, float error, float thickness, int subdivisions, Color color);
void DrawableLine$Move(DrawableLine* dl, Vector2 delta);
void DrawableLine$SetColor(DrawableLine* dl, Color color);
Drawable DrawableLine$New();
