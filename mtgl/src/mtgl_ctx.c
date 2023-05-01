#include "mtgl_ctx.h"

#include "GL/glcorearb.h"

#if _WIN32
#include "mtgl_ctx_win32.h"
#include "GL/wglext.h"
#elif __APPLE__
#include "mtgl_ctx_cocoa.h"
#endif

void
mtgl_ctx_get_default_init_args(mtglctxinitargs *args)
{
	args->profile = mtgl_profile_core;

	args->color_format = mtgl_color_format_rgba;
	args->red_bits = 8;
	args->green_bits = 8;
	args->blue_bits = 8;
	args->alpha_bits = 8;

	args->accum_red_bits = 0;
	args->accum_green_bits = 0;
	args->accum_blue_bits = 0;
	args->accum_alpha_bits = 0;

	args->depth_bits = 24;
	args->stencil_bits = 8;

	args->double_buffer = 1;
	args->allow_sampling = 0;
	args->sample_count = 0;
}

mtglctx *
mtgl_ctx_create(mtglwin *win, int ver_major, int ver_minor, int policy, mtglctxinitargs *args)
{
	mtglctx *ctx = 0;
	mtgllock *statelock = 0;
	mtglcondition *condition = 0;
	mtglctxinitargs args2;

	/* a window is required to make a context */
	if (!win)
		return 0;
	
	/* check if policy is valid */
	switch (policy)
	{
	case mtgl_ctx_policy_default:
	case mtgl_ctx_policy_use_scheduler:
	case mtgl_ctx_policy_one_thread:
		break;
	default:
		return 0;
	}

	/* fetch default context creation hints */
	if (!args)
	{
		mtgl_ctx_get_default_init_args(&args2);
		args = &args2;
	}

	condition = mtgl_condition_create();
	if (!condition)
		return 0;

#if _WIN32
	ctx = (mtglctx *)mtgl_ctx_create_win32((struct mtglwin_win32 *)win, ver_major, ver_minor, args);
#elif __APPLE__
	ctx = (mtglctx *)mtgl_ctx_create_cocoa((struct mtglwin_cocoa *)win, ver_major, ver_minor, args);
#endif

	if (ctx)
	{
		ctx->condition = condition;
		ctx->policy = policy;
	}

	return ctx;
}

mtglctx *
mtgl_ctx_clone(mtglctx *ctx)
{
	mtglctx *nctx = 0;
	mtglcondition *condition = 0;

	condition = mtgl_condition_create();
	if (!condition)
		return 0;

#if _WIN32
	nctx = (mtglctx *)mtgl_ctx_clone_win32((struct mtglctx_win32 *)ctx);
#elif __APPLE__
	nctx = (mtglctx *)mtgl_ctx_clone_cocoa((struct mtglctx_cocoa *)ctx);
#endif

	if (ctx)
		nctx->condition = condition;

	return nctx;
}

void
mtgl_ctx_acquire(mtglctx *ctx)
{
	mtgl_lock_acquire(ctx->lock);
	ctx->nesting++;
	if (ctx->nesting == 1)
#if _WIN32
		mtgl_ctx_acquire_win32((struct mtglctx_win32 *)ctx);
#elif __APPLE__
		mtgl_ctx_acquire_cocoa((struct mtglctx_cocoa *)ctx);
#endif
}

int
mtgl_ctx_try_acquire(mtglctx *ctx)
{
	int r = mtgl_lock_try_acquire(ctx->lock);
	if (!r) return 0;

	/* lock acquired */

	ctx->nesting++;
	if (ctx->nesting == 1)
#if _WIN32
		mtgl_ctx_acquire_win32((struct mtglctx_win32 *)ctx);
#elif __APPLE__
		mtgl_ctx_try_acquire_cocoa((struct mtglctx_cocoa *)ctx);
#else
		return 0;
#endif

	return 1;
}

void
mtgl_ctx_release(mtglctx *ctx)
{
	ctx->nesting--;

	if (ctx->nesting == 0)
#if _WIN32
		mtgl_ctx_release_win32((struct mtglctx_win32 *)ctx);
#elif __APPLE__
		mtgl_ctx_release_cocoa((struct mtglctx_cocoa *)ctx);
#endif

	mtgl_lock_release(ctx->lock);
}

int
mtgl_ctx_sched(mtglctx *ctx, int try_acquire)
{
	// TODO
}

void
mtgl_ctx_set_swap_interval(mtglctx *ctx, int interval)
{
#if _WIN32
	mtgl_ctx_set_swap_interval_win32((struct mtglctx_win32 *)ctx, interval);
#elif __APPLE__
	mtgl_ctx_set_swap_interval_cocoa((struct mtglctx_cocoa *)ctx, interval);
#endif
}

void
mtgl_ctx_destroy(mtglctx *ctx)
{
	if (!ctx) return;

	mtgl_condition_destroy(ctx->condition);

#if _WIN32
	mtgl_ctx_destroy_win32((struct mtglctx_win32 *)ctx);
#elif __APPLE__
	mtgl_ctx_destroy_cocoa((struct mtglctx_cocoa *)ctx);
#endif
}

void *
mtgl_ctx_get_proc(const char *name)
{
#if _WIN32
	return mtgl_ctx_get_proc_win32(name);
#elif __APPLE__
	return mtgl_ctx_get_proc_cocoa(name);
#else
	return 0;
#endif
}
