#include "glwin_private.h"

#include <glad/glad.h>

#include <GL/glcorearb.h>
#include <GL/wglext.h>

struct glctx
{
	glwin *win;
	HGLRC hglrc;
	gllock *lock;
	int ver_major, ver_minor;
	int nesting;
};

static int gl_refs = 0;
static int gl_ver_major = 0, gl_ver_minor = 0;
static HMODULE hGLModule = 0;
static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = 0;
static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = 0;
static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = 0;

/* return address of OpenGL function */
static void *
glctx_get_proc(const char *name)
{
	void *proc;
	proc = wglGetProcAddress(name);
	if (!proc) proc = GetProcAddress(hGLModule, name);
	return proc;
}

/* initialize necessary functions to create an OpenGL context */
static int
glctx_init_funcs(HINSTANCE hInstance)
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
	wglChoosePixelFormatARB = glctx_get_proc("wglChoosePixelFormatARB");
	wglCreateContextAttribsARB = glctx_get_proc("wglCreateContextAttribsARB");
	wglSwapIntervalEXT = glctx_get_proc("wglSwapIntervalEXT");

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

glctx *
glctx_create(glwin *win, int ver_major, int ver_minor)
{
	HINSTANCE hInstance;
	glctx *ctx = 0;
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormat;
	int iOk;
	int iPixelFormatARB;
	UINT uiPixelFormatsFound;

	const float afAttributes[] = { 0, 0 };
	const int aiPixelAttributes[] = {
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
		WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB,     24,
		WGL_DEPTH_BITS_ARB,     24,
		WGL_STENCIL_BITS_ARB,   8,
		0
	};

	GLint aiContextAttributes[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, ver_major,
		WGL_CONTEXT_MINOR_VERSION_ARB, ver_minor,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};

	/* main context already exists */
	if (win->main) return 0;

	hInstance = GetModuleHandleA(NULL);

	/* get necessary wgl funcs to create an instance */
	if (gl_refs == 0 && !glctx_init_funcs(hInstance)) return 0;

	ctx = calloc(1, sizeof(glctx));
	if (!ctx) return 0;

	gllock_acquire(mtgl_get_lock());

	/* create a lock to synchronize on the context */
	ctx->lock = gllock_create();
	if (!ctx->lock) goto failure;

	if (!wglChoosePixelFormatARB(win->hdc, aiPixelAttributes, NULL, 1, &iPixelFormatARB, &uiPixelFormatsFound)) goto failure;
	if (uiPixelFormatsFound == 0) goto failure;

	/* set the device context's pixel format */
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;

	iPixelFormat = ChoosePixelFormat(win->hdc, &pfd);
	if (!iPixelFormat) goto failure;

	iOk = DescribePixelFormat(win->hdc, iPixelFormat, sizeof(pfd), &pfd);
	if (!iOk) goto failure;

	if (!SetPixelFormat(win->hdc, iPixelFormat, &pfd)) goto failure;

	/* create an OpenGL context */
	ctx->hglrc = wglCreateContextAttribsARB(win->hdc, 0, aiContextAttributes);
	if (!ctx->hglrc) goto failure;

	/* load the necessary OpenGL functions, if the version is higher */
	if (ver_major > gl_ver_major || (ver_major == gl_ver_major && ver_minor > gl_ver_minor))
	{
		if (!wglMakeCurrent(win->hdc, ctx->hglrc)) goto failure;
		if (!gladLoadGLLoader(glctx_get_proc)) goto failure;

		gl_ver_minor = ver_minor;
		gl_ver_major = ver_major;

		if (!wglMakeCurrent(NULL, NULL)) goto failure;
	}

	ctx->win = win;
	ctx->ver_major = ver_major;
	ctx->ver_minor = ver_minor;

	gl_refs++;

	gllock_release(mtgl_get_lock());
	return ctx;

failure:

	if (ctx)
	{
		if (ctx->hglrc) wglDeleteContext(ctx->hglrc);
		if (ctx->lock) gllock_destroy(ctx->lock);
		free(ctx);
	}

	if (gl_refs == 0)
	{
		FreeLibrary(hGLModule);
		hGLModule = 0;
		gl_ver_major = 0;
		gl_ver_minor = 0;
		hGLModule = 0;
		wglChoosePixelFormatARB = 0;
		wglCreateContextAttribsARB = 0;
		wglSwapIntervalEXT = 0;
	}

	gllock_release(mtgl_get_lock());
	return 0;
}

glctx *
glctx_clone(glctx *ctx)
{
	glctx *cloned;
	GLint aiContextAttributes[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, -1,
		WGL_CONTEXT_MINOR_VERSION_ARB, -1,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};

	gllock_acquire(mtgl_get_lock());
	gllock_acquire(ctx->lock);

	aiContextAttributes[1] = ctx->ver_major;
	aiContextAttributes[3] = ctx->ver_minor;

	cloned = calloc(1, sizeof(glctx));
	if (!cloned) goto failure;

	cloned->lock = gllock_create();
	if (!cloned->lock) goto failure;

	cloned->win = ctx->win;
	cloned->ver_major = ctx->ver_major;
	cloned->ver_minor = ctx->ver_minor;

	/* create new context */
	cloned->hglrc = wglCreateContextAttribsARB(ctx->win->hdc, 0, aiContextAttributes);
	if (!cloned->hglrc) goto failure;

	/* share the resources */
	if (!wglShareLists(ctx->hglrc, cloned->hglrc)) goto failure;

	gl_refs++;

	gllock_release(ctx->lock);

	return cloned;
failure:
	if (cloned)
	{
		if (cloned->hglrc) wglDeleteContext(cloned->hglrc);
		if (cloned->lock) gllock_destroy(cloned->lock);

		free(cloned);
	}

	gllock_release(ctx->lock);
	gllock_release(mtgl_get_lock());
	return 0;
}

void
glctx_acquire(glctx *ctx)
{
	gllock_acquire(ctx->lock);
	ctx->nesting++;
	if (ctx->nesting == 1)
		wglMakeCurrent(ctx->win->hdc, ctx->hglrc);
}

void
glctx_release(glctx *ctx)
{
	ctx->nesting--;
	if (ctx->nesting == 0)
		wglMakeCurrent(NULL, NULL);
	gllock_release(ctx->lock);
}

void
glctx_set_swap_interval(glctx *ctx, int interval)
{
	glctx_acquire(ctx);
	wglSwapIntervalEXT(interval);
	glctx_release(ctx);
}

void
glctx_destroy(glctx *ctx)
{
	gllock_acquire(mtgl_get_lock());
	gllock_acquire(ctx->lock);

	wglDeleteContext(ctx->hglrc);

	gllock_release(ctx->lock);

	gl_refs--;
	if (gl_refs == 0)
	{
		FreeLibrary(hGLModule);
		hGLModule = 0;
	}

	gllock_destroy(ctx->lock);
	gllock_release(mtgl_get_lock());

	free(ctx);
}