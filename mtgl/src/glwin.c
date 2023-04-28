#include "glwin_private.h"
#include "mtgl_input_private.h"

#include <assert.h>

static const char win_class_name[] = "glwin";
static int win_class_refs = 0;

static int
warn(mtglwin *win, const char *msg)
{
	return 1;
}

static int
error(mtglwin *win, const char *msg)
{
	return 2;
}

int
push_event(mtglwin *win, struct event *evt)
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
mtgl_win_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	mtglwin *win;
	struct event event;
	LPCREATESTRUCTA lpCreateStruct;

	win = (mtglwin *)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

	switch (uMsg)
	{
	case WM_NCCREATE: {
		//SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG)((LPCREATESTRUCTA)lParam)->lpCreateParams);
		return TRUE;
	}
	case WM_CREATE: {
		lpCreateStruct = (LPCREATESTRUCTA)lParam;
		win = lpCreateStruct->lpCreateParams;
		SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)win);
		win->x = lpCreateStruct->x;
		win->y = lpCreateStruct->y;
		win->width = lpCreateStruct->cx;
		win->height = lpCreateStruct->cy;
		return 0;
	}
	case WM_CLOSE:
	case WM_QUIT: {
		win->should_close = 1;

		event.type = mtgl_event_window_event;
		event.data.window_event.event = mtgl_window_closing;
		event.data.window_event.param1 = 0;
		event.data.window_event.param2 = 0;
		push_event(win, &event);
		return 0;
	}
	case WM_MOVE: {
		
		event.type = mtgl_event_window_event;
		event.data.window_event.event = mtgl_window_move;
		event.data.window_event.param1 = LOWORD(lParam);
		event.data.window_event.param2 = HIWORD(lParam);
		push_event(win, &event);

		return 0;
	}
	case WM_SIZE: {
		win->was_resized = 1;

		// resize event
		event.type = mtgl_event_resize;
		push_event(win, &event);

		// window event
		event.type = mtgl_event_window_event;
		switch (wParam)
		{
		case SIZE_MAXIMIZED:
			event.data.window_event.event = mtgl_window_maximize;
			break;
		case SIZE_MINIMIZED:
			event.data.window_event.event = mtgl_window_minimize;
			break;
		case SIZE_RESTORED:
			event.data.window_event.event = mtgl_window_restore;
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

		event.type = mtgl_event_window_event;
		event.data.window_event.event = mtgl_window_changefocus;
		event.data.window_event.param1 = 1;
		event.data.window_event.param2 = 0;
		push_event(win, &event);
		return 0;
	}
	case WM_KILLFOCUS: {
		win->focused = 0;

		event.type = mtgl_event_window_event;
		event.data.window_event.event = mtgl_window_changefocus;
		event.data.window_event.param1 = 0;
		event.data.window_event.param2 = 0;
		push_event(win, &event);
		return 0;
	}
	case WM_CHAR:
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_SYSCHAR:
	case WM_INPUT:
	case WM_INPUT_DEVICE_CHANGE: return glwin_handle_input_message(win, hwnd, uMsg, wParam, lParam);
		break;
	}

	return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

static void
dispatch_events(mtglwin *win)
{
	struct event *event;
	union event_data *data;
	union callback callback;

	/* iterate through event queue and run callbacks */
	for (event = win->events; event < win->events + win->event_last; event++)
	{
		callback = win->callbacks[event->type];
		if (!callback.ptr)
			continue;

		data = &event->data;
		switch (event->type)
		{
		case mtgl_event_resize:
			callback.on_resize(win, win->width, win->height);
			break;
		case mtgl_event_mouse_move: {
			callback.on_mouse_move(
				win,
				data->mouse_move.old_mx, data->mouse_move.old_my,
				win->mx, win->my
			);
			break;
		}
		case mtgl_event_key: {
			callback.on_key(
				win,
				data->key.key,
				data->key.action,
				data->key.mods
			);
			break;
		}
		case mtgl_event_char: {
			callback.on_char(
				win, 
				data->char_.code,
				data->char_.repeat_count,
				data->char_.mods
			);
			break;
		}
		case mtgl_event_mouse_button: {
			callback.on_mouse_button(
				win,
				data->mouse_button.button,
				data->mouse_button.action,
				data->mouse_button.mods
			);
			break;
		}
		case mtgl_event_window_event: {
			callback.on_window_event(
				win,
				data->window_event.event,
				data->window_event.param1,
				data->window_event.param2
			);
			break;
		}
		case mtgl_event_user1:
		case mtgl_event_user2:
		case mtgl_event_user3:
		case mtgl_event_user4:
			callback.on_user_event(win, data->user_event.data);
			break;
		}
	}

	win->event_last = 0;
}

