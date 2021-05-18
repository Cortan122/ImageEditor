#include "resource_loader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "resources.c"
#include "stretchy_buffer.h"

typedef struct ResourceDescriptor {
  const char* name;
  Image image;
} ResourceDescriptor;

static ResourceDescriptor* loadedResources = NULL;

Image LoadImageResource_impl(char* name, unsigned char* data,
                             unsigned int len) {
  for (int i = 0; i < sb_count(loadedResources); i++) {
    if (strcmp(loadedResources[i].name, name) == 0)
      return loadedResources[i].image;
  }

  Image image = LoadImageFromMemory("png", data, len);

  sb_push(loadedResources, ((ResourceDescriptor){name, image}));
  return image;
}

static int fontCharRom[] = {
    32,   33,   34,   35,   36,   37,   38,   39,   40,   41,   42,   43,
    44,   45,   46,   47,   48,   49,   50,   51,   52,   53,   54,   55,
    56,   57,   58,   59,   60,   61,   62,   63,   64,   65,   66,   67,
    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,   78,   79,
    80,   81,   82,   83,   84,   85,   86,   87,   88,   89,   90,   91,
    92,   93,   94,   95,   96,   97,   98,   99,   100,  101,  102,  103,
    104,  105,  106,  107,  108,  109,  110,  111,  112,  113,  114,  115,
    116,  117,  118,  119,  120,  121,  122,  123,  124,  125,  126,  // ascii
    1040, 1041, 1042, 1043, 1044, 1045, 1046, 1047, 1048, 1050, 1051, 1052,
    1053, 1054, 1055, 1056, 1057, 1058, 1059, 1060, 1061, 1062, 1063, 1064,
    1065, 1066, 1067, 1068, 1069, 1070, 1071, 1072, 1073, 1074, 1075, 1076,
    1077, 1078, 1079, 1080, 1082, 1083, 1084, 1085, 1086, 1087, 1088, 1089,
    1090, 1091, 1092, 1093, 1094, 1095, 1096, 1097, 1098, 1099, 1100, 1101,
    1102, 1103,                    // кириллица
    1049, 1081, 1025, 1105, 8470,  // ЙйЁё№
};

static Font LoadConsolasFont() {
  const char* filename = NULL;
#define LOAD_FONT_IF_EXISTS(x) \
  if (filename == NULL && FileExists(x)) filename = x;

  LOAD_FONT_IF_EXISTS("consolas.ttf");
  LOAD_FONT_IF_EXISTS("resources/consolas.ttf");
  LOAD_FONT_IF_EXISTS("/usr/share/fonts/TTF/Consolas-Regular.ttf");
  LOAD_FONT_IF_EXISTS("/usr/share/fonts/TTF/Consolas.ttf");
  LOAD_FONT_IF_EXISTS("/usr/share/fonts/Consolas-Regular.ttf");
  LOAD_FONT_IF_EXISTS("/usr/share/fonts/Consolas.ttf");
  LOAD_FONT_IF_EXISTS("/usr/share/fonts/consolas.ttf");
  LOAD_FONT_IF_EXISTS("/mnt/c/Windows/Fonts/consola.ttf");
  LOAD_FONT_IF_EXISTS("c:/Windows/Fonts/consola.ttf");

  if (!filename) return (Font){0};

  const int fontsize = 40;
  return LoadFontEx(filename, fontsize, fontCharRom,
                    sizeof(fontCharRom) / sizeof(*fontCharRom));
}

static Font LoadMinecraftFont() {
  Font font = LoadFontFromImage(LoadImageResource(font_png), MAGENTA, ' ');
  for (int i = 0; i < font.charsCount; i++) {
    font.chars[i].value = fontCharRom[i];
  }
  font.baseSize = 8;
  return font;
}

static FontDescriptor font_rom[] = {
    {{0}, "default raylib font", GetFontDefault},
    {{0}, "minecraft ascii font", LoadMinecraftFont},
    {{0}, "consolas monospace font", LoadConsolasFont},
};

Font GetFont(int index) {
  if (font_rom[index].font.chars == NULL)
    font_rom[index].font = font_rom[index].loader();
  return font_rom[index].font;
}

FontDescriptor* GetFonts() { return font_rom; }

Vector2 DrawTextScaled(Font font, const char* text, Vector2 position,
                       float scale, Color tint) {
  Vector2 delta = MeasureTextEx(font, text, font.baseSize * scale, scale);
  DrawTextEx(font, text, position, font.baseSize * scale, scale, tint);
  position.x += delta.x;
  // position.y += delta.y;
  return position;
}

Shader LoadShaderResource_impl(char* name, unsigned char* data,
                               unsigned int len) {
  if (data[len] != '\0') {
    if (data[len - 1] == '\n' || data[len - 1] == '\0') {
      data[len - 1] = '\0';
    } else {
      // panic!
      printf("ERROR: LoadShader: No newline at end of file \"%s\"\n", name);
      exit(1);
    }
  }

#ifdef FILTER_POINT
  return LoadShaderFromMemory(NULL, (char*)data);
#else
  return LoadShaderCode(NULL, (char*)data);
#endif
}
