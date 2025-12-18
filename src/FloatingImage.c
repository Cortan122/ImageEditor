#include "FloatingImage.h"

#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include "Drawable.h"
#include "Screenshot.h"
#include "Textbox.h"

Rectangle FloatingImage$getRect(FloatingImage* fi) {
  return (Rectangle){fi->pos.x, fi->pos.y, fi->texture.width * fi->scale, fi->texture.height * fi->scale};
}

void FloatingImage$Draw(FloatingImage* fi) {
  DrawTextureEx(fi->texture, fi->pos, 0, fi->scale, WHITE);

  if (!fi->isDone) {
    DrawRectangleLinesEx(FloatingImage$getRect(fi), 4, fi->frameColor);
  }
}

bool FloatingImage$Update(FloatingImage* fi) {
  if (!fi->hasFirstPoint || IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
    fi->pos = GetMousePosition();
    fi->pos.x = roundf(fi->pos.x);
    fi->pos.y = roundf(fi->pos.y);
    fi->hasFirstPoint = true;
  }

  if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
    Vector2 delta = Vector2Subtract(GetMousePosition(), fi->pos);
    fi->scale = fmaxf(fabsf(delta.x / fi->texture.width), fabsf(delta.y / fi->texture.height));
  }

  if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) {
    fi->isDone = true;
    return false;
  }

  if (IsKeyPressed(KEY_P)) {
    fi->nearestNeighborToggle = !fi->nearestNeighborToggle;
    SetTextureFilter(fi->texture, fi->nearestNeighborToggle ? TEXTURE_FILTER_BILINEAR : TEXTURE_FILTER_POINT);
  }

  return true;
}

void FloatingImage$Delete(FloatingImage* fi) {
  UnloadTexture(fi->texture);
}

void FloatingImage$Move(FloatingImage* fi, Vector2 delta) {
  fi->pos = Vector2Add(fi->pos, delta);
  fi->pos.x = roundf(fi->pos.x);
  fi->pos.y = roundf(fi->pos.y);
}

bool FloatingImage$InRectangle(FloatingImage* fi, Rectangle rect) {
  return CheckCollisionRecs(rect, FloatingImage$getRect(fi));
}

void FloatingImage$SetColor(FloatingImage* fi, Color color) {
  fi->frameColor = color;
}

Drawable FloatingImage$New(char* name) {
  FloatingImage* self = calloc(1, sizeof(FloatingImage));
  self->texture = LoadTexture(name);
  self->frameColor = ORANGE;
  self->scale = 1;
  return DRAWABLE(self, FloatingImage);
}

Drawable FloatingImage$NewFromClipboard() {
  Image img = getImageFromClipboard();
  if (img.data == NULL) {
    // the thing that got pasted is not an image, try pasting as text instead
    return Textbox$NewFromClipboard();
  }

  FloatingImage* self = calloc(1, sizeof(FloatingImage));
  self->texture = LoadTextureFromImage(img);
  UnloadImage(img);

  self->frameColor = ORANGE;
  self->scale = 1;
  return DRAWABLE(self, FloatingImage);
}
