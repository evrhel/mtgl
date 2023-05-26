#if __linux__
#include "mtgl_ctx_x11.h"

#include "mtgl_win_x11.h"

struct mtglctx_x11 *mtgl_ctx_create_x11(struct mtglwin_x11 *win, int ver_major, int ver_minor, mtglctxinitargs *argsp)
{
    return 0;
}

struct mtglctx_x11 *mtgl_ctx_clone_x11(struct mtglctx_x11 *ctx)
{
    // TODO: support this
    return 0;
}

void mtgl_ctx_acquire_x11(struct mtglctx_x11 *ctx)
{
    mtglwin_x11 *win = (mtglwin_x11 *)ctx->ctx.win;
    glXMakeCurrent(win->d, win->w, ctx->glx);
}

void mtgl_ctx_release_x11(struct mtglctx_x11 *ctx)
{
    mtglwin_x11 *win = (mtglwin_x11 *)ctx->ctx.win;
    glXMakeCurrent(win->d, None, NULL);
}

void mtgl_ctx_set_swap_interval_x11(struct mtglctx_x11 *ctx, int interval)
{

}

void mtgl_ctx_destroy_x11(struct mtglctx_x11 *ctx)
{

}

void *mtgl_ctx_get_proc_x11(const char *name)
{

}

#endif