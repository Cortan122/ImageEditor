#pragma once
#include "Canvas.h"
#include "Drawable.h"

typedef struct CropRectangle {
  Canvas* parentCanvas;
  Vector2 point1;
  Vector2 point2;
  bool hasFirstPoint;
  bool isDone;
  bool isHollow;

  // default value: DARKGREEN
  Color frameColor;
} CropRectangle;

Rectangle CropRectangle$get(CropRectangle* cr);
Rectangle CropRectangle$getSmall(CropRectangle* cr);
void CropRectangle$selfDestruct(CropRectangle* cr);
void CropRectangle$crop(CropRectangle* cr);
void CropRectangle$Draw(CropRectangle* cr);
bool CropRectangle$Update(CropRectangle* cr);
void CropRectangle$Delete(CropRectangle* cr);
void CropRectangle$Move(CropRectangle* cr, Vector2 delta);
void CropRectangle$SetColor(CropRectangle* cr, Color color);
Drawable CropRectangle$New(Canvas* c);
