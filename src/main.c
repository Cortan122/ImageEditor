#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Editor.h"
#include "config.h"

int main(int argc, char** argv) {
  const char* display = getenv("DISPLAY");
  const char* session = getenv("XDG_SESSION_TYPE");
  bool is_display_unset = display == NULL || strcmp(display, "") == 0;
  if (is_display_unset && strcmp(session, "wayland") == 0) {
    fprintf(stderr, "You don't seem to have the DISPLAY variable set. Did you forget to install xorg-xwayland?\n");
    return 1;
  } else if (is_display_unset) {
    fprintf(stderr, "You don't seem to have the DISPLAY variable set. Are you trying to start me from a tty session?\n");
    return 1;
  }

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

  // TODO: actually think about this feature
  // have like an Editor$preferredSize
  // and limit it to a maximum of DEFAULT_SCREENWIDTH, DEFAULT_SCREENHEIGHT
  // and some reasonable minimums as well
  // having it perfect looks kinda wrong as well
  /* int width = theEditor.canvas.texture.width;
  int height = theEditor.canvas.texture.height + 40;
  SetWindowSize(width, height); */

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
