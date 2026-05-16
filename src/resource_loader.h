#pragma once
#include <raylib.h>

#include "resources.h"

#define array_length(x) (sizeof(x)/sizeof(*x))

typedef Font (*FontLoader)(void);

typedef struct FontDescriptor {
  Font font;
  const char* name;
  FontLoader loader;
} FontDescriptor;

Font GetFont(int index);
FontDescriptor* GetFonts();
int GetFontCount();
Vector2 DrawTextScaled(Font font, const char* text, Vector2 position, float scale, Color tint);

#define LoadImageResource(name) LoadImageResource_impl(#name, resources_##name, resources_##name##_len)
Image LoadImageResource_impl(char* name, unsigned char* data, unsigned int len);

#define LoadShaderResource(name) LoadShaderResource_impl(#name, resources_##name, resources_##name##_len)
Shader LoadShaderResource_impl(char* name, unsigned char* data, unsigned int len);
