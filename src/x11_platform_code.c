#include "x11_platform_code.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

// to convert to raylib image:
// image = LoadImageFromMemory(".png", data, length);
uint8_t* x11GetImageFromClipboardNonblocking(int *length) {
    // code taken from: https://github.com/raysan5/raylib/blob/master/src/platforms/rcore_desktop_glfw.c
    // REF: https://github.com/ColleagueRiley/Clipboard-Copy-Paste/blob/main/x11.c
    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) return NULL;

    Window root = DefaultRootWindow(dpy);
    Window win = XCreateSimpleWindow(
        dpy,      // The connection to the X Server
        root,     // The 'Parent' window (usually the desktop/root)
        0, 0,     // X and Y position on the screen
        1, 1,     // Width and Height (1x1 pixel)
        0,        // Border width
        0,        // Border color
        0         // Background color
    );

    Atom clipboard = XInternAtom(dpy, "CLIPBOARD", False);
    Atom targetType = XInternAtom(dpy, "image/png", False); // Ask for PNG
    Atom property = XInternAtom(dpy, "RAYLIB_CLIPBOARD_MANAGER", False);

    // Request the data: "Convert whatever is in CLIPBOARD to image/png and put it in RAYLIB_CLIPBOARD_MANAGER"
    XConvertSelection(dpy, clipboard, targetType, property, win, CurrentTime);

    // Wait for the SelectionNotify event
    XEvent ev = {0};
    XNextEvent(dpy, &ev);

    Atom actualType = {0};
    int actualFormat = 0;
    unsigned long nitems = 0;
    unsigned long bytesAfter = 0;
    unsigned char *data = NULL;

    // Read the data from our ghost window's property
    XGetWindowProperty(dpy, win, property, 0, ~0L, False, AnyPropertyType, &actualType, &actualFormat, &nitems, &bytesAfter, &data);

    uint8_t *retval = NULL;
    if (data != NULL) {
        retval = calloc(nitems, 1);
        memcpy(retval, data, nitems);
        *length = nitems;
        XFree(data);
    }

    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);

    return retval;
}
#endif
