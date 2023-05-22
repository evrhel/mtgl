#pragma once

#if __linux__

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <mtgl/mtgl.h>
#include "mtgl_win.h"

#ifdef __cplusplus
extern "C"
{
#endif

    struct mtglwin_x11
    {
        struct mtglwin win;
        
        Display *dpy;
        XVisualInfo *vi;
        XSetWindowAttributes swa;
        Window window;
        XWindowAttributes gwa;
        XEvent xev;

    };

    struct mtglwin_x11 *mtgl_win_create_x11(const char *title, int width, int height, int flags, int device, void *user_data);

#ifdef __cplusplus
}
#endif

#endif