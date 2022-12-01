#pragma once

#include "mtgl_input_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct glwin glwin;
	typedef struct glctx glctx;
	typedef struct glthread glthread;
	typedef struct gllock gllock;

	typedef int(*glthread_fn)(void *);

	typedef void(*glwin_resize_cb_fn)(glwin *win, int width, int height);
	typedef void(*glwin_mouse_move_cb_fn)(glwin *win, int old_x, int old_y, int x, int y);
	typedef void(*glwin_key_cb_fn)(glwin *win, int key, enum glwin_key_state action, int mods);
	typedef void(*glwin_char_cb_fn)(glwin *win, unsigned int code, int repeat_count, int mods);
	typedef void(*glwin_mouse_button_cb_fn)(glwin *win, int button, enum glwin_key_state action, int mods);
	typedef void(*glwin_mouse_event_cb_fn)(glwin *win, int entered);

	enum win_event_type
	{
		glwin_event_resize,
		glwin_event_mouse_move,
		glwin_event_key,
		glwin_event_char,
		glwin_event_mouse_button,

		glwin_event_last = 16
	};

	int mtgl_init();
	gllock *mtgl_get_lock();
	void mtgl_done();

	glwin *glwin_create(int width, int height, void *user_data);
	void *glwin_get_user_data(glwin *win);
	void glwin_show_window(glwin *win, int shown);
	int glwin_should_close(glwin *win);
	void glwin_set_should_close(glwin *win, int should_close);
	void glwin_poll_events(glwin *win);
	void glwin_swap_buffers(glwin *win);
	void glwin_set_event_callback(glwin *win, enum win_event_type type, void *cb);
	int glwin_was_resized(glwin *win);
	void glwin_get_size(glwin *win, int *width, int *height);
	void glwin_set_size(glwin *win, int width, int height);
	void glwin_get_mouse_pos(glwin *win, int *x, int *y);
	enum glwin_key_state glwin_get_key(glwin *win, int key);
	enum glwin_key_state glwin_get_mouse_button(glwin *win, int key);
	float glwin_get_time(glwin *win);
	void glwin_destroy(glwin *win);

	glctx *glctx_create(glwin *win, int ver_major, int ver_minor);
	glctx *glctx_clone(glctx *ctx);
	void glctx_acquire(glctx *ctx);
	int glctx_try_acquire(glctx *ctx);
	void glctx_release(glctx *ctx);
	void glctx_set_swap_interval(glctx *ctx, int interval);
	void glctx_destroy(glctx *ctx);
	void *glctx_get_proc(const char *name);

	glthread *glthread_create(glctx *ctx, glthread_fn entry, void *param1);
	void glthread_detach(glthread *thread);
	int glthread_join(glthread *thread);

	gllock *gllock_create();
	void gllock_acquire(gllock *lock);
	int gllock_try_acquire(gllock *lock);
	void gllock_release(gllock *lock);
	void gllock_destroy(gllock *lock);

#ifdef __cplusplus
}
#endif