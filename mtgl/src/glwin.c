#include "glwin_private.h"

static const char win_class_name[] = "glwin";
static int win_class_refs = 0;

static LRESULT CALLBACK
glwin_win_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	glwin *win;

	win = GetWindowLongPtrA(hwnd, GWLP_USERDATA);

	switch (uMsg)
	{
	case WM_NCCREATE: {
		SetWindowLongPtrA(hwnd, GWLP_USERDATA, ((CREATESTRUCTA *)lParam)->lpCreateParams);
		return TRUE;
	}
	case WM_CLOSE: {
		win->should_close = 1;
		return 0;
	}
	case WM_SIZE: {
		win->width = LOWORD(lParam);
		win->height = HIWORD(lParam);
		win->was_resized = 1;
		return 0;
	}
	}

	return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

glwin *
glwin_create(int width, int height)
{
	HINSTANCE hInstance;
	glwin *win = 0;
	WNDCLASSA wc;
	ATOM atom = 0;
	DWORD dwStyle;
	
	hInstance = GetModuleHandleA(NULL);

	gllock_acquire(mtgl_get_lock());

	/* register window class if necessary */
	if (win_class_refs == 0)
	{
		ZeroMemory(&wc, sizeof(wc));
		wc.lpfnWndProc = glwin_win_proc;
		wc.hInstance = hInstance;
		wc.lpszClassName = win_class_name;
		wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		wc.hIcon = NULL;

		atom = RegisterClassA(&wc);
		if (!atom) 0;
	}

	win = calloc(1, sizeof(glwin));
	if (!win) goto failure;

	win->lock = gllock_create();
	if (!win->lock) goto failure;

	/* create the window */
	dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	win->hwnd = CreateWindowExA(
		0,
		win_class_name,
		"glwindow",
		dwStyle,
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height,
		NULL,
		NULL,
		hInstance,
		win
	);
	if (!win->hwnd) goto failure;

	/* get the device context */
	win->hdc = GetDC(win->hwnd);
	if (!win->hdc) goto failure;

	win_class_refs++;
	gllock_release(mtgl_get_lock());
	return win;

failure:

	if (win)
	{
		if (win->hdc) ReleaseDC(win->hwnd, win->hdc);
		if (win->hwnd) DestroyWindow(win->hwnd);
		if (win->lock) gllock_destroy(win->lock);

		free(win);
	}

	if (atom) UnregisterClassA(win_class_name, hInstance);


	gllock_release(mtgl_get_lock());
	return 0;
}

void
glwin_show_window(glwin *win, int shown)
{
	ShowWindow(win->hwnd, shown ? SW_SHOW : SW_HIDE);
}

int
glwin_should_close(glwin *win)
{
	return win->should_close;
}

void
glwin_poll_events(glwin *win)
{
	BOOL bResult;
	MSG msg;

	win->was_resized = 0;

	while ((bResult = PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
}

int
glwin_was_resized(glwin *win)
{
	return win->was_resized;
}

void
glwin_swap_buffers(glwin *win)
{
	SwapBuffers(win->hdc);
}

void
glwin_get_size(glwin *win, int *const width, int *const height)
{
	*width = win->width;
	*height = win->height;
}

void
glwin_destroy(glwin *win)
{
	if (!win) return;

	gllock_acquire(mtgl_get_lock());

	ReleaseDC(win->hwnd, win->hdc);
	DestroyWindow(win->hwnd);

	free(win);

	win_class_refs--;
	if (win_class_refs == 0) UnregisterClassA(win_class_name, GetModuleHandleA(NULL));

	gllock_release(mtgl_get_lock());
}