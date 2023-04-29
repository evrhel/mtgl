#include "mtgl_ctx.h"

#if _WIN32
#include "mtgl_ctx_win32.h"
#endif

#include "GL/glcorearb.h"
#include "GL/wglext.h"

void
mtgl_ctx_get_default_init_args(mtglctxinitargs *args)
{
	args->profile = mtgl_profile_core;

	args->color_format = mtgl_color_format_rgba;
	args->red_bits = 8;
	args->green_bits = 8;
	args->blue_bits = 8;
	args->alpha_bits = 8;

	args->depth_bits = 24;
	args->stencil_bits = 8;

	args->double_buffer = 1;
	args->allow_sampling = 0;
	args->sample_count = 0;
}

mtglctx *
mtgl_ctx_create(mtglwin *win, int ver_major, int ver_minor, mtglctxinitargs *argsp)
{
#if _WIN32
	return (mtglctx *)mtgl_ctx_create_win32((struct mtglwin_win32 *)win, ver_major, ver_minor, argsp);
#endif
}

mtglctx *
mtgl_ctx_clone(mtglctx *ctx)
{
#if _WIN32
	return (mtglctx *)mtgl_ctx_clone_win32((struct mtglctx_win32 *)ctx);
#endif
}

void
mtgl_ctx_acquire(mtglctx *ctx)
{
#if _WIN32
	mtgl_ctx_acquire_win32((struct mtglctx_win32 *)ctx);
#endif
}

int
mtgl_ctx_try_acquire(mtglctx *ctx)
{
#if _WIN32
	mtgl_ctx_try_acquire_win32((struct mtglctx_win32 *)ctx);
#endif
}

void
mtgl_ctx_release(mtglctx *ctx)
{
#if _WIN32
	mtgl_ctx_release_win32((struct mtglctx_win32 *)ctx);
#endif
}

void
mtgl_ctx_set_swap_interval(mtglctx *ctx, int interval)
{
#if _WIN32
	mtgl_ctx_set_swap_interval_win32((struct mtglctx_win32 *)ctx, interval);
#endif
}

void
mtgl_ctx_destroy(mtglctx *ctx)
{
#if _WIN32
	mtgl_ctx_destroy_win32((struct mtglctx_win32 *)ctx);
#endif
}

void *
mtgl_ctx_get_proc(const char *name)
{
#if _WIN32
	return mtgl_ctx_get_proc_win32(name);
#endif
}
