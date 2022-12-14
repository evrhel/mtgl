#include "glwin_private.h"
#include "mtgl_input_private.h"

#include <assert.h>

static const char win_class_name[] = "glwin";
static int win_class_refs = 0;

static int
warn(glwin *win, const char *msg)
{
	return 1;
}

static int
error(glwin *win, const char *msg)
{
	return 2;
}

int
push_event(glwin *win, struct event *evt)
{
	if (win->event_last == GLWIN_EVENT_QUEUE_SIZE)
		return warn(win, "event queue full");

	evt->win = win;
	win->events[win->event_last] = *evt;
	win->event_last++;

	return 0;
}

/* Windows event handler */
static LRESULT CALLBACK
glwin_win_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	glwin *win;
	UINT cbSize;
	RAWINPUT raw[sizeof(RAWINPUT)];
	RAWMOUSE *rm;
	RAWKEYBOARD *rk;
	USHORT vk;
	struct event event;
	int button_id, button_num;
	LPCREATESTRUCTA lpCreateStruct;
	PDEV_BROADCAST_DEVICEINTERFACE_A pDevInterface;
	HANDLE hDevice;

	win = (glwin *)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

	switch (uMsg)
	{
	case WM_NCCREATE: {
		//SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG)((LPCREATESTRUCTA)lParam)->lpCreateParams);
		return TRUE;
	}
	case WM_CREATE: {
		lpCreateStruct = (LPCREATESTRUCTA)lParam;
		win = lpCreateStruct->lpCreateParams;
		SetWindowLongPtrA(hwnd, GWLP_USERDATA, win);
		win->x = lpCreateStruct->x;
		win->y = lpCreateStruct->y;
		win->width = lpCreateStruct->cx;
		win->height = lpCreateStruct->cy;
		return 0;
	}
	case WM_CLOSE:
	case WM_QUIT: {
		win->should_close = 1;

		event.type = glwin_event_window_event;
		event.data.window_event.event = glwin_window_closing;
		event.data.window_event.param1 = 0;
		event.data.window_event.param2 = 0;
		push_event(win, &event);
		return 0;
	}
	case WM_MOVE: {
		
		event.type = glwin_event_window_event;
		event.data.window_event.event = glwin_window_move;
		event.data.window_event.param1 = LOWORD(lParam);
		event.data.window_event.param2 = HIWORD(lParam);
		push_event(win, &event);

		return 0;
	}
	case WM_SIZE: {
		win->was_resized = 1;

		// resize event
		event.type = glwin_event_resize;
		push_event(win, &event);

		// window event
		event.type = glwin_event_window_event;
		switch (wParam)
		{
		case SIZE_MAXIMIZED:
			event.data.window_event.event = glwin_window_maximize;
			break;
		case SIZE_MINIMIZED:
			event.data.window_event.event = glwin_window_minimize;
			break;
		case SIZE_RESTORED:
			event.data.window_event.event = glwin_window_restore;
			break;
		case SIZE_MAXSHOW:
		case SIZE_MAXHIDE:
		default:
			event.type = 0;
		}

		if (event.type)
		{
			event.data.window_event.param1 = LOWORD(lParam);
			event.data.window_event.param2 = HIWORD(lParam);
			push_event(win, &event);
		}

		return 0;
	}
	case WM_SETFOCUS: {
		win->focused = 1;

		event.type = glwin_event_window_event;
		event.data.window_event.event = glwin_window_changefocus;
		event.data.window_event.param1 = 1;
		event.data.window_event.param2 = 0;
		push_event(win, &event);
		return 0;
	}
	case WM_KILLFOCUS: {
		win->focused = 0;

		event.type = glwin_event_window_event;
		event.data.window_event.event = glwin_window_changefocus;
		event.data.window_event.param1 = 0;
		event.data.window_event.param2 = 0;
		push_event(win, &event);
		return 0;
	}
	case WM_INPUT_DEVICE_CHANGE: {
		handle_input_device_event(win, lParam, wParam == GIDC_ARRIVAL ? mtgl_device_connected : mtgl_device_disconnected);
		return 0;
	}
	case WM_CHAR:
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_SYSCHAR:
	case WM_INPUT: return mtgl_handle_input_message(win, hwnd, uMsg, wParam, lParam);
		break;
	}

	return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

