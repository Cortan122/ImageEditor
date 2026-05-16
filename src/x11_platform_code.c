#include "x11_platform_code.h"

#include <X11/Xlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PNG_FILE_MAGIC "\x89PNG\r\n\x1a\n"

#ifndef _WIN32
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#include <X11/extensions/sync.h>

// We actually need to keep the ghost window around, because in case we are trying to paste from ourselves,
// our main process will try to send the clipboard to a dead window, and it will crash.
// But we have to destroy our old window every time, because otherwise it would have extra alarms in its event queue.
static Display* dpy = NULL;
static Window win = None;
static XSyncAlarm syncAlarm;
static int sync_event = 0;
static XSyncAlarmAttributes attr;
static unsigned long flags = 0;

static void setupAlarm() {
    // Alarm code from https://nrk.neocities.org/articles/x11-timeout-with-xsyncalarm
    int sync_error, tmp;
    if (!XSyncQueryExtension(dpy, &sync_event, &sync_error)) {
        printf("\x1b[31mERROR: XSync extension not available\x1b[0m\n");
        exit(1);
    }
    if (!XSyncInitialize(dpy, &tmp, &tmp)) {
        printf("\x1b[31mERROR: failed to initialize XSync extension\x1b[0m\n");
        exit(1);
    }

    int ncounter = 0;
    XSyncSystemCounter *counters = XSyncListSystemCounters(dpy, &ncounter);
    XSyncCounter servertime = None;
    if (counters) {
        for (int i = 0; i < ncounter; i++) {
            if (strcmp(counters[i].name, "SERVERTIME") == 0) {
                servertime = counters[i].counter;
                break;
            }
        }
        XSyncFreeSystemCounterList(counters);
    }
    if (servertime == None) {
        printf("\x1b[31mERROR: SERVERTIME counter not found\x1b[0m\n");
        exit(1);
    }

    attr.trigger.counter = servertime;
    flags |= XSyncCACounter;
    XSyncIntToValue(&attr.trigger.wait_value, 16); // 16ms
    flags |= XSyncCAValue;
    attr.trigger.value_type = XSyncRelative;
    flags |= XSyncCAValueType;
    attr.trigger.test_type = XSyncPositiveComparison;
    flags |= XSyncCATestType;
    XSyncIntToValue(&attr.delta, 0);
    flags |= XSyncCADelta;

    syncAlarm = XSyncCreateAlarm(dpy, flags, &attr);
}

bool x11NextEventSafe(int type, int attempts) {
    // Wait for the SelectionNotify event.
    // Try `attempts` times, and quit if we keep hitting the alarm.
    for (int i = 0; i < attempts; i++) {
        XEvent ev = {0};
        XNextEvent(dpy, &ev);

        // Check if we got the alarm
        if (ev.type == sync_event + XSyncAlarmNotify) {
            if (i > 0) {
                printf("\x1b[33mWARN: X11 clipboard timed out \x1b[32m%d\x1b[33m times\x1b[0m\n", i);
            }
        } else if (ev.type == PropertyNotify && ev.xproperty.state != PropertyNewValue) {
            continue;
        } else if (ev.type == type) {
            return true;
        } else if (ev.type == PropertyNotify) {
            continue;
        } else {
            printf("\x1b[33mWARN: Unknown X11 event \x1b[32m%d\x1b[33m\x1b[0m\n", ev.type);
        }

        XSyncChangeAlarm(dpy, syncAlarm, flags, &attr);
    }

    return false;
}

void x11CreateWindow() {
    if (win != None) {
        XDestroyWindow(dpy, win);
        win = None;
    }

    Window root = DefaultRootWindow(dpy);
    win = XCreateSimpleWindow(
        dpy,      // The connection to the X Server
        root,     // The 'Parent' window (usually the desktop/root)
        0, 0,     // X and Y position on the screen
        1, 1,     // Width and Height (1x1 pixel)
        0,        // Border width
        0,        // Border color
        0         // Background color
    );

    // We also care about PropertyNotify events, because INCR
    XSelectInput(dpy, win, PropertyChangeMask);
}

