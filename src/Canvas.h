#pragma once
#include <raylib.h>

#include "Drawable.h"
#include "Screenshot.h"
#include "stretchy_buffer.h"

typedef struct Canvas {
  float scale;
  Vector2 pos;
  Texture texture;  // alias for screenshot.texture
  Screenshot screenshot;

  float scaleIndex;
  float scrollAccumulator;
  bool nearestNeighborToggle;
  bool scheduledReload;

  Texture transparencyTexture;
  float screenWidth;
  float screenHeight;
  Vector2 prevMousePos;
  Vector2 marginTopLeft;
  Vector2 marginBottomRight;

  Color color;
  Drawable* drawables;  // pointer type: stretchy buffer
  bool isActive;        // todo: rename
  bool isUnmodified;
} Canvas;

void Canvas$recenter(Canvas* c);
void Canvas$rescale(Canvas* c);
bool Canvas$popDrawable(Canvas* c);
void Canvas$updateSize(Canvas* c);
void Canvas$reload(Canvas* c, bool checkClipboard);
void Canvas$zoom(Canvas* c, int delta);
void Canvas$updateDrawable(Canvas* c);
void Canvas$takeScreenshot(Canvas* c);
void Canvas$keyboardShortcuts(Canvas* c);
bool Canvas$copy(Canvas* c, char* name);
void Canvas$addDrawable(Canvas* c, Drawable d);
void Canvas$Update(Canvas* c);
void Canvas$Draw(Canvas* c);
void Canvas$Delete(Canvas* c);
bool Canvas$loadImage(Canvas* c, char* file);
void Canvas$SetColor(Canvas* c, Color color);

Vector2 Canvas$getMousePosition(Canvas* c);
Color Canvas$getColorUnderMouse(Canvas* c, bool* is_some);
