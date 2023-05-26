#pragma once
#if __linux__

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <GL/gl.h>
#include <GL/glx.h>

#include "mtgl_ctx.h"

#ifdef __cplusplus
extern "C"
{
#endif

    struct mtglwin_x11;

    struct mtglctx_x11
    {
        struct mtglctx ctx;
        GLXContext glx;
    };

    struct mtglctx_x11 *mtgl_ctx_create_x11(struct mtglwin_x11 *win, int ver_major, int ver_minor, mtglctxinitargs *argsp);
    struct mtglctx_x11 *mtgl_ctx_clone_x11(struct mtglctx_x11 *ctx);
    void mtgl_ctx_acquire_x11(struct mtglctx_x11 *ctx); // not thread-safe
    void mtgl_ctx_release_x11(struct mtglctx_x11 *ctx); // not thread-safe
    void mtgl_ctx_set_swap_interval_x11(struct mtglctx_x11 *ctx, int interval);
    void mtgl_ctx_destroy_x11(struct mtglctx_x11 *ctx);
    void *mtgl_ctx_get_proc_x11(const char *name);

#ifdef __cplusplus
}
#endif

#endif
