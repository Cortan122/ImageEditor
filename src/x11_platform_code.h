#pragma once

#include <stddef.h>
#include <stdint.h>

uint8_t* x11GetImageFromClipboardSubprocess(int *length);
uint8_t* x11GetImageFromClipboardNonblocking(int *length);
void x11CleanupState();
