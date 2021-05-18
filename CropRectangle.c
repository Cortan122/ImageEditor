#include "CropRectangle.h"

#include <stdlib.h>

const Color CropRectangle_color = {0, 0, 0, 102};

Vector2 roundv2(Vector2 v) { return (Vector2){roundf(v.x), roundf(v.y)}; }

Rectangle CropRectangle$get(CropRectangle* cr) {
  Rectangle res = {
      .x = fminf(cr->point1.x, cr->point2.x),
      .y = fminf(cr->point1.y, cr->point2.y),
  };
  res.width = fmaxf(cr->point1.x, cr->point2.x) - res.x;
  res.height = fmaxf(cr->point1.y, cr->point2.y) - res.y;
  return res;
}

Rectangle CropRectangle$getSmall(CropRectangle* cr) {
  Rectangle rec = CropRectangle$get(cr);

  if (rec.x < 0) {
    rec.width += rec.x;
    rec.x = 0;
  }
  if (rec.y < 0) {
    rec.height += rec.y;
    rec.y = 0;
  }
  rec.width = fmaxf(0, rec.width);
  rec.height = fmaxf(0, rec.height);

  int maxx = cr->parentCanvas->screenshot.image.width;
  int maxy = cr->parentCanvas->screenshot.image.height;
  rec.x = fminf(rec.x, maxx);
  rec.y = fminf(rec.y, maxy);
  if (rec.width + rec.x > maxx) rec.width = maxx - rec.x;
  if (rec.height + rec.y > maxy) rec.height = maxy - rec.y;

  return rec;
}

void CropRectangle$selfDestruct(CropRectangle* cr) {
  if (cr->parentCanvas == NULL) return;
  Canvas$popDrawable(cr->parentCanvas);
}

void CropRectangle$crop(CropRectangle* cr) {
  if (cr->parentCanvas == NULL) return;
  // todo: maybe allow extending and fill with WHITE (ImageResizeCanvas)
  Rectangle rec = CropRectangle$getSmall(cr);
  Canvas* c = cr->parentCanvas;  // we cant use the CropRectangle pointer after
                                 // self-destructing

  Image img = ImageFromImage(c->screenshot.image, rec);
  Screenshot$setImage(&c->screenshot, img);
  Canvas$reload(c, false);
  CropRectangle$selfDestruct(cr);

  Vector2 delta = {-rec.x, -rec.y};
  for (int i = 0; i < sb_count(c->drawables); i++) {
    Drawable$Move(c->drawables + i, delta);
  }
}

void CropRectangle$Draw(CropRectangle* cr) {
  if (cr->parentCanvas == NULL) {
    if (cr->isHollow) {
      DrawRectangleLinesEx(CropRectangle$get(cr), 4, cr->frameColor);
    } else {
      DrawRectangleRec(CropRectangle$get(cr), cr->frameColor);
      if (!cr->isDone) {
        DrawRectangleLinesEx(CropRectangle$get(cr), 4, Fade(BLACK, 0.3));
      }
    }
    return;
  }

  if (cr->hasFirstPoint) {
    Rectangle rec = CropRectangle$getSmall(cr);
    int invx = cr->parentCanvas->texture.width - rec.x - rec.width;
    int invy = cr->parentCanvas->texture.height - rec.y - rec.height;

    DrawRectangle(0, 0, rec.x, cr->parentCanvas->texture.height,
                  CropRectangle_color);
    DrawRectangle(rec.x, 0, rec.width, rec.y, CropRectangle_color);
    DrawRectangle(rec.x, rec.y + rec.height, rec.width, invy,
                  CropRectangle_color);
    DrawRectangle(rec.x + rec.width, 0, invx, cr->parentCanvas->texture.height,
                  CropRectangle_color);

    DrawRectangleLinesEx(CropRectangle$get(cr), 2, cr->frameColor);
  } else {
    DrawRectangle(0, 0, cr->parentCanvas->texture.width,
                  cr->parentCanvas->texture.height, CropRectangle_color);
  }
}

bool CropRectangle$Update(CropRectangle* cr) {
  if (cr->hasFirstPoint && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
    CropRectangle$Delete(cr);
    CropRectangle$crop(cr);
    return false;
  }

  if (IsKeyPressed(KEY_ESCAPE)) {
    CropRectangle$Delete(cr);
    CropRectangle$selfDestruct(cr);
    return false;
  }

  if (!cr->hasFirstPoint && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    cr->point1 = roundv2(GetMousePosition());
    cr->hasFirstPoint = true;
  }

  if (cr->hasFirstPoint) cr->point2 = roundv2(GetMousePosition());

  if (IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT)) {
    cr->isHollow = !cr->isHollow;
  }

  return true;
}

void CropRectangle$Delete(CropRectangle* cr) {
  cr->isDone = true;
  SetMouseCursor(MOUSE_CURSOR_DEFAULT);
}

void CropRectangle$Move(CropRectangle* cr, Vector2 delta) {
  cr->point1 = Vector2Add(cr->point1, delta);
  cr->point2 = Vector2Add(cr->point2, delta);
}

void CropRectangle$SetColor(CropRectangle* cr, Color color) {
  cr->frameColor = color;
}

Drawable CropRectangle$New(Canvas* c) {
  SetMouseCursor(MOUSE_CURSOR_CROSSHAIR);

  CropRectangle* self = calloc(1, sizeof(CropRectangle));
  self->parentCanvas = c;
  self->frameColor = c ? DARKGREEN : SKYBLUE;
  Drawable res = DRAWABLE(self, CropRectangle);
  if (c == NULL) res.name = "Box";
  return res;
}