mtglwin *
mtgl_win_create(const char *title, int width, int height, int flags, int device, void *user_data)
{
	HINSTANCE hInstance;
	mtglwin *win = 0;
	WNDCLASSA wc;
	ATOM atom = 0;
	DWORD dwStyle;
	int i;
	
	hInstance = GetModuleHandleA(NULL); // current executable

	mtgl_lock_acquire(mtgl_get_lock());

	/* register window class if necessary */
	if (win_class_refs == 0)
	{
		ZeroMemory(&wc, sizeof(wc));
		wc.lpfnWndProc = &mtgl_win_proc;
		wc.hInstance = hInstance;
		wc.lpszClassName = win_class_name;
		wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		wc.hIcon = NULL;

		atom = RegisterClassA(&wc);
		if (!atom) 0;
	}

	win = calloc(1, sizeof(mtglwin));
	if (!win) goto failure;

	win->lock = mtgl_lock_create();
	if (!win->lock) goto failure;

	win->flags = flags;

	/* create the window */
	dwStyle = WS_OVERLAPPEDWINDOW;
	win->hwnd = CreateWindowExA(
		0,								// no extra style
		win_class_name,					// mtgl window class
		title,							// user title
		dwStyle,						// window style
		CW_USEDEFAULT, CW_USEDEFAULT,	// default position
		width, height,					// user width and height
		NULL,							// no parent
		NULL,							// no menu
		hInstance,						// current application module
		win								// the mtgl window
	);
	if (!win->hwnd) goto failure;

	/* get the device context */
	win->hdc = GetDC(win->hwnd);
	if (!win->hdc) goto failure;

	/* setup timer */
	if (!QueryPerformanceFrequency(&win->freq)) goto failure;
	QueryPerformanceCounter(&win->start);

	win->user_data = user_data;

	/* initialize joysticks */
	for (i = 0; i < mtgl_joystick_last; i++)
		mtgl_init_joystick(&win->aJoysticks[i]);

	/* register input devices */

	if (flags & mtgl_wf_raw_mouse_input)
	{
		win->rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		win->rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
		win->rid[0].dwFlags = RIDEV_NOLEGACY;
		win->rid[0].hwndTarget = win->hwnd;

		if (!RegisterRawInputDevices(&win->rid[0], 1, sizeof(win->rid[0])))
			goto failure;
	}

	if (flags & mtgl_wf_raw_keyboard_input)
	{
		win->rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
		win->rid[1].usUsage = HID_USAGE_GENERIC_KEYBOARD;
		win->rid[1].dwFlags = RIDEV_NOLEGACY;
		win->rid[1].hwndTarget = win->hwnd;

		if (!RegisterRawInputDevices(&win->rid[1], 1, sizeof(win->rid[1])))
			goto failure;
	}

	win_class_refs++;
	mtgl_lock_release(mtgl_get_lock());
	return win;

failure:

	if (win)
	{
		if (win->hdc) ReleaseDC(win->hwnd, win->hdc);
		if (win->hwnd) DestroyWindow(win->hwnd);
		if (win->lock) mtgl_lock_destroy(win->lock);

		free(win);
	}

	if (atom) UnregisterClassA(win_class_name, hInstance);

	mtgl_lock_release(mtgl_get_lock());
	return 0;
}

void
mtgl_set_title(mtglwin *win, const char *title)
{
	mtgl_lock_acquire(win->lock);
	SetWindowTextA(win->hwnd, title);
	mtgl_lock_release(win->lock);
}

void *
mtgl_get_user_data(mtglwin *win)
{
	return win->user_data;
}

void
mtgl_show_window(mtglwin *win, int shown)
{
	RECT rc;

	mtgl_lock_acquire(win->lock);
	ShowWindow(win->hwnd, shown ? SW_SHOW : SW_HIDE);

	GetWindowRect(win->hwnd, &rc);
	win->x = rc.left;
	win->y = rc.top;

	mtgl_lock_release(win->lock);
}

int
mtgl_should_close(mtglwin *win)
{
	int should_close;

	mtgl_lock_acquire(win->lock);
	should_close = win->should_close;
	mtgl_lock_release(win->lock);

	return should_close;
}

void
mtgl_set_should_close(mtglwin *win, int should_close)
{
	mtgl_lock_acquire(win->lock);
	win->should_close = should_close;
	mtgl_lock_release(win->lock);
}

void
mtgl_queue_event(mtglwin *win, int type, void *data)
{
	struct event evt;

	evt.type = type;
	evt.data.user_event.data = data;

	mtgl_lock_acquire(win->lock);
	push_event(win, &evt);
	mtgl_lock_release(win->lock);
}

void
mtgl_poll_events(mtglwin *win)
{
	BOOL bResult;
	MSG msg;

	RECT rc;

	mtgl_lock_acquire(win->lock);

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

	mtgl_lock_release(win->lock);
}

void
mtgl_swap_buffers(mtglwin *win)
{
	SwapBuffers(win->hdc);
}

