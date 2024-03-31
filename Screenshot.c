#include "Screenshot.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define Rectangle Rectangle_winapi
#define CloseWindow CloseWindow_winapi
#define ShowCursor ShowCursor_winapi
#include <Windows.h>
#undef Rectangle
#undef CloseWindow
#undef ShowCursor
#undef DrawTextEx

Image getImageFromClipboard() {
  Image image = {0};

  if (!OpenClipboard(NULL)) return image;
  HANDLE handle = GetClipboardData(CF_DIBV5);
  if (handle) {
    char* data = GlobalLock(handle);
    if (data) {
      BITMAPV5HEADER* header = (BITMAPV5HEADER*)data;

      uint32_t* imagedata = MemAlloc(header->bV5SizeImage);
      image.data = imagedata;
      image.mipmaps = 1;
      image.height = header->bV5Height;
      image.width = header->bV5Width;

      uint32_t* idata = (uint32_t*)(data + header->bV5Size);
      if (header->bV5Compression == BI_BITFIELDS && header->bV5SizeImage > 12) {
        if (idata[0] == 0x00ff0000 && idata[1] == 0x0000ff00 &&
            idata[2] == 0x000000ff) {
          idata += 3;
        }
      }

      if (header->bV5BitCount == 32) {
        image.format = UNCOMPRESSED_R8G8B8A8;
        for (int i = 0; i < image.height * image.width; i++) {
          uint32_t temp = idata[i];
          imagedata[i] = (temp & 0xff00ff00) | (temp & 0x00ff0000) >> 16 |
                         (temp & 0x000000ff) << 16;
        }
      } else {
        image.format = UNCOMPRESSED_R8G8B8;
        int linelen = header->bV5SizeImage / header->bV5Height;
        for (int i = 0; i < image.height; i++) {
          char* d1 = (char*)imagedata + i * image.width * 3;
          char* d2 = (char*)idata + i * linelen;
          for (int j = 0; j < image.width; j++) {
            d1[j * 3 + 0] = d2[j * 3 + 2];
            d1[j * 3 + 1] = d2[j * 3 + 1];
            d1[j * 3 + 2] = d2[j * 3 + 0];
          }
        }
      }

      GlobalUnlock(handle);
    }
  }
  CloseClipboard();

  ImageFlipVertical(&image);
  return image;
}

bool putImageToClipboard(Image image) {
  if (image.data == NULL || image.width <= 0 || image.height <= 0 ||
      image.mipmaps != 1 || image.format != UNCOMPRESSED_R8G8B8A8)
    return false;

  size_t datasize = image.width * image.height * 4 + sizeof(BITMAPV5HEADER);
  BITMAPV5HEADER header = {
      .bV5Size = sizeof(BITMAPV5HEADER),
      .bV5Width = image.width,
      .bV5Height = image.height,
      .bV5Planes = 1,
      .bV5BitCount = 32,
      .bV5SizeImage = image.width * image.height * 4,
      .bV5RedMask = 0x00ff0000,
      .bV5GreenMask = 0x0000ff00,
      .bV5BlueMask = 0x000000ff,
      .bV5Intent = LCS_GM_IMAGES,
      .bV5CSType = 1934772034,  // 'sRGB'
  };

  HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, datasize);
  if (handle == NULL) return false;

  char* data = GlobalLock(handle);
  if (data == NULL) return false;
  memcpy(data, &header, sizeof(BITMAPV5HEADER));
  uint32_t* imagedata = (uint32_t*)(data + sizeof(BITMAPV5HEADER));
  uint32_t* idata = image.data;
  for (int i = 0; i < image.height; i++) {
    for (int j = 0; j < image.width; j++) {
      uint32_t temp = idata[(image.height - i - 1) * image.width + j];
      imagedata[i * image.width + j] = (temp & 0xff00ff00) |
                                       (temp & 0x00ff0000) >> 16 |
                                       (temp & 0x000000ff) << 16;
    }
  }
  GlobalUnlock(handle);

  bool res = false;
  if (OpenClipboard(NULL)) {
    if (EmptyClipboard()) {
      res = SetClipboardData(CF_DIBV5, handle) != NULL;
    }
  }
  CloseClipboard();

  GlobalFree(handle);
  return res;
}

