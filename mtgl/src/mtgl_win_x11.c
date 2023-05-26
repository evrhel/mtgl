#include "mtgl_win_x11.h"

#include <stdlib.h>
#include <string.h>

#if __linux__

struct mtglwin_x11 *mtgl_win_create_x11(const char *title, int width, int height, int flags, int device, void *user_data)
{
    // TODO: this really doesn't do anything, it's just some code I found online that I was testing

    struct mtglwin_x11 *winx11 = 0;

    winx11 = calloc(1, sizeof(struct mtglwin_x11));
    if (!winx11)
        return 0;

    winx11->d = XOpenDisplay(NULL);
    if (!winx11->d)
        goto failure;

    winx11->s = DefaultScreen(winx11->d);
    winx11->w = XCreateSimpleWindow(
        winx11->d,
        RootWindow(winx11->d, winx11->s),
        10, 10,
        100, 100,
        1,
        BlackPixel(winx11->d, winx11->s),
        WhitePixel(winx11->d, winx11->s));
   
    XSelectInput(winx11->d, winx11->w, ExposureMask | KeyPressMask);
    XMapWindow(winx11->d, winx11->w);

    return winx11;
failure:

    if (winx11->w)
    {
        XUnmapWindow(winx11->d, winx11->w);
        XDestroyWindow(winx11->d, winx11->w);
    }

    if (winx11->d)
        XCloseDisplay(winx11->d);

    free(winx11);

    return 0;
}

void mtgl_set_title_x11(struct mtglwin_x11 *win, const char *title)
{
    // TODO: set the title
}

void mtgl_show_window_x11(struct mtglwin_x11 *win, int shown)
{
    // TODO: show/hide the window
}

void mtgl_poll_events_x11(struct mtglwin_x11 *win)
{
    int count = 1;
    XEvent e;

    while (XPending(win->d))
    {
        XNextEvent(win->d, &e);

        // TODO: process the event
    }
}

void mtgl_swap_buffers_x11(struct mtglwin_x11 *win)
{
    // TODO: swap buffers
}

void mtgl_get_full_size_x11(struct mtglwin_x11 *win, int *width, int *height)
{
    // TODO: retrieve full size
}

void mtgl_set_size_x11(struct mtglwin_x11 *win, int width, int height)
{
    // TODO: set the size
}

void mtgl_set_pos_x11(struct mtglwin_x11 *win, int x, int y)
{
    // TODO: set the position
}

float mtgl_get_time_x11(struct mtglwin_x11 *win)
{
    // TODO: store the time
    return 0.0f;
}

void mtgl_win_destroy_x11(struct mtglwin_x11 *win)
{
    // TODO: this cleanup is unlikely complete

    if (!win) return;

    XCloseDisplay(win->d);

    free(win);
}

#endif