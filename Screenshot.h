#pragma once
#include <raylib.h>

typedef struct Screenshot {
  Image image;
  Texture texture;
  RenderTexture renderTexture;
  int width;
  int height;
} Screenshot;

Image getImageFromClipboard();
bool putImageToClipboard(Image image);
bool takeScreenshot();
void waitEvents();

bool Screenshot$setImage(Screenshot* cm, Image img);
bool Screenshot$update(Screenshot* cm);
void Screenshot$begin(Screenshot* cm);
bool Screenshot$end(Screenshot* cm, char* name);
void Screenshot$Delete(Screenshot* cm);
