#include "mtgl_win.h"

#if _WIN32
#include "mtgl_win_win32.h"
#endif

#include <assert.h>

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
mtgl_push_event(mtglwin *win, struct event *evt)
{
	if (win->event_last == EVENT_QUEUE_SIZE)
		return warn(win, "event queue full");

	evt->win = win;
	win->events[win->event_last] = *evt;
	win->event_last++;

	return 0;
}

int
mtgl_push_button_event(mtglwin *win, struct event *event, int action, int button)
{
	win->mb_states[mtgl_mouse1] = mtgl_pressed;

	event->type = mtgl_event_mouse_button;
	event->data.mouse_button.action = mtgl_pressed;
	event->data.mouse_button.button = mtgl_mouse1;
	event->data.mouse_button.mods = win->mods;
	return mtgl_push_event(win, event);
}

void
mtgl_dispatch_events(mtglwin *win)
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
#if _WIN32
	return &mtgl_win_create_win32(title, width, height, flags, device, user_data)->win;
#else
	return 0;
#endif
}

void
mtgl_set_title(mtglwin *win, const char *title)
{
#if _WIN32
	mtgl_set_title_win32((struct mtglwin_win32 *)win, title);
#endif
}

void *
mtgl_get_user_data(mtglwin *win)
{
	return win->user_data;
}

void
mtgl_show_window(mtglwin *win, int shown)
{
#if _WIN32
	mtgl_show_window_win32((struct mtglwin_win32 *)win, shown);
#endif
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
	mtgl_push_event(win, &evt);
	mtgl_lock_release(win->lock);
}

void
mtgl_poll_events(mtglwin *win)
{
#if _WIN32
	mtgl_poll_events_win32((struct mtglwin_win32 *)win);
#endif
}

void mtgl_swap_buffers(mtglwin *win)
{
#if _WIN32
	mtgl_swap_buffers_win32((struct mtglwin_win32 *)win);
#endif
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
#if _WIN32
	mtgl_get_full_size_win32((struct mtglwin_win32 *)win, width, height);
#endif
}

void
mtgl_set_size(mtglwin *win, int width, int height)
{
#if _WIN32
	mtgl_set_size_win32((struct mtglwin_win32 *)win, width, height);
#endif
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
#if _WIN32
	mtgl_set_pos_win32((struct mtglwin_win32 *)win, x, y);
#endif
}

void
mtgl_get_mouse_pos(mtglwin *win, int *x, int *y)
{
	mtgl_lock_acquire(win->lock);
	*x = win->mx;
	*y = win->my;
	mtgl_lock_release(win->lock);
}

int
mtgl_get_key(mtglwin *win, int key)
{
	int state;
	if (key < 0 || key >= (sizeof(win->key_states) / sizeof(win->key_states[0])))
		return -1;

	mtgl_lock_acquire(win->lock);
	state = win->key_states[key];
	mtgl_lock_release(win->lock);

	return state;
}

int
mtgl_get_mouse_button(mtglwin *win, int key)
{
	int state;
	if (key < 0 || key >= (sizeof(win->mb_states) / sizeof(win->mb_states[0])))
		return -1;

	mtgl_lock_acquire(win->lock);
	state = win->mb_states[key];
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

float
mtgl_get_time(mtglwin *win)
{
#if _WIN32
	return mtgl_get_time_win32((struct mtglwin_win32 *)win);
#else
	return 0.0f;
#endif
}

int
mtgl_get_joystick_count(mtglwin *win)
{
	int i;
	int count = 0;

	for (i = 0; i < JOY_COUNT; i++)
		if (win->joysticks[i].connected)
			count++;

	return count;
}

int
mtgl_get_joystick_raw_state(mtglwin *win, int id, int *state)
{
	struct joystick *joystick;

	if (id < 0 || id >= JOY_COUNT) return mtgl_device_bad_id;

	joystick = &win->joysticks[id];
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
mtgl_get_joystick_info(mtglwin *win, int id, mtgljoystickinfo *info)
{
	struct joystick *joystick;

	if (id < 0 || id >= JOY_COUNT) return mtgl_device_bad_id;

	joystick = &win->joysticks[id];
	if (!joystick->connected) return mtgl_device_disconnected;

	return mtgl_device_connected;
}
void
mtgl_win_destroy(mtglwin *win)
{
	if (!win) return;
#if _WIN32
	mtgl_win_destroy_win32((struct mtglwin_win32 *)win);
#endif
}