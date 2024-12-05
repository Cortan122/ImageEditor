#include <raylib.h>

#include "Editor.h"
#include "config.h"

int main(int argc, char** argv) {
  const int screenWidth = DEFAULT_SCREENWIDTH;
  const int screenHeight = DEFAULT_SCREENHEIGHT;
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
  InitWindow(screenWidth, screenHeight, WINDOW_TITLE);
  SetWindowMinSize(MINSIZE_WIDTH, MINSIZE_HEIGHT);
  SetTargetFPS(60);
  SetExitKey(0);
  EnableEventWaiting();  // now the fps counter is scuffed

  Editor theEditor = {0};
  Editor$open(&theEditor, argv[1]);

  int i = 0;
  while (!WindowShouldClose()) {
    BeginDrawing();

    Editor$Update(&theEditor);
    Editor$Draw(&theEditor);

    EndDrawing();
  }
  Editor$Delete(&theEditor);

  CloseWindow();
  return 0;
}
