#include "CropRectangle.h"

#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "config.h"

const Color CropRectangle_color = {0, 0, 0, 102};

Vector2 roundv2(Vector2 v) {
  return (Vector2){roundf(v.x), roundf(v.y)};
}

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

Rectangle CropRectangle$extendRectToEdge(CropRectangle* cr) {
  Rectangle rec = CropRectangle$getSmall(cr);

  int canvas_width = cr->parentCanvas->texture.width;
  int canvas_height = cr->parentCanvas->texture.height;
  double height_ratio = rec.height / canvas_height;
  double width_ratio = rec.width / canvas_width;

  if (height_ratio > width_ratio) {
    rec.y = 0;
    rec.height = canvas_height;
  } else {
    rec.x = 0;
    rec.width = canvas_width;
  }

  return rec;
}

void CropRectangle$selfDestruct(CropRectangle* cr) {
  if (cr->parentCanvas == NULL) return;
  Canvas$popDrawable(cr->parentCanvas);
}

void CropRectangle$hollowCrop(CropRectangle* cr) {
  Rectangle rec = CropRectangle$extendRectToEdge(cr);
  Canvas* c = cr->parentCanvas;
  int canvas_width = c->texture.width;
  int canvas_height = c->texture.height;

  Rectangle first_rec, second_rec, dest_rec;
  int width, height;
  Vector2 delta;

  if (rec.height == canvas_height) {
    int end_x  = rec.x + rec.width;
    first_rec  = (Rectangle){.x = 0, .y = 0, .width = rec.x, .height = canvas_height};
    second_rec = (Rectangle){.x = end_x, .y = 0, .width = canvas_width - end_x, .height = canvas_height};
    dest_rec   = (Rectangle){.x = rec.x, .y = 0, .width = canvas_width - end_x, .height = canvas_height};
    delta  = (Vector2){.x = -rec.width, .y = 0};
    width  = first_rec.width + second_rec.width;
    height = canvas_height;
  } else {
    int end_y  = rec.y + rec.height;
    first_rec  = (Rectangle){.x = 0, .y = 0, .width = canvas_width, .height = rec.y};
    second_rec = (Rectangle){.x = 0, .y = end_y, .width = canvas_width, .height = canvas_height - end_y};
    dest_rec   = (Rectangle){.x = 0, .y = rec.y, .width = canvas_width, .height = canvas_height - end_y};
    delta  = (Vector2){.x = 0, .y = -rec.height};
    width  = canvas_width;
    height = first_rec.height + second_rec.height;
  }

  Image img;
  if (first_rec.width * first_rec.height == 0) {
    img = ImageFromImage(c->screenshot.image, second_rec);
  } else {
    img = ImageFromImage(c->screenshot.image, first_rec);
    ImageResizeCanvas(&img, width, height, 0, 0, MAGENTA);
    ImageDraw(&img, c->screenshot.image, second_rec, dest_rec, WHITE);
  }

  Screenshot$setImage(&c->screenshot, img);
  Canvas$reload(c, false);
  CropRectangle$selfDestruct(cr);

  for (int i = 0; i < sb_count(c->drawables); i++) {
    if (Drawable$InRectangle(c->drawables + i, second_rec)) {
      Drawable$Move(c->drawables + i, delta);
    }
  }
}

void CropRectangle$crop(CropRectangle* cr) {
  if (cr->parentCanvas == NULL) return;
  if (cr->isHollow) {
    CropRectangle$hollowCrop(cr);
    return;
  }

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

  int canvas_width = cr->parentCanvas->texture.width;
  int canvas_height = cr->parentCanvas->texture.height;

  if (cr->hasFirstPoint && !cr->isHollow) {
    Rectangle rec = CropRectangle$getSmall(cr);

    int invx = canvas_width - rec.x - rec.width;
    int invy = canvas_height - rec.y - rec.height;

    DrawRectangle(0, 0, rec.x, canvas_height, CropRectangle_color);
    DrawRectangle(rec.x, 0, rec.width, rec.y, CropRectangle_color);
    DrawRectangle(rec.x, rec.y + rec.height, rec.width, invy, CropRectangle_color);
    DrawRectangle(rec.x + rec.width, 0, invx, canvas_height, CropRectangle_color);

    DrawRectangleLinesEx(CropRectangle$get(cr), 2, cr->frameColor);
  } else if (cr->hasFirstPoint && cr->isHollow) {
    Rectangle rec = CropRectangle$extendRectToEdge(cr);

    DrawRectangleRec(rec, CropRectangle_color);
    DrawRectangleLinesEx(rec, 2, cr->frameColor);
  } else {
    DrawRectangle(0, 0, canvas_width, canvas_height, CropRectangle_color);
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

bool CropRectangle$InRectangle(CropRectangle* cr, Rectangle rect) {
  return CheckCollisionRecs(CropRectangle$get(cr), rect);
}

void CropRectangle$SetColor(CropRectangle* cr, Color color) {
  cr->frameColor = color;
}

Drawable CropRectangle$New(Canvas* c) {
  SetMouseCursor(MOUSE_CURSOR_CROSSHAIR);

  CropRectangle* self = calloc(1, sizeof(CropRectangle));
  self->parentCanvas = c;
  self->frameColor = c ? DEFAULT_CROP_COLOR : DEFAULT_BOX_COLOR;
  Drawable res = DRAWABLE(self, CropRectangle);
  if (c == NULL) res.name = "Box";
  return res;
}
