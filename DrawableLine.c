#include "DrawableLine.h"

#include <raymath.h>
#include <string.h>

#include "config.h"
#include "BezierCurve.h"

#define nfree(x) \
  do {           \
    free(x);     \
    (x) = NULL;  \
  } while (0)
#define sb_nfree(x) \
  do {              \
    sb_free(x);     \
    (x) = NULL;     \
  } while (0)

void DrawableLine$_init(DrawableLine* dl) {
  if (dl->subdivisions) return;
  dl->error = 10;
  dl->thickness = 2;
  dl->subdivisions = 30;
  dl->color = DEFAULT_LINE_COLOR;
}

void DrawableLine$add(DrawableLine* dl, Vector2 point) {
  int len = sb_count(dl->line);
  if (!len || dl->line[len - 1].x != point.x || dl->line[len - 1].y != point.y) {
    sb_push(dl->line, point);
    nfree(dl->triangleStrip);
  }
}

// actually identical to DrawableLine$reset
void DrawableLine$Delete(DrawableLine* dl) {
  sb_nfree(dl->line);
  nfree(dl->triangleStrip);
}

Vector2 Vector2Normal(Vector2 a, Vector2 b) {
  Vector2 norm = Vector2Normalize(Vector2Subtract(b, a));
  return (Vector2){-norm.y, norm.x};
}

Vector2* trigonizeLine(Vector2* line, int len, float thickness) {
  Vector2* res = calloc(len * 2, sizeof(Vector2));

  for (int i = 0; i < len; i++) {
    Vector2 miter;
    if (i == 0) {
      miter = Vector2Normal(line[i], line[i + 1]);
    } else if (i == len - 1) {
      miter = Vector2Normal(line[i - 1], line[i]);
    } else {
      Vector2 norm1 = Vector2Normal(line[i - 1], line[i]);
      Vector2 norm2 = Vector2Normal(line[i], line[i + 1]);
      miter = Vector2Normalize(Vector2Add(norm1, norm2));

      float scale = 1 / Vector2DotProduct(norm1, miter);
      if (scale > 10) scale = 10;  // todo?: bevel
      miter = Vector2Scale(miter, scale);
    }
    miter = Vector2Scale(miter, thickness);
    res[2 * i + 1] = Vector2Add(line[i], miter);
    res[2 * i] = Vector2Subtract(line[i], miter);
  }

  return res;
}

Vector2* DrawableLine$getTriangleStrip(DrawableLine* dl) {
  if (dl->triangleStrip) return dl->triangleStrip;
  DrawableLine$_init(dl);

  BezierPath bez = FitCurve((Vector2d*)dl->line, sb_count(dl->line), dl->error);
  Vector2* bezline = calloc(dl->subdivisions * sb_count(bez), sizeof(Vector2));
  int bezi = 0;
  for (int i = 0; i < sb_count(bez); i++) {
    for (int j = 0; j < dl->subdivisions; j++) {
      Vector2d v2d = Bezier(3, bez[i], j / (dl->subdivisions - 1.));
      if (!bezi || bezline[bezi - 1].x != v2d.x || bezline[bezi - 1].y != v2d.y) {
        bezline[bezi++] = (Vector2){v2d.x, v2d.y};
      }
    }
  }
  sb_free(bez);

  dl->triangleStrip = trigonizeLine(bezline, bezi, dl->thickness);
  dl->triangleStripLength = bezi * 2;
  free(bezline);

  return dl->triangleStrip;
}

void DrawableLine$drawDebug(DrawableLine* dl) {
  DrawFPS(0, 0);
  DrawText(TextFormat("%.2f error", dl->error), 0, 20, 20, LIME);
  DrawText(TextFormat("%.2f thickness", dl->thickness), 0, 40, 20, LIME);
  DrawText(TextFormat("%d subdivisions", dl->subdivisions), 0, 60, 20, LIME);

  DrawText(TextFormat("   %.2f hue", ColorToHSV(dl->color).x), 0, 80, 20, LIME);
  DrawRectangle(2, 82, 16, 16, dl->color);
}

void DrawableLine$Draw(DrawableLine* dl) {
  DrawableLine$getTriangleStrip(dl);
  DrawTriangleStrip(dl->triangleStrip, dl->triangleStripLength, dl->color);
}

bool DrawableLine$Update(DrawableLine* dl) {
  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) DrawableLine$Delete(dl);
  if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) DrawableLine$add(dl, GetMousePosition());

  return !IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
}

#define setOption(prop, cond)            \
  if (prop > cond && dl->prop != prop) { \
    dl->prop = prop;                     \
    res = true;                          \
  }

bool DrawableLine$setOptions(DrawableLine* dl, float error, float thickness, int subdivisions, Color color) {
  DrawableLine$_init(dl);
  bool res = false;

  setOption(error, 0);
  setOption(thickness, 0);
  setOption(subdivisions, 1);

  if (res) nfree(dl->triangleStrip);

  if (memcmp(&dl->color, &color, sizeof(Color)) && memcmp(&color, &BLANK, sizeof(Color))) {
    dl->color = color;
    res = true;
  }

  return res;
}

void DrawableLine$Move(DrawableLine* dl, Vector2 delta) {
  DrawableLine$_init(dl);
  for (int i = 0; i < sb_count(dl->line); i++) {
    dl->line[i] = Vector2Add(dl->line[i], delta);
  }
  nfree(dl->triangleStrip);
}

void DrawableLine$SetColor(DrawableLine* dl, Color color) {
  DrawableLine$_init(dl);
  dl->color = color;
}

Drawable DrawableLine$New() {
  return DRAWABLE(calloc(1, sizeof(DrawableLine)), DrawableLine);
}