static void
dispatch_events(glwin *win)
{
	struct event *event;
	union event_data *data;
	union callback callback;

	for (event = win->events; event < win->events + win->event_last; event++)
	{
		callback = win->callbacks[event->type];
		if (!callback.ptr)
			continue;

		data = &event->data;
		switch (event->type)
		{
		case glwin_event_resize:
			callback.on_resize(win, win->width, win->height);
			break;
		case glwin_event_mouse_move: {
			callback.on_mouse_move(
				win,
				data->mouse_move.old_mx, data->mouse_move.old_my,
				win->mx, win->my
			);
			break;
		}
		case glwin_event_key: {
			callback.on_key(
				win,
				data->key.key,
				data->key.action,
				data->key.mods
			);
			break;
		}
		case glwin_event_char: {
			callback.on_char(
				win, 
				data->char_.code,
				data->char_.repeat_count,
				data->char_.mods
			);
			break;
		}
		case glwin_event_mouse_button: {
			callback.on_mouse_button(
				win,
				data->mouse_button.button,
				data->mouse_button.action,
				data->mouse_button.mods
			);
			break;
		}
		case glwin_event_window_event: {
			callback.on_window_event(
				win,
				data->window_event.event,
				data->window_event.param1,
				data->window_event.param2
			);
			break;
		}
		case glwin_event_user1:
		case glwin_event_user2:
		case glwin_event_user3:
		case glwin_event_user4:
			callback.on_user_event(win, data->user_event.data);
			break;
		}
	}

	win->event_last = 0;
}