void
mtgl_set_event_callback(mtglwin *win, int type, void *cb)
{
	if (type < 0 || type >= mtgl_event_last) return;

	mtgl_lock_acquire(win->lock);
	win->callbacks[type].ptr = cb;
	mtgl_lock_release(win->lock);
}

int
mtgl_was_resized(mtglwin *win)
{
	int was_resized;

	mtgl_lock_acquire(win->lock);
	was_resized = win->was_resized;
	mtgl_lock_release(win->lock);

	return win->was_resized;
}

void
mtgl_get_size(mtglwin *win, int *const width, int *const height)
{
	mtgl_lock_acquire(win->lock);
	*width = win->width;
	*height = win->height;
	mtgl_lock_release(win->lock);
}

void
mtgl_get_full_size(mtglwin *win, int *width, int *height)
{
	RECT rc;

	mtgl_lock_acquire(win->lock);
	GetWindowRect(win->hwnd, &rc);
	mtgl_lock_release(win->lock);

	*width = rc.right - rc.left;
	*height = rc.bottom - rc.top;
}

void
mtgl_set_size(mtglwin *win, int width, int height)
{
	mtgl_lock_acquire(win->lock);
	SetWindowPos(win->hwnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOOWNERZORDER);
	mtgl_lock_release(win->lock);
}

void
mtgl_set_full_size(mtglwin *win, int width, int height)
{
	mtgl_set_size(win, width, height);
}

void
mtgl_get_pos(mtglwin *win, int *x, int *y)
{
	mtgl_lock_acquire(win->lock);
	*x = win->x;
	*y = win->y;
	mtgl_lock_release(win->lock);
}

void
mtgl_set_pos(mtglwin *win, int x, int y)
{
	mtgl_lock_acquire(win->lock);
	SetWindowPos(win->hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER);
	mtgl_lock_release(win->lock);
}

void
mtgl_get_mouse_pos(mtglwin *win, int *x, int *y)
{
	mtgl_lock_acquire(win->lock);
	*x = win->mx;
	*y = win->my;
	mtgl_lock_release(win->lock);
}

float
mtgl_get_time(mtglwin *win)
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return (float)(li.QuadPart - win->start.QuadPart) / win->freq.QuadPart;
}

int
mtgl_get_joystick_count(mtglwin *win)
{
	int i;
	int count = 0;

	for (i = 0; i < mtgl_joystick_last; i++)
		if (win->aJoysticks[i].connected)
			count++;

	return count;
}

int
mtgl_get_joystick_info(mtglwin *win, int id, int *info)
{
	struct joystick *joystick;

	if (id < 0 || id >= mtgl_joystick_last) return mtgl_device_bad_id;

	joystick = &win->aJoysticks[id];
	if (!joystick->connected) return mtgl_device_disconnected;



	return mtgl_device_connected;
}

int
mtgl_get_joystick_raw_state(mtglwin *win, int id, int *state)
{
	struct joystick *joystick;

	if (id < 0 || id >= mtgl_joystick_last) return mtgl_device_bad_id;

	joystick = &win->aJoysticks[id];
	if (!joystick->connected) return mtgl_device_disconnected;

	return mtgl_device_connected;
}

int
mtgl_get_joystick_state(mtglwin *win, int id, int *state)
{
	int result;
	int rawstate;

	result = mtgl_get_joystick_raw_state(win, id, &rawstate);
	if (result != mtgl_device_connected) return result;

	return mtgl_device_connected;
}

int
mtgl_get_key(mtglwin *win, int key)
{
	enum glwin_key_state state;
	if (key < 0 || key >= (sizeof(win->aKeyStates) / sizeof(win->aKeyStates[0])))
		return -1;

	mtgl_lock_acquire(win->lock);
	state = win->aKeyStates[key];
	mtgl_lock_release(win->lock);

	return state;
}

int
mtgl_get_mouse_button(mtglwin *win, int key)
{
	int state;
	if (key < 0 || key >= (sizeof(win->aKeyStates) / sizeof(win->aKeyStates[0])))
		return -1;

	mtgl_lock_acquire(win->lock);
	state = win->aMouseButtonStates[key];
	mtgl_lock_release(win->lock);

	return state;
}

int
mtgl_has_focus(mtglwin *win)
{
	int result;

	mtgl_lock_acquire(win->lock);
	result = win->focused;
	mtgl_lock_release(win->lock);

	return result;
}

void
mtgl_win_destroy(mtglwin *win)
{
	if (!win) return;

	mtgl_lock_acquire(mtgl_get_lock());

	if (win->ppd) HeapFree(win->hHeap, 0, win->ppd);

	ReleaseDC(win->hwnd, win->hdc);
	DestroyWindow(win->hwnd);
	
	mtgl_lock_destroy(win->lock);

	free(win);

	win_class_refs--;
	if (win_class_refs == 0) UnregisterClassA(win_class_name, GetModuleHandleA(NULL));

	mtgl_lock_release(mtgl_get_lock());
}