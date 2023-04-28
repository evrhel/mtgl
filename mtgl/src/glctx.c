#include "glwin_private.h"

#pragma comment(lib, "opengl32.lib")

#if _WIN32
#include <gl/GL.h>
#elif __APPLE__
#include <OpenGL/gl.h>
#endif

#include "GL/glcorearb.h"
#include "GL/wglext.h"

#define DRAW_TO_WINDOW_VAL 1
#define SUPPORT_OPENGL_VAL 3
#define DOUBLE_BUFFER_VAL 5
#define PIXEL_TYPE_VAL 7
#define RED_BITS_VAL 9
#define GREEN_BITS_VAL 11
#define BLUE_BITS_VAL 13
#define ALPHA_BITS_VAL 15
#define DEPTH_BITS_VAL 17
#define STENCIL_BITS_VAL 19
#define SAMPLE_BUFFERS_VAL 21
#define SAMPLES_VAL 23

#define VER_MAJOR_VAL 1
#define VER_MINOR_VAL 3
#define PROFILE_VAL 5

enum ctxtype
{
	ctxtype_root,
	ctxtype_child
};

struct mtglctx
{
	int type;
	mtglwin *win;
	HGLRC hglrc;
	mtgllock *lock;
	int ver_major, ver_minor;
	int profile;
	int nesting;
};

static int gl_refs = 0;
static HMODULE hGLModule = 0;
static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = 0;
static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = 0;
static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = 0;

static inline int
get_color_format(int colorformat)
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
get_profile(int profile)
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
mtgl_ctx_init_funcs(HINSTANCE hInstance)
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

void
mtgl_ctx_default_init_args(mtglctxinitargs *args)
{
	args->profile = mtgl_profile_core;

	args->color_format = mtgl_color_format_rgba;
	args->red_bits = 8;
	args->green_bits = 8;
	args->blue_bits = 8;
	args->alpha_bits = 8;

	args->depth_bits = 24;
	args->stencil_bits = 8;

	args->double_buffer = GL_TRUE;
	args->allow_sampling = GL_FALSE;
	args->sample_count = 1;
}

mtglctx *
mtgl_ctx_create(mtglwin *win, int ver_major, int ver_minor, const mtglctxinitargs *args)
{
	HINSTANCE hInstance;
	mtglctx *ctx = 0;
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormat;
	int iOk;
	int iPixelFormatARB;
	UINT uiPixelFormatsFound;

	float afAttributes[] = { 0, 0 };
	int aiPixelAttributes[] = {
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,			// 0
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,			// 2
		WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,			// 4
		WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,	// 6
		WGL_RED_BITS_ARB,		8,					// 8
		WGL_GREEN_BITS_ARB,		8,					// 10
		WGL_BLUE_BITS_ARB,		8,					// 12
		WGL_ALPHA_BITS_ARB,		8,					// 14
		WGL_DEPTH_BITS_ARB,     24,					// 16
		WGL_STENCIL_BITS_ARB,   8,					// 18
		WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,			// 20
		WGL_SAMPLES_ARB,		16,					// 22
		0
	};

	GLint aiContextAttributes[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, -1,
		WGL_CONTEXT_MINOR_VERSION_ARB, -1,
		WGL_CONTEXT_PROFILE_MASK_ARB, -1,
		0
	};

	/* main context already exists */
	if (win->main) return 0;

	/* use default args if not supplied */
	if (!args) mtgl_ctx_default_init_args(args);

	/* setup context attributes */
	aiContextAttributes[VER_MAJOR_VAL] = ver_major;
	aiContextAttributes[VER_MINOR_VAL] = ver_minor;
	aiContextAttributes[PROFILE_VAL] = get_profile(args->profile);

	/* setup pixel attributes */
	aiPixelAttributes[DOUBLE_BUFFER_VAL] = args->double_buffer;
	aiPixelAttributes[PIXEL_TYPE_VAL] = get_color_format(args->color_format);
	aiPixelAttributes[RED_BITS_VAL] = args->red_bits;
	aiPixelAttributes[GREEN_BITS_VAL] = args->green_bits;
	aiPixelAttributes[BLUE_BITS_VAL] = args->blue_bits;
	aiPixelAttributes[ALPHA_BITS_VAL] = args->alpha_bits;
	aiPixelAttributes[DEPTH_BITS_VAL] = args->depth_bits;
	aiPixelAttributes[STENCIL_BITS_VAL] = args->stencil_bits;
	aiPixelAttributes[SAMPLE_BUFFERS_VAL] = args->allow_sampling;
	aiPixelAttributes[SAMPLES_VAL] = args->sample_count;

	hInstance = GetModuleHandleA(NULL);

	/* get necessary wgl funcs to create an instance */
	if (gl_refs == 0 && !glctx_init_funcs(hInstance)) return 0;

	ctx = calloc(1, sizeof(mtglctx));
	if (!ctx) return 0;

	mtgl_lock_acquire(mtgl_get_lock());

	/* create a lock to synchronize on the context */
	ctx->lock = mtgl_lock_create();
	if (!ctx->lock) goto failure;

	if (!wglChoosePixelFormatARB(win->hdc, aiPixelAttributes, NULL, 1, &iPixelFormatARB, &uiPixelFormatsFound)) goto failure;
	if (uiPixelFormatsFound == 0) goto failure;

	/* set the device context's pixel format */
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);

	iOk = DescribePixelFormat(win->hdc, iPixelFormatARB, sizeof(pfd), &pfd);
	if (!iOk) goto failure;

	if (!SetPixelFormat(win->hdc, iPixelFormatARB, &pfd)) goto failure;

	/* create an OpenGL context */
	ctx->hglrc = wglCreateContextAttribsARB(win->hdc, 0, aiContextAttributes);
	if (!ctx->hglrc) goto failure;

	ctx->type = ctxtype_root;
	ctx->win = win;
	ctx->ver_major = ver_major;
	ctx->ver_minor = ver_minor;
	ctx->profile = aiContextAttributes[PROFILE_VAL];

	gl_refs++;

	mtgl_lock_release(mtgl_get_lock());
	return ctx;