glwin *
glwin_create(const char *title, int width, int height, int flags, int device, void *user_data)
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

	win->flags = flags;

	/* create the window */
	dwStyle = WS_OVERLAPPEDWINDOW;
	win->hwnd = CreateWindowExA(
		0,
		win_class_name,
		title,
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

	if (!QueryPerformanceFrequency(&win->freq)) goto failure;
	QueryPerformanceCounter(&win->start);

	win->user_data = user_data;

	/* register input devices */

	if (flags & glwin_wf_raw_mouse_input)
	{
		win->rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		win->rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
		win->rid[0].dwFlags = RIDEV_NOLEGACY;
		win->rid[0].hwndTarget = win->hwnd;

		if (!RegisterRawInputDevices(&win->rid[0], 1, sizeof(win->rid[0])))
			goto failure;
	}

	if (flags & glwin_wf_raw_keyboard_input)
	{
		win->rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
		win->rid[1].usUsage = HID_USAGE_GENERIC_KEYBOARD;
		win->rid[1].dwFlags = RIDEV_NOLEGACY;
		win->rid[1].hwndTarget = win->hwnd;

		if (!RegisterRawInputDevices(&win->rid[1], 1, sizeof(win->rid[1])))
			goto failure;
	}

	win->hHeap = GetProcessHeap();

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
glwin_set_title(glwin *win, const char *title)
{
	gllock_acquire(win->lock);
	SetWindowTextA(win->hwnd, title);
	gllock_release(win->lock);
}

void *
glwin_get_user_data(glwin *win)
{
	return win->user_data;
}

void
glwin_show_window(glwin *win, int shown)
{
	RECT rc;

	gllock_acquire(win->lock);
	ShowWindow(win->hwnd, shown ? SW_SHOW : SW_HIDE);

	GetWindowRect(win->hwnd, &rc);
	win->x = rc.left;
	win->y = rc.top;

	gllock_release(win->lock);
}

int
glwin_should_close(glwin *win)
{
	int should_close;

	gllock_acquire(win->lock);
	should_close = win->should_close;
	gllock_release(win->lock);

	return should_close;
}

void
glwin_set_should_close(glwin *win, int should_close)
{
	gllock_acquire(win->lock);
	win->should_close = should_close;
	gllock_release(win->lock);
}

void
glwin_queue_event(glwin *win, enum win_event_type type, void *data)
{
	struct event evt;

	evt.type = type;
	evt.data.user_event.data = data;

	gllock_acquire(win->lock);
	push_event(win, &evt);
	gllock_release(win->lock);
}

void
glwin_poll_events(glwin *win)
{
	BOOL bResult;
	MSG msg;

	RECT rc;

	gllock_acquire(win->lock);

	win->was_resized = 0;

	win->dmx = 0;
	win->dmy = 0;
	win->wheel = 0;

	/* handle all waiting messages */
	while ((bResult = PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}

	/* window was resized, need to set the size */
	if (win->was_resized)
	{
		GetClientRect(win->hwnd, &rc);
		win->width = rc.right - rc.left;
		win->height = rc.bottom - rc.top;
	}

	/* call user callbacks */
	dispatch_events(win);

	gllock_release(win->lock);
}

void
glwin_swap_buffers(glwin *win)
{
	SwapBuffers(win->hdc);
}

void
glwin_set_event_callback(glwin *win, enum win_callback_type type, void *cb)
{
	if (type < 0 || type >= glwin_event_last) return;

	gllock_acquire(win->lock);
	win->callbacks[type].ptr = cb;
	gllock_release(win->lock);
}

int
glwin_was_resized(glwin *win)
{
	int was_resized;

	gllock_acquire(win->lock);
	was_resized = win->was_resized;
	gllock_release(win->lock);

	return win->was_resized;
}

void
glwin_get_size(glwin *win, int *const width, int *const height)
{
	gllock_acquire(win->lock);
	*width = win->width;
	*height = win->height;
	gllock_release(win->lock);
}

void
glwin_get_full_size(glwin *win, int *width, int *height)
{
	RECT rc;

	gllock_acquire(win->lock);
	GetWindowRect(win->hwnd, &rc);
	gllock_release(win->lock);

	*width = rc.right - rc.left;
	*height = rc.bottom - rc.top;
}

void
glwin_set_size(glwin *win, int width, int height)
{
	gllock_acquire(win->lock);
	SetWindowPos(win->hwnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOOWNERZORDER);
	gllock_release(win->lock);
}

void
glwin_set_full_size(glwin *win, int width, int height)
{
	glwin_set_size(win, width, height);
}

void
glwin_get_pos(glwin *win, int *x, int *y)
{
	gllock_acquire(win->lock);
	*x = win->x;
	*y = win->y;
	gllock_release(win->lock);
}

void
glwin_set_pos(glwin *win, int x, int y)
{
	gllock_acquire(win->lock);
	SetWindowPos(win->hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER);
	gllock_release(win->lock);
}

void
glwin_get_mouse_pos(glwin *win, int *x, int *y)
{
	gllock_acquire(win->lock);
	*x = win->mx;
	*y = win->my;
	gllock_release(win->lock);
}

float
glwin_get_time(glwin *win)
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return (float)(li.QuadPart - win->start.QuadPart) / win->freq.QuadPart;
}

int
glwin_get_joystick_state(glwin *win, enum glwin_joystick_id id, glwinjoystickstate *state)
{

}

enum glwin_key_state
glwin_get_key(glwin *win, int key)
{
	enum glwin_key_state state;
	if (key < 0 || key >= (sizeof(win->aKeyStates) / sizeof(win->aKeyStates[0])))
		return -1;

	gllock_acquire(win->lock);
	state = win->aKeyStates[key];
	gllock_release(win->lock);

	return state;
}

enum glwin_key_state
glwin_get_mouse_button(glwin *win, int key)
{
	enum glwin_key_state state;
	if (key < 0 || key >= (sizeof(win->aKeyStates) / sizeof(win->aKeyStates[0])))
		return -1;

	gllock_acquire(win->lock);
	state = win->aMouseButtonStates[key];
	gllock_release(win->lock);

	return state;
}

int
glwin_has_focus(glwin *win)
{
	int result;

	gllock_acquire(win->lock);
	result = win->focused;
	gllock_release(win->lock);

	return result;
}

void
glwin_destroy(glwin *win)
{
	if (!win) return;

	gllock_acquire(mtgl_get_lock());

	if (win->ppd) HeapFree(win->hHeap, 0, win->ppd);

	ReleaseDC(win->hwnd, win->hdc);
	DestroyWindow(win->hwnd);
	
	gllock_destroy(win->lock);

	free(win);

	win_class_refs--;
	if (win_class_refs == 0) UnregisterClassA(win_class_name, GetModuleHandleA(NULL));

	gllock_release(mtgl_get_lock());
}