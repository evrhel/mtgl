#if __APPLE__
#pragma once

#include "mtgl_ctx.h"

#ifdef __cplusplus
extern "C"
{
#endif

    struct mtglwin_cocoa;

    struct mtglctx_cocoa
    {
        struct mtglctx ctx;
        void *pixel_format; // id
        void *context;      // id
    };

    struct mtglctx_cocoa *mtgl_ctx_create_cocoa(struct mtglwin_cocoa *win, int ver_major, int ver_minor, mtglctxinitargs *argsp);
	struct mtglctx_cocoa *mtgl_ctx_clone_cocoa(struct mtglwin_cocoa *ctx);
	void mtgl_ctx_acquire_cocoa(struct mtglctx_cocoa *ctx);
	int mtgl_ctx_try_acquire_cocoa(struct mtglctx_cocoa *ctx);
	void mtgl_ctx_release_cocoa(struct mtglctx_cocoa *ctx);
	void mtgl_ctx_set_swap_interval_cocoa(struct mtglctx_cocoa *ctx, int interval);
	void mtgl_ctx_destroy_cocoa(struct mtglctx_cocoa *ctx);
	void *mtgl_ctx_get_proc_cocoa(const char *name);

#ifdef __cplusplus
}
#endif

#endif