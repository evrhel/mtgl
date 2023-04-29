#pragma once

#include <mtgl/mtgl.h>

#include "mtgl_input_private.h"

#define JOY_NATIVE_SIZE 96
#define JOY_COUNT mtgl_joystick_last

#define EVT_CB_COUNT mtgl_event_last

#define EVENT_QUEUE_SIZE 32

#ifdef __cplusplus
extern "C" {
#endif

	typedef unsigned char state_t;

	/* data for specific event */
	union event_data
	{
		struct
		{
			char _unused[1];
		} resize;

		struct
		{
			int old_mx, old_my;
		} mouse_move;

		struct
		{
			int key;
			int action;
			int mods;
		} key;

		struct
		{
			unsigned int code;
			int repeat_count;
			int mods;
		} char_;
	
		struct
		{
			int button;
			int action;
			int mods;
		} mouse_button;

		struct
		{
			int event;
			int param1;
			int param2;
		} window_event;

		struct
		{
			int type;
			int state;
			int id;
		} device_event;

		struct
		{
			void *data;
		} user_event;
	};

	/* window event */
	struct event
	{
		int type;				// event type - describes data
		mtglwin *win;			// the window
		union event_data data;	// event data
	};

	/* window callback */
	union callback
	{
		void *ptr;
		mtgl_resize_cb_fn on_resize;
		mtgl_mouse_move_cb_fn on_mouse_move;
		mtgl_key_cb_fn on_key;
		mtgl_char_cb_fn on_char;
		mtgl_mouse_button_cb_fn on_mouse_button;
		mtgl_window_event_cb_fn on_window_event;
		mtgl_device_event_cb_fn on_device_event;
		mtgl_user_event_cb_fn on_user_event;
	};

	struct joystick
	{
		int connected;		// whether the joystick is connected

		char name[64];		// name of joystick
		char vendor[64];	// vendor of joystick

		char native[JOY_NATIVE_SIZE];	// native joystick handle
	};

	struct mtglwin
	{
		mtglctx *main;		// main OpenGL context
		mtgllock *lock;		// lock for this window
		int flags;			// flags passed to the creation function
		int should_close;	// whether the window should close
		int was_resized;	// whether the window was resized
		int width, height;	// dimensions of user area
		int x, y;			// position of window
		int mx, my;			// mouse position
		int dmx, dmy;		// delta mouse position
		int focused;		// whether the window is focused
		int mods;			// mofifier keys held on an input event

		int mb_states[8];			// state of each mouse button
		int key_states[0x100];		// state of each key
		int keyMods;				// modifier keys held down
		struct joystick *joysticks;	// joysticks

		short wheel;				// state of mouse wheel

		union callback *callbacks;	// user event callbacks length = mtgl_event_last

		struct event *events;		// event queue
		int event_last;				// end of event queue

		void *user_data;			// user data passed to creation function
	};

	/* event management */

	int mtgl_push_event(mtglwin *, struct event *);
	int mtgl_push_button_event(mtglwin *win, struct event *event, int action, int button);
	void mtgl_dispatch_events(mtglwin *);

#ifdef __cplusplus
}
#endif