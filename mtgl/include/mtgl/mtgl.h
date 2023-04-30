#pragma once

#include "mtgl_input_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct mtglwin mtglwin;
	typedef struct mtglctx mtglctx;
	typedef struct mtgllock mtgllock;
	typedef struct mtgldevice mtgldevice;
	typedef struct mtglctxinitargs mtglctxinitargs;

	typedef int(*glthread_fn)(void *);

	typedef void(*mtgl_resize_cb_fn)(mtglwin *win, int width, int height);
	typedef void(*mtgl_mouse_move_cb_fn)(mtglwin *win, int old_x, int old_y, int x, int y);
	typedef void(*mtgl_key_cb_fn)(mtglwin *win, int key, int action, int mods);
	typedef void(*mtgl_char_cb_fn)(mtglwin *win, unsigned int code, int repeat_count, int mods);
	typedef void(*mtgl_mouse_button_cb_fn)(mtglwin *win, int button, int action, int mods);
	typedef void(*mtgl_mouse_event_cb_fn)(mtglwin *win, int entered);
	typedef void(*mtgl_window_event_cb_fn)(mtglwin *win, int event, int param1, int param2);
	typedef void(*mtgl_device_event_cb_fn)(mtglwin *win, int type, int state, int id);
	typedef void(*mtgl_user_event_cb_fn)(mtglwin *win, void *data);

	enum mtglcolorformat
	{
		mtgl_color_format_any = 0,

		mtgl_color_format_rgba
	};

	enum mtglprofile
	{
		mtgl_profile_core,
		mtgl_profile_compatability
	};

	struct mtgldevice
	{
		enum mtgl_device_type type;
		int id;
		int is_primary;
		char name[128];
		char string[128];
	};

	struct mtglctxinitargs
	{
		int profile;

		int double_buffer;

		int color_format;

		int red_bits;
		int green_bits;
		int blue_bits;
		int alpha_bits;

		int depth_bits;
		int stencil_bits;

		int accum_red_bits;
		int accum_green_bits;
		int accum_blue_bits;
		int accum_alpha_bits;
	
		int allow_sampling;
		int sample_count;
	};

	int mtgl_init();
	mtgllock *mtgl_get_lock();
	void *mtgl_enumerate_devices(void *it, mtgldevice *device, int filter);
	void mtgl_enumerate_devices_done(void *it);
	void mtgl_done();

	mtglwin *mtgl_win_create(const char *title, int width, int height, int flags, int device, void *user_data);
	void mtgl_set_title(mtglwin *win, const char *title);
	void *mtgl_get_user_data(mtglwin *win);
	void mtgl_show_window(mtglwin *win, int shown);
	int mtgl_should_close(mtglwin *win);
	void mtgl_set_should_close(mtglwin *win, int should_close);
	void mtgl_queue_event(mtglwin *win, int type, void *data);
	void mtgl_poll_events(mtglwin *win);
	void mtgl_swap_buffers(mtglwin *win);
	void mtgl_set_event_callback(mtglwin *win, int type, void *cb);
	int mtgl_was_resized(mtglwin *win);
	void mtgl_get_size(mtglwin *win, int *width, int *height);
	void mtgl_get_full_size(mtglwin *win, int *width, int *height);
	void mtgl_set_size(mtglwin *win, int width, int height);
	void mtgl_set_full_size(mtglwin *win, int width, int height);
	void mtgl_get_pos(mtglwin *win, int *x, int *y);
	void mtgl_set_pos(mtglwin *win, int x, int y);
	void mtgl_get_mouse_pos(mtglwin *win, int *x, int *y);
	int mtgl_get_key(mtglwin *win, int key);
	int mtgl_get_mouse_button(mtglwin *win, int key);
	int mtgl_has_focus(mtglwin *win);
	float mtgl_get_time(mtglwin *win);
	int mtgl_get_joystick_count(mtglwin *win);
	int mtgl_get_joystick_info(mtglwin *win, int id, mtgljoystickinfo *info);
	int mtgl_get_joystick_raw_state(mtglwin *win, int, int *state);
	int mtgl_get_joystick_state(mtglwin *win, int id, int *state);
	void mtgl_win_destroy(mtglwin *win);

	void mtgl_ctx_get_default_init_args(mtglctxinitargs *args);
	mtglctx *mtgl_ctx_create(mtglwin *win, int ver_major, int ver_minor, mtglctxinitargs *argsp);
	mtglctx *mtgl_ctx_clone(mtglctx *ctx);
	void mtgl_ctx_acquire(mtglctx *ctx);
	int mtgl_ctx_try_acquire(mtglctx *ctx);
	void mtgl_ctx_release(mtglctx *ctx);
	void mtgl_ctx_set_swap_interval(mtglctx *ctx, int interval);
	void mtgl_ctx_destroy(mtglctx *ctx);
	void *mtgl_ctx_get_proc(const char *name);

	mtgllock *mtgl_lock_create();
	void mtgl_lock_acquire(mtgllock *lock);
	int mtgl_lock_try_acquire(mtgllock *lock);
	void mtgl_lock_release(mtgllock *lock);
	void mtgl_lock_destroy(mtgllock *lock);

#ifdef __cplusplus
}
#endif