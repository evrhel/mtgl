#pragma once

#if _WIN32

#include <Windows.h>

#include "mtgl_ctx.h"

#ifdef __cplusplus
extern "C"
{
#endif

	struct mtglwin_win32;

	/* WGL OpenGL context */
	struct mtglctx_win32
	{
		struct mtglctx ctx; // base class
		HGLRC hglrc; // OpenGL rendering context
	};

	struct mtglctx_win32 *mtgl_ctx_create_win32(struct mtglwin_win32 *win, int ver_major, int ver_minor, mtglctxinitargs *argsp);
	struct mtglctx_win32 *mtgl_ctx_clone_win32(struct mtglctx_win32 *ctx);
	void mtgl_ctx_acquire_win32(struct mtglctx_win32 *ctx); // not thread-safe
	void mtgl_ctx_release_win32(struct mtglctx_win32 *ctx); // not thread-safe
	void mtgl_ctx_set_swap_interval_win32(struct mtglctx_win32 *ctx, int interval);
	void mtgl_ctx_destroy_win32(struct mtglctx_win32 *ctx);
	void *mtgl_ctx_get_proc_win32(const char *name);

#ifdef __cplusplus
}
#endif

#endif