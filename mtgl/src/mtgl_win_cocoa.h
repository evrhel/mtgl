#if __APPLE__
#pragma once

#if defined(__OBJC__)
#include <Cocoa/Cocoa.h>
#endif

#include <time.h>

#include <mtgl/mtgl.h>
#include "mtgl_win.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /* Cocoa window */
    struct mtglwin_cocoa
    {
        struct mtglwin win; // base class
        
        void *window;       // MTGLWindow
        void *delegate;     // MTGLWindowDelegate
        void *view;         // MTGLView
        void *layer;        // id

        struct timespec start;  // start time
    };

    struct mtglwin_cocoa *mtgl_win_create_cocoa(const char *title, int width, int height, int flags, int device, void *user_data);
    void mtgl_set_title_cocoa(struct mtglwin_cocoa *win, const char *title);
	void mtgl_show_window_cocoa(struct mtglwin_cocoa *win, int shown);
	void mtgl_poll_events_cocoa(struct mtglwin_cocoa *win);
	void mtgl_swap_buffers_cocoa(struct mtglwin_cocoa *win);
	void mtgl_get_full_size_cocoa(struct mtglwin_cocoa *win, int *width, int *height);
	void mtgl_set_size_cocoa(struct mtglwin_cocoa *win, int width, int height);
	void mtgl_set_pos_cocoa(struct mtglwin_cocoa *win, int x, int y);
	float mtgl_get_time_cocoa(struct mtglwin_cocoa *win);
	void mtgl_win_destroy_cocoa(struct mtglwin_cocoa *win);


#ifdef __cplusplus
}
#endif

#endif