bool takeScreenshot() {
  INPUT input = {0};
  input.type = INPUT_KEYBOARD;
  input.ki.wVk = 0x2c;
  input.ki.wScan = 0x137;

  return SendInput(1, &input, sizeof(INPUT)) == 1;
}

void waitEvents() { WaitMessage(); }

#else
#include <unistd.h>

#define PE(x)      \
  if ((x) == -1) { \
    perror(#x);    \
    return res;    \
  }

char tempFileName[] = "/tmp/screenshotXXXXXX.png";
int tempFileDescriptor;

char* createTempFile() {
  char* res = NULL;
  strcpy(tempFileName, "/tmp/screenshotXXXXXX.png");
  tempFileDescriptor = mkstemps(tempFileName, 4);
  PE(tempFileDescriptor);
  PE(setenv("tempfilename", tempFileName, 1));
  return tempFileName;
}

bool deleteTempFile() {
  bool res = false;
  PE(close(tempFileDescriptor));
  PE(unlink(tempFileName));
  return true;
}

Image getImageFromClipboard() {
  Image res = {0};

  if (createTempFile()) {
    if (!system(
            "xclip -selection clipboard -t image/png -o > \"$tempfilename\"")) {
      res = LoadImage(tempFileName);
    }
  }
  deleteTempFile();

  return res;
}

bool putImageToClipboard(Image image) {
  bool res = false;

  if (createTempFile()) {
    if (ExportImage(image, tempFileName)) {
      if (!system(
              "xclip -selection clipboard -t image/png -i \"$tempfilename\"")) {
        res = true;
      }
    }
  }
  deleteTempFile();

  return res;
}

bool takeScreenshot() {
  bool res = false;

  if (createTempFile()) {
    if (!system("import \"$tempfilename\" && xclip -selection clipboard -t "
                "image/png -i \"$tempfilename\"")) {
      res = true;
    }
  }
  deleteTempFile();

  return res;
}

#include <GLFW/glfw3.h>

void waitEvents() { glfwWaitEvents(); }

#endif

bool Screenshot$setImage(Screenshot* cm, Image img) {
  if (img.data == NULL) return false;
  UnloadImage(cm->image);
  cm->image = (Image){0};
  cm->image = img;
  cm->width = img.width;
  cm->height = img.height;

  UnloadRenderTexture(cm->renderTexture);
  cm->renderTexture = (RenderTexture){0};
  UnloadTexture(cm->texture);
  cm->texture = LoadTextureFromImage(img);
  return true;
}

bool Screenshot$update(Screenshot* cm) {
  double t1 = GetTime();
  Image img = getImageFromClipboard();
  double t2 = GetTime();
  printf("getImageFromClipboard(%d) = %.2fms\n", img.data != NULL,
         (t2 - t1) * 1000);
  return Screenshot$setImage(cm, img);
}

void Screenshot$begin(Screenshot* cm) {
  if (cm->renderTexture.id == 0) {
    cm->renderTexture = LoadRenderTexture(cm->width, cm->height);
  }
  BeginTextureMode(cm->renderTexture);
}

bool Screenshot$end(Screenshot* cm, char* name) {
  EndTextureMode();
  Image img = LoadImageFromTexture(cm->renderTexture.texture);
  ImageFlipVertical(&img);

  bool res;
  if (name) {
    res = ExportImage(img, name);
  } else {
    double t1 = GetTime();
    res = putImageToClipboard(img);
    if (!res) res = putImageToClipboard(img);
    double t2 = GetTime();
    printf("putImageToClipboard(%d) = %.2fms\n", res, (t2 - t1) * 1000);
  }

  UnloadImage(img);
  return res;
}

void Screenshot$Delete(Screenshot* cm) {
  UnloadImage(cm->image);
  UnloadTexture(cm->texture);
  UnloadRenderTexture(cm->renderTexture);
  cm->image = (Image){0};
  cm->texture = (Texture){0};
  cm->renderTexture = (RenderTexture){0};
  cm->width = 0;
  cm->height = 0;
}
