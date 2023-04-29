#if _WIN32

#include "mtgl_ctx_win32.h"
#include "mtgl_win_win32.h"

#pragma comment(lib, "opengl32.lib")

#include <gl/GL.h>
#include "GL/glcorearb.h"
#include "GL/wglext.h"

#define DRAW_TO_WINDOW_VAL 1
#define SUPPORT_OPENGL_VAL 3
#define DOUBLE_BUFFER_VAL 5
#define PIXEL_TYPE_VAL 7
#define COLOR_BITS_VAL 9
#define ALPHA_BITS_VAL 11
#define DEPTH_BITS_VAL 13
#define STENCIL_BITS_VAL 15
#define SAMPLE_BUFFERS_VAL 17
#define SAMPLES_VAL 19

#define VER_MAJOR_VAL 1
#define VER_MINOR_VAL 3
#define PROFILE_VAL 5

static int gl_refs = 0;
static HMODULE hGLModule = 0;
static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = 0;
static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = 0;
static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = 0;

static inline int
get_color_format_win32(int colorformat)
{
	switch (colorformat)
	{
	case mtgl_color_format_any:
	case mtgl_color_format_rgba:
		return WGL_TYPE_RGBA_ARB;
	default:
		return 0;
	}
}

static inline int
get_profile_win32(int profile)
{
	switch (profile)
	{
	case mtgl_profile_core:
		return WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
	case mtgl_profile_compatability:
		return WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
	default:
		return 0;
	}
}

/* initialize necessary functions to create an OpenGL context */
static int
mtgl_ctx_init_funcs_win32(HINSTANCE hInstance)
{
	const char class_name[] = "dummyclass";
	WNDCLASSA wc;
	HWND hwnd = 0;
	HDC hdc = 0;
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormat;
	int iOk;
	HGLRC hglrc = 0;
	ATOM atom = 0;

	/* register window class */
	ZeroMemory(&wc, sizeof(wc));
	wc.lpfnWndProc = DefWindowProcA;
	wc.hInstance = hInstance;
	wc.lpszClassName = class_name;
	wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	atom = RegisterClassA(&wc);
	if (!atom) goto failure;

	/* create window */
	hwnd = CreateWindowExA(
		0,
		class_name,
		NULL,
		0,
		CW_USEDEFAULT, CW_USEDEFAULT,
		1, 1,
		NULL,
		NULL,
		hInstance,
		NULL
	);
	if (!hwnd) goto failure;

	/* get device context */
	hdc = GetDC(hwnd);
	if (!hdc) goto failure;

	/* get pixel format */
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;

	iPixelFormat = ChoosePixelFormat(hdc, &pfd);
	if (!iPixelFormat) goto failure;

	/* set the pixel format */
	iOk = DescribePixelFormat(hdc, iPixelFormat, sizeof(pfd), &pfd);
	if (!iOk) goto failure;

	if (!SetPixelFormat(hdc, iPixelFormat, &pfd)) goto failure;

	/* create the opengl context and make it current */
	hglrc = wglCreateContext(hdc);
	if (!hglrc) goto failure;

	if (!wglMakeCurrent(hdc, hglrc)) goto failure;

	/* load OpenGL module */
	hGLModule = LoadLibraryA("OPENGL32.DLL");
	if (!hGLModule) goto failure;

	/* load library functions */
	wglChoosePixelFormatARB = mtgl_ctx_get_proc("wglChoosePixelFormatARB");
	wglCreateContextAttribsARB = mtgl_ctx_get_proc("wglCreateContextAttribsARB");
	wglSwapIntervalEXT = mtgl_ctx_get_proc("wglSwapIntervalEXT");

	if (!wglChoosePixelFormatARB) goto failure;
	if (!wglCreateContextAttribsARB) goto failure;
	if (!wglSwapIntervalEXT) goto failure;

	/* cleanup temporary resources */
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hglrc);
	ReleaseDC(hwnd, hdc);
	DestroyWindow(hwnd);
	UnregisterClassA(class_name, hInstance);

	return 1;
failure:

	wglSwapIntervalEXT = 0;
	wglCreateContextAttribsARB = 0;
	wglChoosePixelFormatARB = 0;

	if (hGLModule)
	{
		FreeLibrary(hGLModule);
		hGLModule = 0;
	}

	if (hglrc) wglDeleteContext(hglrc);
	if (hdc) ReleaseDC(hwnd, hdc);
	if (hwnd) DestroyWindow(hwnd);
	if (atom) UnregisterClassA(class_name, hInstance);

	return 0;
}