failure:

	if (ctx)
	{
		if (ctx->hglrc) wglDeleteContext(ctx->hglrc);
		if (ctx->lock) mtgl_lock_destroy(ctx->lock);
		free(ctx);
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

mtglctx *
mtgl_ctx_clone(mtglctx *ctx)
{
	mtglctx *cloned;
	GLint aiContextAttributes[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, -1,
		WGL_CONTEXT_MINOR_VERSION_ARB, -1,
		WGL_CONTEXT_PROFILE_MASK_ARB, -1,
		0
	};

	mtgl_lock_acquire(mtgl_get_lock());
	mtgl_lock_acquire(ctx->lock);

	aiContextAttributes[VER_MAJOR_VAL] = ctx->ver_major;
	aiContextAttributes[VER_MINOR_VAL] = ctx->ver_minor;
	aiContextAttributes[PROFILE_VAL] = ctx->profile;

	cloned = calloc(1, sizeof(mtglctx));
	if (!cloned) goto failure;

	cloned->lock = mtgl_lock_create();
	if (!cloned->lock) goto failure;

	cloned->type = ctxtype_child;
	cloned->win = ctx->win;
	cloned->ver_major = ctx->ver_major;
	cloned->ver_minor = ctx->ver_minor;

	/* create new context */
	cloned->hglrc = wglCreateContextAttribsARB(ctx->win->hdc, 0, aiContextAttributes);
	if (!cloned->hglrc) goto failure;

	/* share the resources */
	if (!wglShareLists(ctx->hglrc, cloned->hglrc)) goto failure;

	gl_refs++;

	mtgl_lock_release(ctx->lock);

	return cloned;
failure:
	if (cloned)
	{
		if (cloned->hglrc) wglDeleteContext(cloned->hglrc);
		if (cloned->lock) mtgl_lock_destroy(cloned->lock);

		free(cloned);
	}

	mtgl_lock_release(ctx->lock);
	mtgl_lock_release(mtgl_get_lock());
	return 0;
}

void
mtgl_ctx_acquire(mtglctx *ctx)
{
	mtgl_lock_acquire(ctx->lock);
	ctx->nesting++;
	if (ctx->nesting == 1)
		wglMakeCurrent(ctx->win->hdc, ctx->hglrc);
}

int
mtgl_ctx_try_acquire(mtglctx *ctx)
{
	if (!mtgl_lock_try_acquire(ctx->lock)) return 0;

	ctx->nesting++;
	if (ctx->nesting == 1)
		wglMakeCurrent(ctx->win->hdc, ctx->hglrc);

	return 1;
}

void
mtgl_ctx_release(mtglctx *ctx)
{
	ctx->nesting--;
	if (ctx->nesting == 0)
		wglMakeCurrent(NULL, NULL);
	mtgl_lock_release(ctx->lock);
}

void
mtgl_ctx_set_swap_interval(mtglctx *ctx, int interval)
{
	mtgl_ctx_acquire(ctx);
	wglSwapIntervalEXT(interval);
	mtgl_ctx_release(ctx);
}

void
mtgl_ctx_destroy(mtglctx *ctx)
{
	mtgl_lock_acquire(mtgl_get_lock());
	mtgl_lock_acquire(ctx->lock);

	wglDeleteContext(ctx->hglrc);

	mtgl_lock_release(ctx->lock);

	gl_refs--;
	if (gl_refs == 0)
	{
		FreeLibrary(hGLModule);
		hGLModule = 0;
	}

	mtgl_lock_destroy(ctx->lock);
	mtgl_lock_release(mtgl_get_lock());

	free(ctx);
}

void *
mtgl_ctx_get_proc(const char *name)
{
	void *proc;
	proc = wglGetProcAddress(name);
	if (!proc && hGLModule) proc = GetProcAddress(hGLModule, name);
	return proc;
}
