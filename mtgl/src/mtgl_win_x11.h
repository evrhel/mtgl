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
        
        Display *d; // X11 display
        Window w; // X11 window
        int s; // X11 screen
    };

    struct mtglwin_x11 *mtgl_win_create_x11(const char *title, int width, int height, int flags, int device, void *user_data);
    void mtgl_set_title_x11(struct mtglwin_x11 *win, const char *title);
	void mtgl_show_window_x11(struct mtglwin_x11 *win, int shown);
	void mtgl_poll_events_x11(struct mtglwin_x11 *win);
	void mtgl_swap_buffers_x11(struct mtglwin_x11 *win);
	void mtgl_get_full_size_x11(struct mtglwin_x11 *win, int *width, int *height);
	void mtgl_set_size_x11(struct mtglwin_x11 *win, int width, int height);
	void mtgl_set_pos_x11(struct mtglwin_x11 *win, int x, int y);
	float mtgl_get_time_x11(struct mtglwin_x11 *win);
	void mtgl_win_destroy_x11(struct mtglwin_x11 *win);

#ifdef __cplusplus
}
#endif

#endif