struct mtglctx_win32 *
mtgl_ctx_create_win32(struct mtglwin_win32 *win, int ver_major, int ver_minor, mtglctxinitargs *argsp)
{
	HINSTANCE hInstance;
	struct mtglctx_win32 *ctxw32 = 0;
	PIXELFORMATDESCRIPTOR pfd;
	int iOk;
	int iPixelFormatARB;
	UINT uiPixelFormatsFound;
	mtglctxinitargs args;

	float afAttributes[] = { 0, 0 };
	int aiPixelAttributes[] = {
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,			// 0
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,			// 2
		WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,			// 4
		WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,	// 6
		WGL_COLOR_BITS_ARB,		24,					// 8
		WGL_ALPHA_BITS_ARB,		8,					// 10
		WGL_DEPTH_BITS_ARB,     24,					// 12
		WGL_STENCIL_BITS_ARB,   8,					// 14
		WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,			// 16
		WGL_SAMPLES_ARB,		16,					// 18
		0
	};

	GLint aiContextAttributes[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, -1,
		WGL_CONTEXT_MINOR_VERSION_ARB, -1,
		WGL_CONTEXT_PROFILE_MASK_ARB, -1,
		0
	};

	/* main context already exists */
	if (win->win.main) return 0;

	/* use default args if not supplied */
	if (!argsp) mtgl_ctx_get_default_init_args(&args);
	else args = *argsp;

	/* setup context attributes */
	aiContextAttributes[VER_MAJOR_VAL] = ver_major;
	aiContextAttributes[VER_MINOR_VAL] = ver_minor;
	aiContextAttributes[PROFILE_VAL] = get_profile_win32(args.profile);

	/* setup pixel attributes */
	aiPixelAttributes[DOUBLE_BUFFER_VAL] = args.double_buffer;
	aiPixelAttributes[PIXEL_TYPE_VAL] = get_color_format_win32(args.color_format);
	aiPixelAttributes[COLOR_BITS_VAL] = args.red_bits + args.green_bits + args.blue_bits;
	aiPixelAttributes[ALPHA_BITS_VAL] = args.alpha_bits;
	aiPixelAttributes[DEPTH_BITS_VAL] = args.depth_bits;
	aiPixelAttributes[STENCIL_BITS_VAL] = args.stencil_bits;
	aiPixelAttributes[SAMPLE_BUFFERS_VAL] = args.allow_sampling;
	aiPixelAttributes[SAMPLES_VAL] = args.sample_count;

	hInstance = GetModuleHandleA(NULL);

	/* get necessary wgl funcs to create an instance */
	if (gl_refs == 0 && !mtgl_ctx_init_funcs_win32(hInstance)) return 0;

	ctxw32 = calloc(1, sizeof(struct mtglctx_win32));
	if (!ctxw32) return 0;

	mtgl_lock_acquire(mtgl_get_lock());

	/* create a lock to synchronize on the context */
	ctxw32->ctx.lock = mtgl_lock_create();
	if (!ctxw32->ctx.lock) goto failure;

	if (!wglChoosePixelFormatARB(win->hdc, aiPixelAttributes, NULL, 1, &iPixelFormatARB, &uiPixelFormatsFound)) goto failure;
	if (uiPixelFormatsFound == 0) goto failure;

	/* set the device context's pixel format */
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);

	iOk = DescribePixelFormat(win->hdc, iPixelFormatARB, sizeof(pfd), &pfd);
	if (!iOk) goto failure;

	if (!SetPixelFormat(win->hdc, iPixelFormatARB, &pfd)) goto failure;

	/* create an OpenGL context */
	ctxw32->hglrc = wglCreateContextAttribsARB(win->hdc, 0, aiContextAttributes);
	if (!ctxw32->hglrc) goto failure;

	ctxw32->ctx.type = ctxtype_root;
	ctxw32->ctx.win = (mtglwin *)win;
	ctxw32->ctx.ver_major = ver_major;
	ctxw32->ctx.ver_minor = ver_minor;
	ctxw32->ctx.profile = aiContextAttributes[PROFILE_VAL];

	gl_refs++;

	mtgl_lock_release(mtgl_get_lock());
	return ctxw32;

failure:

	if (ctxw32)
	{
		if (ctxw32->hglrc) wglDeleteContext(ctxw32->hglrc);
		if (ctxw32->ctx.lock) mtgl_lock_destroy(ctxw32->ctx.lock);
		free(ctxw32);
	}

	if (gl_refs == 0)
	{
		FreeLibrary(hGLModule);
		hGLModule = 0;
		hGLModule = 0;
		wglChoosePixelFormatARB = 0;
		wglCreateContextAttribsARB = 0;
		wglSwapIntervalEXT = 0;
	}

	mtgl_lock_release(mtgl_get_lock());
	return 0;
}

