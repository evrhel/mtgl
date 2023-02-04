#pragma once

#include "mtgl_input_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct glwin glwin;
	typedef struct glctx glctx;
	typedef struct glthread glthread;
	typedef struct gllock gllock;
	typedef struct mtgldevice mtgldevice;
	typedef struct glctxinitargs glctxinitargs;

	typedef int(*glthread_fn)(void *);

	typedef void(*glwin_resize_cb_fn)(glwin *win, int width, int height);
	typedef void(*glwin_mouse_move_cb_fn)(glwin *win, int old_x, int old_y, int x, int y);
	typedef void(*glwin_key_cb_fn)(glwin *win, int key, enum glwin_key_state action, int mods);
	typedef void(*glwin_char_cb_fn)(glwin *win, unsigned int code, int repeat_count, int mods);
	typedef void(*glwin_mouse_button_cb_fn)(glwin *win, int button, enum glwin_key_state action, int mods);
	typedef void(*glwin_mouse_event_cb_fn)(glwin *win, int entered);
	typedef void(*glwin_window_event_cb_fn)(glwin *win, enum glwin_window_event event, int param1, int param2);
	typedef void(*glwin_device_event_cb_fn)(glwin *win, enum mtgl_device_type type, enum mtgl_device_state state, int id);
	typedef void(*glwin_user_event_cb_fn)(glwin *win, void *data);

	struct mtgldevice
	{
		enum mtgl_device_type type;
		int id;
		int is_primary;
		char name[128];
		char string[128];
	};

	struct glctxinitargs
	{
		int red_bits;
		int green_bits;
		int blue_bits;
		int alpha_bits;

		int depth_bits;
		int stencil_bits;

		int double_buffer;
		int sample_count;
	};

	int mtgl_init();
	gllock *mtgl_get_lock();
	void *mtgl_enumerate_devices(void *it, mtgldevice *device, int filter);
	void mtgl_enumerate_devices_done(void *it);
	void mtgl_done();

	glwin *glwin_create(const char *title, int width, int height, int flags, int device, void *user_data);
	void glwin_set_title(glwin *win, const char *title);
	void *glwin_get_user_data(glwin *win);
	void glwin_show_window(glwin *win, int shown);
	int glwin_should_close(glwin *win);
	void glwin_set_should_close(glwin *win, int should_close);
	void glwin_queue_event(glwin *win, enum win_event_type type, void *data);
	void glwin_poll_events(glwin *win);
	void glwin_swap_buffers(glwin *win);
	void glwin_set_event_callback(glwin *win, enum win_event_type type, void *cb);
	int glwin_was_resized(glwin *win);
	void glwin_get_size(glwin *win, int *width, int *height);
	void glwin_get_full_size(glwin *win, int *width, int *height);
	void glwin_set_size(glwin *win, int width, int height);
	void glwin_set_full_size(glwin *win, int width, int height);
	void glwin_get_pos(glwin *win, int *x, int *y);
	void glwin_set_pos(glwin *win, int x, int y);
	void glwin_get_mouse_pos(glwin *win, int *x, int *y);
	int glwin_get_key(glwin *win, int key);
	int glwin_get_mouse_button(glwin *win, int key);
	int glwin_has_focus(glwin *win);
	float glwin_get_time(glwin *win);
	int glwin_get_joystick_count(glwin *win);
	int glwin_get_joystick_info(glwin *win, enum glwin_joystick_id id, glwinjoystickinfo *info);
	int glwin_get_joystick_raw_state(glwin *win, enum glwin_joystick_id id, glwinrawjoystickstate *state);
	int glwin_get_joystick_state(glwin *win, enum glwin_joystick_id id, glwinjoystickstate *state);
	void glwin_destroy(glwin *win);

	void glctx_default_init_args(glctxinitargs *args);
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