// To convert to raylib image:
//     image = LoadImageFromMemory(".png", data, length);
uint8_t* x11GetImageFromClipboardNonblocking(int *length) {
    // Code taken from: https://github.com/raysan5/raylib/blob/master/src/platforms/rcore_desktop_glfw.c
    // REF: https://github.com/ColleagueRiley/Clipboard-Copy-Paste/blob/main/x11.c

    x11CleanupState();
    if (dpy == NULL) {
        dpy = XOpenDisplay(NULL);
        if (!dpy) return NULL;

        setupAlarm();
        x11CreateWindow();
    }

    Atom clipboard = XInternAtom(dpy, "CLIPBOARD", False);
    Atom targetType = XInternAtom(dpy, "image/png", False); // Ask for PNG
    Atom property = XInternAtom(dpy, "RAYLIB_CLIPBOARD_MANAGER", False);

    // Request the data: "Convert whatever is in CLIPBOARD to image/png and put it in RAYLIB_CLIPBOARD_MANAGER"
    XConvertSelection(dpy, clipboard, targetType, property, win, CurrentTime);

    if (!x11NextEventSafe(SelectionNotify, 10)) return NULL;

    Atom actualType = {0};
    int actualFormat = 0;
    unsigned long nitems = 0;
    unsigned long bytesAfter = 0;
    unsigned char *data = NULL;

    // Read the data from our ghost window's property
    XGetWindowProperty(dpy, win, property, 0, ~0L, False, AnyPropertyType, &actualType, &actualFormat, &nitems, &bytesAfter, &data);

    char *actualTypeName = "";
    if (actualType) {
        actualTypeName = XGetAtomName(dpy, actualType);
    }

    uint8_t *retval = NULL;
    if (strcmp(actualTypeName, "image/png") == 0) {
        if (data == NULL) {
            printf("\x1b[33mWARN: We got a png with no data\x1b[0m\n");
        } else {
            printf("\x1b[90mINFO: Got direct png of size \x1b[32m%lu\x1b[0m\n", nitems);
            retval = calloc(nitems, 1);
            memcpy(retval, data, nitems);
            *length = nitems;
        }
    } else if (strcmp(actualTypeName, "UTF8_STRING") == 0) {
        // When we get a UTF8_STRING here, it does not mean that the clipboard contains a utf8 string.
        // Why would it ever be this simple?
        // It actually means that the sender wants us to use the INCR protocol, but isn't ready with the image yet...
        // If we wait, the next property set here would be the INCR one.
        // The `data` buffer here is just empty...
        printf("\x1b[33mWARN: We got a UTF8_STRING when asking for a PNG\x1b[0m\n");
        printf("\x1b[36mINFO: Try pasting again in a while...\x1b[0m\n");
    } else if (strcmp(actualTypeName, "INCR") == 0) {
        size_t INCR_length = 0;
        uint8_t* INCR_dynamic_arr = NULL;
        printf("\x1b[36mINFO: INCR Protocol...\x1b[0m\n");

    again:;
	    XDeleteProperty(dpy, win, property);
	    XFlush(dpy);

        if (!x11NextEventSafe(PropertyNotify, 100)) {
            printf("horrible horrible fate\n");
            exit(1);
        }

        if (data) {
            XFree(data);
            data = NULL;
        }
        XGetWindowProperty(dpy, win, property, 0, ~0L, False, AnyPropertyType, &actualType, &actualFormat, &nitems, &bytesAfter, &data);

        if (actualType == targetType) {
            // We got data!!
            int bytes_per_item = actualFormat / 8;
            int num_bytes = bytes_per_item * nitems;
            if (num_bytes == 0) {
                printf("\x1b[36mINFO: INCR Protocol finished!!\x1b[0m\n");
                *length = INCR_length;
                retval = INCR_dynamic_arr;
                goto short_circuit;
            }

            printf("\x1b[36mINFO: INCR Protocol chunk \x1b[32m%d\x1b[36m bytes\x1b[0m\n", num_bytes);

            size_t new_length = INCR_length + num_bytes;
            INCR_dynamic_arr = realloc(INCR_dynamic_arr, new_length);
	        memcpy(INCR_dynamic_arr + INCR_length, data, num_bytes);
            INCR_length = new_length;
        }

        goto again;
    } else if (*actualTypeName) {
        printf("\x1b[33mWARN: Unexpected Atom name %s\x1b[0m\n", actualTypeName);
    }

short_circuit:;
    XDeleteProperty(dpy, win, property);
    XFlush(dpy);
    if (*actualTypeName) XFree(actualTypeName);
    if (data) XFree(data);
    return retval;
}

void x11CleanupState() {
    if (dpy == NULL) return;

    XSyncDestroyAlarm(dpy, syncAlarm);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
    win = None;
    dpy = NULL;
}
#endif
