#include "Canvas.h"

#include <math.h>

#include "CropRectangle.h"
#include "DrawableLine.h"
#include "FloatingImage.h"
#include "Textbox.h"
#include "resource_loader.h"

const float scaleRom[] = {0, 0.12, 0.25, 0.5, 0.75, 1, 2, 4, 8, 0};
const float movementSpeed = 100;

void Canvas$recenter(Canvas* c) {
  // todo: maybe there should be an option for free movement

  float tempx = c->screenWidth / 2. - c->scale * c->texture.width / 2.;
  if (tempx >= 0)
    c->pos.x = tempx;
  else if (c->pos.x > 0)
    c->pos.x = 0;
  else if (c->pos.x < tempx * 2)
    c->pos.x = tempx * 2;

  float tempy = c->screenHeight / 2. - c->scale * c->texture.height / 2.;
  if (tempy >= 0)
    c->pos.y = tempy;
  else if (c->pos.y > 0)
    c->pos.y = 0;
  else if (c->pos.y < tempy * 2)
    c->pos.y = tempy * 2;
}

void Canvas$rescale(Canvas* c) {
  float aspectRatio1 = c->screenWidth / c->screenHeight;
  float aspectRatio2 = c->texture.width / (float)c->texture.height;
  if (aspectRatio1 > aspectRatio2) {
    c->scale = c->screenHeight / c->texture.height;
  } else {
    c->scale = c->screenWidth / c->texture.width;
  }

  c->pos = (Vector2){0, 0};
  Canvas$recenter(c);

  c->scaleIndex = -1;
  for (int i = 1;; i++) {
    if (fabs(scaleRom[i] - c->scale) < 0.01) {
      c->scaleIndex = i;
      break;
    } else if (scaleRom[i] > c->scale || scaleRom[i] == 0) {
      c->scaleIndex = i - 0.5;
      break;
    }
  }
}

bool Canvas$popDrawable(Canvas* c) {
  if (sb_count(c->drawables) == 0) return false;

  Drawable$Delete(&sb_last(c->drawables));
  sb_add(c->drawables, -1);
  c->isActive = false;

  return true;
}

void Canvas$updateSize(Canvas* c) {
  c->screenWidth =
      GetScreenWidth() - c->marginTopLeft.x - c->marginBottomRight.x;
  c->screenHeight =
      GetScreenHeight() - c->marginTopLeft.y - c->marginBottomRight.y;
}

void Canvas$reload(Canvas* c, bool checkClipboard) {
  if (checkClipboard) {
    while (Canvas$popDrawable(c))
      ;
    Screenshot$update(&c->screenshot);
  }

  c->texture = c->screenshot.texture;
  SetTextureFilter(c->texture,
                   c->nearestNeighborToggle ? FILTER_BILINEAR : FILTER_POINT);
  Canvas$updateSize(c);
  Canvas$rescale(c);
}

void Canvas$zoom(Canvas* c, int delta) {
  if (!delta) return;

  // use mouse position as fixed point
  Vector2 pos = GetMousePosition();
  Vector2 relpos;
  relpos.x = (c->pos.x - pos.x) / c->scale;
  relpos.y = (c->pos.y - pos.y) / c->scale;

  while (delta) {
    int newIndex;
    if (delta > 0) {
      delta--;
      newIndex = floorf(c->scaleIndex) + 1;
    } else {
      delta++;
      newIndex = ceilf(c->scaleIndex) - 1;
    }
    if (scaleRom[newIndex]) {
      c->scale = scaleRom[newIndex];
      c->scaleIndex = newIndex;
    } else
      break;
  }

  c->pos.x = relpos.x * c->scale + pos.x;
  c->pos.y = relpos.y * c->scale + pos.y;

  Canvas$recenter(c);
}

void Canvas$updateDrawable(Canvas* c) {
  Vector2 realpos = Vector2Add(c->marginTopLeft, c->pos);
  SetMouseOffset(-realpos.x, -realpos.y);
  SetMouseScale(1 / c->scale, 1 / c->scale);
  c->isActive = Drawable$Update(&sb_last(c->drawables));
  SetMouseOffset(0, 0);
  SetMouseScale(1, 1);
}

void Canvas$takeScreenshot(Canvas* c) {
  takeScreenshot();
  c->scheduledReload = true;
}

void Canvas$keyboardShortcuts(Canvas* c) {
  bool isctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);

  if (IsKeyTyped(KEY_EQUAL) || IsKeyTyped(KEY_KP_ADD)) Canvas$zoom(c, 1);
  if (IsKeyTyped(KEY_MINUS) || IsKeyTyped(KEY_KP_SUBTRACT)) Canvas$zoom(c, -1);
  if (IsKeyTyped(KEY_HOME) || IsKeyTyped(KEY_R)) Canvas$rescale(c);
  if (!isctrl && IsKeyTyped(KEY_P)) {
    c->nearestNeighborToggle = !c->nearestNeighborToggle;
    SetTextureFilter(c->texture,
                     c->nearestNeighborToggle ? FILTER_BILINEAR : FILTER_POINT);
  }

  if (IsKeyTyped(KEY_L)) Canvas$takeScreenshot(c);
  if (isctrl && IsKeyTyped(KEY_V)) Canvas$reload(c, true);

  Vector2 delta = {0};
  if (!isctrl && IsKeyTyped(KEY_W) || IsKeyTyped(KEY_UP)) delta.y--;
  if (!isctrl && IsKeyTyped(KEY_S) || IsKeyTyped(KEY_DOWN)) delta.y++;
  if (!isctrl && IsKeyTyped(KEY_A) || IsKeyTyped(KEY_LEFT)) delta.x--;
  if (!isctrl && IsKeyTyped(KEY_D) || IsKeyTyped(KEY_RIGHT)) delta.x++;
  c->pos.x += -delta.x * movementSpeed;
  c->pos.y += -delta.y * movementSpeed;
  if (delta.x || delta.y) Canvas$recenter(c);
}

