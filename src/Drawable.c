#include "Drawable.h"

#include <stdbool.h>
#include <stdlib.h>

void Drawable$Draw(Drawable* drawable) {
  if (drawable->self == NULL) return;
  drawable->draw(drawable->self);
}

bool Drawable$Update(Drawable* drawable) {
  if (drawable->self == NULL) return false;
  return drawable->update(drawable->self);
}

void Drawable$Delete(Drawable* drawable) {
  if (drawable->self == NULL) return;
  drawable->delete (drawable->self);
  free(drawable->self);
  drawable->self = NULL;
}

void Drawable$Move(Drawable* drawable, Vector2 delta) {
  if (drawable->self == NULL) return;
  drawable->move(drawable->self, delta);
}

bool Drawable$InRectangle(Drawable* drawable, Rectangle rect) {
  if (drawable->self == NULL) return false;
  return drawable->inRectangle(drawable->self, rect);
}

void Drawable$SetColor(Drawable* drawable, Color color) {
  if (drawable->self == NULL) return;
  if (color.a == 0) return;
  drawable->setColor(drawable->self, color);
}