struct mtglctx_win32 *
mtgl_ctx_clone_win32(struct mtglctx_win32 *ctx)
{
	struct mtglctx_win32 *clonedw32;
	struct mtglwin_win32 *winw32 = (struct mtglwin_win32 *)ctx->ctx.win;
	GLint aiContextAttributes[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, -1,
		WGL_CONTEXT_MINOR_VERSION_ARB, -1,
		WGL_CONTEXT_PROFILE_MASK_ARB, -1,
		0
	};

	mtgl_lock_acquire(mtgl_get_lock());
	mtgl_lock_acquire(ctx->ctx.lock);

	aiContextAttributes[VER_MAJOR_VAL] = ctx->ctx.ver_major;
	aiContextAttributes[VER_MINOR_VAL] = ctx->ctx.ver_minor;
	aiContextAttributes[PROFILE_VAL] = ctx->ctx.profile;

	clonedw32 = calloc(1, sizeof(struct mtglctx_win32));
	if (!clonedw32) goto failure;

	clonedw32->ctx.lock = mtgl_lock_create();
	if (!clonedw32->ctx.lock) goto failure;

	clonedw32->ctx.type = ctxtype_child;
	clonedw32->ctx.win = ctx->ctx.win;
	clonedw32->ctx.ver_major = ctx->ctx.ver_major;
	clonedw32->ctx.ver_minor = ctx->ctx.ver_minor;

	/* create new context */
	clonedw32->hglrc = wglCreateContextAttribsARB(winw32->hdc, 0, aiContextAttributes);
	if (!clonedw32->hglrc) goto failure;

	/* share the resources */
	if (!wglShareLists(ctx->hglrc, clonedw32->hglrc)) goto failure;

	gl_refs++;

	mtgl_lock_release(ctx->ctx.lock);

	return clonedw32;
failure:
	if (clonedw32)
	{
		if (clonedw32->hglrc) wglDeleteContext(clonedw32->hglrc);
		if (clonedw32->ctx.lock) mtgl_lock_destroy(clonedw32->ctx.lock);

		free(clonedw32);
	}

	mtgl_lock_release(ctx->ctx.lock);
	mtgl_lock_release(mtgl_get_lock());
	return 0;
}

void
mtgl_ctx_acquire_win32(struct mtglctx_win32 *ctx)
{
	struct mtglwin_win32 *winw32 = (struct mtglwin_win32 *)ctx->ctx.win;

	mtgl_lock_acquire(ctx->ctx.lock);
	ctx->ctx.nesting++;
	if (ctx->ctx.nesting == 1)
		wglMakeCurrent(winw32->hdc, ctx->hglrc);
}

int
mtgl_ctx_try_acquire_win32(struct mtglctx_win32 *ctx)
{
	struct mtglwin_win32 *winw32 = (struct mtglwin_win32 *)ctx->ctx.win;

	if (!mtgl_lock_try_acquire(ctx->ctx.lock)) return 0;

	ctx->ctx.nesting++;
	if (ctx->ctx.nesting == 1)
		wglMakeCurrent(winw32->hdc, ctx->hglrc);

	return 1;
}

void
mtgl_ctx_release_win32(struct mtglctx_win32 *ctx)
{
	ctx->ctx.nesting--;
	if (ctx->ctx.nesting == 0)
		wglMakeCurrent(NULL, NULL);
	mtgl_lock_release(ctx->ctx.lock);
}

void
mtgl_ctx_set_swap_interval_win32(struct mtglctx_win32 *ctx, int interval)
{
	mtgl_ctx_acquire_win32(ctx);
	wglSwapIntervalEXT(interval);
	mtgl_ctx_release_win32(ctx);
}

void
mtgl_ctx_destroy_win32(struct mtglctx_win32 *ctx)
{
	mtgl_lock_acquire(mtgl_get_lock());
	mtgl_lock_acquire(ctx->ctx.lock);

	wglDeleteContext(ctx->hglrc);

	mtgl_lock_release(ctx->ctx.lock);

	gl_refs--;
	if (gl_refs == 0)
	{
		FreeLibrary(hGLModule);
		hGLModule = 0;
	}

	mtgl_lock_destroy(ctx->ctx.lock);
	mtgl_lock_release(mtgl_get_lock());

	free(ctx);
}

void *
mtgl_ctx_get_proc_win32(const char *name)
{
	void *proc;
	proc = wglGetProcAddress(name);
	if (!proc && hGLModule) proc = GetProcAddress(hGLModule, name);
	return proc;
}

#endif