bool Canvas$copy(Canvas* c, char* name) {
  Screenshot$begin(&c->screenshot);
  DrawTexture(c->texture, 0, 0, WHITE);
  for (int i = 0; i < sb_count(c->drawables); i++) {
    Drawable$Draw(c->drawables + i);
  }
  return Screenshot$end(&c->screenshot, name);
}

void Canvas$addDrawable(Canvas* c, Drawable d) {
  sb_push(c->drawables, d);
  Canvas$updateDrawable(c);
  Drawable$SetColor(&sb_last(c->drawables), c->color);
}

void Canvas$Update(Canvas* c) {
  Canvas$updateSize(c);
  if (IsWindowResized()) Canvas$rescale(c);
  if (c->scheduledReload) {
    c->scheduledReload = false;
    Canvas$reload(c, true);
  }

  // todo?: дробный зум
  float delta = GetMouseWheelMove() + c->scrollAccumulator;
  float continuousDelta = fmodf(delta, 1.0);
  int discreteDelta = delta - continuousDelta;
  Canvas$zoom(c, discreteDelta);
  c->scrollAccumulator = continuousDelta;

  bool isctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
  if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON) ||
      isctrl && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
    Vector2 mousePos = GetMousePosition();
    c->pos.x += mousePos.x - c->prevMousePos.x;
    c->pos.y += mousePos.y - c->prevMousePos.y;
    Canvas$recenter(c);
  }

  if (c->isActive) {
    Canvas$updateDrawable(c);
  }
  if (!c->isActive) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
      Canvas$addDrawable(c, DrawableLine$New());
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
      Canvas$addDrawable(c, Textbox$New());
    if (IsKeyTyped(KEY_X)) Canvas$addDrawable(c, CropRectangle$New(c));
    if (IsKeyTyped(KEY_B)) Canvas$addDrawable(c, CropRectangle$New(NULL));

    int count;
    char** file = GetDroppedFiles(&count);
    for (int i = 0; i < count; i++) {
      Canvas$addDrawable(c, FloatingImage$New(file[i]));
    }
    ClearDroppedFiles();

    Canvas$keyboardShortcuts(c);
  }

  if (isctrl && IsKeyTyped(KEY_Z)) Canvas$popDrawable(c);
  if (isctrl && IsKeyTyped(KEY_C)) Canvas$copy(c, NULL);
  if (isctrl && IsKeyTyped(KEY_P)) Canvas$takeScreenshot(c);
  if (IsKeyTyped(KEY_F5)) Canvas$reload(c, true);
  if (IsKeyTyped(KEY_PAGE_UP)) Canvas$zoom(c, 1);
  if (IsKeyTyped(KEY_PAGE_DOWN)) Canvas$zoom(c, -1);

  c->prevMousePos = GetMousePosition();
}

void Canvas$Draw(Canvas* c) {
  if (c->transparencyTexture.id == 0) {
    c->transparencyTexture =
        LoadTextureFromImage(LoadImageResource(transparency_png));
  }

  BeginScissorMode(c->marginTopLeft.x, c->marginTopLeft.y, c->screenWidth,
                   c->screenHeight);

  Vector2 realpos = Vector2Add(c->marginTopLeft, c->pos);
  Texture tiling = c->transparencyTexture;
  DrawTextureQuad(
      tiling,
      (Vector2){c->screenWidth / tiling.width, c->screenHeight / tiling.height},
      (Vector2){0},
      (Rectangle){c->marginTopLeft.x, c->marginTopLeft.y, c->screenWidth,
                  c->screenHeight},
      WHITE);
  DrawTextureEx(c->texture, realpos, 0, c->scale, WHITE);

  BeginMode2D((Camera2D){realpos, (Vector2){0}, 0, c->scale});
  for (int i = 0; i < sb_count(c->drawables); i++) {
    Drawable$Draw(c->drawables + i);
  }
  EndMode2D();
  EndScissorMode();
}

bool Canvas$loadImage(Canvas* c, char* file) {
  bool res = true;
  if (file) {
    Image image = LoadImage(file);
    res = Screenshot$setImage(&c->screenshot, image);
    UnloadImage(image);
  } else if (!Screenshot$update(&c->screenshot)) {
    Screenshot$setImage(&c->screenshot, LoadImageResource(nothing_png));
  }

  Canvas$reload(c, false);
  return res;
}

void Canvas$SetColor(Canvas* c, Color color) {
  c->color = color;
  if (c->isActive) {
    Drawable$SetColor(&sb_last(c->drawables), color);
  }
}
