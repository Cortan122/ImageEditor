#pragma once
#include <raylib.h>

#include "Drawable.h"

typedef enum LineRenderingMode {
  LRM_BEZIER = 0,
  LRM_STRAIGHT,
} LineRenderingMode;

typedef struct DrawableLine {
  // pointer type: stretchy buffer, NULL if empty
  Vector2* line;

  // pointer type: malloced buffer, NULL if dirty
  Vector2* triangleStrip;
  int triangleStripLength;

  float error;      // default value: 10
  float thickness;  // default value: 2
  int subdivisions; // default value: 30     (must be >= 2)
  Color color;      // default value: RED

  LineRenderingMode mode;
} DrawableLine;

void DrawableLine$_init(DrawableLine* dl);
void DrawableLine$add(DrawableLine* dl, Vector2 point);
void DrawableLine$Delete(DrawableLine* dl);
Vector2* DrawableLine$getTriangleStrip(DrawableLine* dl);
void DrawableLine$drawDebug(DrawableLine* dl);
void DrawableLine$Draw(DrawableLine* dl);
bool DrawableLine$Update(DrawableLine* dl);
bool DrawableLine$setOptions(DrawableLine* dl, float error, float thickness, int subdivisions, Color color);
void DrawableLine$setMode(DrawableLine* dl, LineRenderingMode mode);
void DrawableLine$Move(DrawableLine* dl, Vector2 delta);
void DrawableLine$SetColor(DrawableLine* dl, Color color);
Drawable DrawableLine$New(LineRenderingMode mode);
