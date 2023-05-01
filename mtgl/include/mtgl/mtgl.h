#pragma once

#include "mtgl_input_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

	// Window handle, defined in mtgl_win.h
	typedef struct mtglwin mtglwin;

	// OpenGL context, defined in mtgl_ctx.h
	typedef struct mtglctx mtglctx;

	// Lock, defined in mtgl_lock.h
	typedef struct mtgllock mtgllock;

	// Condition variable, defined in mtgl_lock.h
	typedef struct mtglcondition mtglcondition;

	// Device, defined in mtgl.h
	typedef struct mtgldevice mtgldevice;

	// OpenGL context initialization arguments, defined in mtgl.h
	typedef struct mtglctxinitargs mtglctxinitargs;

	//////////////////////////////////////////////////////////////////////////
	// Callbacks
	//////////////////////////////////////////////////////////////////////////
	
	// Called when the window is resized
	//
	// Parameters:
	// - win: The window that was resized
	// - width: The new width of the window
	// - height: The new height of the window
	typedef void(*mtgl_resize_cb_fn)(mtglwin *win, int width, int height);

	// Called when the window is moved
	//
	// Parameters:
	// - win: The window that was moved
	// - old_x: The old x position of the window
	// - old_y: The old y position of the window
	// - x: The new x position of the window
	// - y: The new y position of the window
	typedef void(*mtgl_mouse_move_cb_fn)(mtglwin *win, int old_x, int old_y, int x, int y);

	// Called when a key is pressed or released
	//
	// Parameters:
	// - win: The window that received the event
	// - key: The key that was pressed or released, see mtgl_key in mtgl_input_defs.h.
	// - action: The action that was performed on the key, see mtgl_key_state
	// 			 in mtgl_input_defs.h.
	// - mods: The modifier keys that were held when the button was pressed
	typedef void(*mtgl_key_cb_fn)(mtglwin *win, int key, int action, int mods);

	// Called when a character is input
	//
	// Parameters:
	// - win: The window that received the event
	// - code: The unicode code point of the character
	// - repeat_count: The number of times the character was repeated
	// - mods: The modifier keys that were held when the button was pressed
	typedef void(*mtgl_char_cb_fn)(mtglwin *win, unsigned int code, int repeat_count, int mods);

	// Called when a mouse button is pressed or released
	//
	// Parameters:
	// - win: The window that received the event
	// - button: The button that was pressed or released, see mtgl_mouse_button
	// 			 in mtgl_input_defs.h.
	// - action: The action that was performed on the button, see mtgl_key_state
	// 			 in mtgl_input_defs.h.
	// - mods: The modifier keys that were held when the button was pressed
	typedef void(*mtgl_mouse_button_cb_fn)(mtglwin *win, int button, int action, int mods);

	// Called when the mouse enters or leaves the window
	//
	// Parameters:
	// - win: The window that received the event
	// - entered: 1 if the mouse entered the window, 0 if it left
	typedef void(*mtgl_mouse_event_cb_fn)(mtglwin *win, int entered);

	// Called on a miscallaneous window event, see mtgl_window_event. Parameters
	// param1 and param2 have different meanings depending on the value of event.
	// See mtgl_window_event in mtgl_input_defs.h for more information.
	//
	// Parameters:
	// - win: The window that received the event
	// - event: The event that occurred, see mtgl_window_event
	// - param1: The first parameter of the event
	// - param2: The second parameter of the event
	typedef void(*mtgl_window_event_cb_fn)(mtglwin *win, int event, int param1, int param2);
	
	// Called when a device event occurs
	//
	// Parameters:
	// - win: The window that received the event
	// - type: The type of device, see mtgl_device_type in mtgl_input_defs.h
	// - state: The state of the device, see mtgl_device_state in mtgl_input_defs.h
	// - id: The id of the device
	typedef void(*mtgl_device_event_cb_fn)(mtglwin *win, int type, int state, int id);

	// Called when a user event occurs, see mtgl_queue_event
	//
	// Parameters:
	// - win: The window that received the event
	// - data: The data that was passed to mtgl_queue_event
	typedef void(*mtgl_user_event_cb_fn)(mtglwin *win, void *data);

	// color format
	enum mtglcolorformat
	{
		// use any color format
		mtgl_color_format_any = 0,

		// use RGBA color format
		mtgl_color_format_rgba
	};

	// OpenGL profile
	enum mtglprofile
	{
		// use any profile
		mtgl_profile_any,

		// core profile
		mtgl_profile_core,

		// compatability profile
		mtgl_profile_compatability
	};

	// mtgl OpenGL context policy
	enum mtglctxpolicy
	{
		// Default context policy, allows synchronization on the context. When
		// the OpenGL context is needed, make a call to mtgl_ctx_acquire or
		// mtgl_ctx_try_acquire. When done, call mtgl_ctx_release so others
		// may use it.
		mtgl_ctx_policy_default = 0,

		// Use a context scheduler policy. The thread on which the context
		// was created must continually call mtgl_ctx_sched. All other threads
		// use the API as normal. This is more complex to implement, but should
		// be used if the main thread is using the context for most of the time
		// as with the default context policy, there may be a lot of the main
		// thread releasing the context only to get it back immediately. Using
		// this releases the context on the main thread only when needed.
		mtgl_ctx_policy_use_scheduler,

		// The context should not be thread-safe. Use when only one thread is
		// going to use the OpenGL context. With this policy, no synchronization
		// calls are needed, and the thread that created the context will
		// always have control of the context.
		mtgl_ctx_policy_one_thread
	};

	// Return result from mtgl_ctx_sched
	enum mtglschedresult
	{
		// The OpenGL context has been acquired
		mtgl_sched_ctx_acquired,

		// The OpenGL context is in use
		mtgl_sched_ctx_in_use,

		// This thread is not allowed to call mtgl_ctx_sched
		mtgl_sched_not_allowed,

		// The function failed for any reason
		mtgl_sched_failure
	};

	// Information on how a context should be acquired
	enum mtglacquirebehavior
	{
		// Never acquire the context
		mtgl_ctx_acquire_never,

		// Always acquire the context
		mtgl_ctx_acquire_always,

		// Always acquire the context if it is avaliable
		mtgl_ctx_acquire_if_avaliable,

		// Sometimes acquire the context if it is avaliable
		mtgl_ctx_acquire_sometimes
	};

	// Information about a context acquisition request
	enum mtglacquireresult
	{
		// The context was acquired
		mtgl_ctx_acquire_success,

		// The context was not acquired
		mtgl_ctx_acquire_fail,

		// The context is in use
		mtgl_ctx_acquire_busy,

		// This thread is not allowed to acquire the context
		mtgl_ctx_acquire_not_allowed,

		// Any error occured while acquiring the context
		mtgl_ctx_acquire_error
	};

	// Device info
	struct mtgldevice
	{
		// device type, see mtgl_device_type in mtgl_input_defs.h
		int type;

		// device id
		int id;

		// whether the device is the primary device of its type
		int is_primary;

		// device name
		char name[128];
		
		// device vendor
		char string[128];
	};

	// OpenGL context initialization info
	struct mtglctxinitargs
	{
		// OpenGL profile, see mtglprofile
		int profile;

		// Whether to use double buffering
		int double_buffer;

		// Color format, see mtglcolorformat
		int color_format;

		// Number of bits for each color component
		int red_bits;
		int green_bits;
		int blue_bits;
		int alpha_bits;

		// Number of bits for the depth component
		int depth_bits;

		// Number of bits for the stencil component
		int stencil_bits;

		// Number of bits for each accumulation buffer component
		int accum_red_bits;
		int accum_green_bits;
		int accum_blue_bits;
		int accum_alpha_bits;
	
		// Allow multisampling
		int allow_sampling;

		// Number of samples for multisampling
		int sample_count;
	};

	///////////////////////////////////////////////////////////////////////////
	// API
	///////////////////////////////////////////////////////////////////////////

	// Initialize mtgl
	//
	int mtgl_init();

	// Get a lock for the mtgl library
	//
	mtgllock *mtgl_get_lock();

	// Enumerate system devices. Call mtgl_enumerate_devices_done when finished.
	//
	// Parameters:
	// - it: An iterator to use for enumeration. Pass NULL to start a new
	// 		 enumeration.
	// - device: A pointer to a mtgldevice struct to fill with information about
	// 			 the device.
	// - filter: A filter to select which devices to enumerate, see mtgl_device_type
	// 			 in mtgl_input_defs.h.
	//
	// Returns:
	// An iterator to use for the next call to mtgl_enumerate_devices, or NULL if
	// there are no more devices to enumerate.
	void *mtgl_enumerate_devices(void *it, mtgldevice *device, int filter);

	// Finish enumerating system devices
	//
	// Parameters:
	// - it: The iterator to finish
	void mtgl_enumerate_devices_done(void *it);

	// Cleanup mtgl
	//
	void mtgl_done();

	///////////////////////////////////////////////////////////////////////////
	// Window API
	///////////////////////////////////////////////////////////////////////////

	// Create a new window.
	//
	// Parameters:
	// - title: The title of the window.
	// - width: The width of the window.
	// - height: The height of the window.
	// - flags: Flags to use when creating the window, see mtgl_window_flags.
	// - device: Unused.
	// - user_data: User data to associate with the window. Can be retrieved with
	// 				mtgl_get_user_data.
	//
	// Returns:
	// A pointer to the new window, or NULL if the window could not be created.
	mtglwin *mtgl_win_create(const char *title, int width, int height, int flags, int device, void *user_data);

	// Set the title of a window.
	//
	// Parameters:
	// - win: The window to set the title of.
	// - title: The new title of the window.
	void mtgl_set_title(mtglwin *win, const char *title);

	// Retrieve the user data associated with a window.
	//
	// Parameters:
	// - win: The window to retrieve the user data from.
	//
	// Returns:
	// The user data associated with the window.
	void *mtgl_get_user_data(mtglwin *win);

	// Show or hide a window.
	//
	// Parameters:
	// - win: The window to show or hide.
	// - shown: 0 to hide the window, 1 to show the window.
	void mtgl_show_window(mtglwin *win, int shown);

	// Whether a window should close. This either set by a an input event
	// (such as a close button being pressed) or by calling mtgl_set_should_close
	// on the window.
	//
	// Parameters:
	// - win: The window to check.
	//
	// Returns:
	// 1 if the window should close, 0 otherwise.
	int mtgl_should_close(mtglwin *win);

	// Set whether a window should close.
	//
	// Parameters:
	// - win: The window to set.
	// - should_close: 1 if the window should close, 0 otherwise.
	void mtgl_set_should_close(mtglwin *win, int should_close);

	// Manually queue a window event. The event will be processed on the next
	// call to mtgl_poll_events. This call should only be used for events that
	// are not generated by the windowing system. Use it for events such as
	// mtgl_event_user*, which can be processed by event callbacks.
	//
	// Parameters:
	// - win: The window to queue the event for.
	// - type: The type of the event, see mtgl_event_type in mtgl_input_defs.h.
	// - data: The user data associated with the event. This varies per event
	// 		   type, but for user events, this parameter is passed to the event
	// 		   callback.
	void mtgl_queue_event(mtglwin *win, int type, void *data);

	// Poll for events. This processed any events that have been queued by
	// the windowing system or by mtgl_queue_event. Not calling this function
	// will cause the window to become unresponsive.
	//
	// Parameters:
	// - win: The window to poll events for.
	void mtgl_poll_events(mtglwin *win);

	// Swap the front and back buffers of a window. Do this after rendering
	// to the back buffer to display the rendered image on the window.
	//
	// Parameters:
	// - win: The window to swap buffers for.
	void mtgl_swap_buffers(mtglwin *win);

	// Set an event callback for a specific event. This callback will be called
	// when the event is processed by mtgl_poll_events.
	//
	// Parameters:
	// - win: The window to set the callback for.
	// - type: The type of the event, see mtgl_event_type in mtgl_input_defs.h.
	// - cb: The callback to call when the event is processed. This callback
	// 		 should be a pointer to a function with the signature of one
	// 		 the respective function type defined in this header. (For instance
	// 		 for mtgl_event_resize this should be a mtgl_resize_cb_fn). 
	void mtgl_set_event_callback(mtglwin *win, int type, void *cb);

	// Check if a window was resized since the last call to mtgl_poll_events.
	//
	// Parameters:
	// - win: The window to check.
	//
	// Returns:
	// 1 if the window was resized, 0 otherwise.
	int mtgl_was_resized(mtglwin *win);

	// Get the size of a window, not including decorations (such as the title bar).
	// This is the space in which content will be displayed.
	//
	// Parameters:
	// - win: The window to get the size of.
	// - width: A pointer to an integer to store the width of the window in.
	// - height: A pointer to an integer to store the height of the window in.
	void mtgl_get_size(mtglwin *win, int *width, int *height);

	// Get the size of a window, including decorations.
	//
	// Parameters:
	// - win: The window to get the size of.
	// - width: A pointer to an integer to store the width of the window in.
	// - height: A pointer to an integer to store the height of the window in.
	void mtgl_get_full_size(mtglwin *win, int *width, int *height);

	// Set the size of a window, not including decorations.
	//
	// Parameters:
	// - win: The window to set the size of.
	// - width: The new width of the window.
	// - height: The new height of the window.
	void mtgl_set_size(mtglwin *win, int width, int height);

	// Set the size of a window, including decorations.
	//
	// Parameters:
	// - win: The window to set the size of.
	// - width: The new width of the window.
	// - height: The new height of the window.
	void mtgl_set_full_size(mtglwin *win, int width, int height);

	// Get the position of a window.
	//
	// Parameters:
	// - win: The window to get the position of.
	// - x: A pointer to an integer to store the x position of the window in.
	// - y: A pointer to an integer to store the y position of the window in.
	void mtgl_get_pos(mtglwin *win, int *x, int *y);

	// Set the position of a window.
	//
	// Parameters:
	// - win: The window to set the position of.
	// - x: The new x position of the window.
	// - y: The new y position of the window.
	void mtgl_set_pos(mtglwin *win, int x, int y);

	// Get the position of the mouse relative to the window, in pixels.
	//
	// Parameters:
	// - win: The window to get the mouse position relative to.
	// - x: A pointer to an integer to store the x position of the mouse in.
	// - y: A pointer to an integer to store the y position of the mouse in.
	void mtgl_get_mouse_pos(mtglwin *win, int *x, int *y);

	// Get the state of a key on the keyboard for a window.
	//
	// Parameters:
	// - win: The window to get the key state for.
	// - key: The key to get the state of, see mtgl_key in mtgl_input_defs.h.
	//
	// Returns:
	// One of mtgl_key_state in mtgl_input_defs.h.
	int mtgl_get_key(mtglwin *win, int key);

	// Get the state of a mouse button for a window.
	//
	// Parameters:
	// - win: The window to get the mouse button state for.
	// - key: The mouse button to get the state of, see mtgl_mouse_button in
	// 		 mtgl_input_defs.h.
	//
	// Returns:
	// One of mtgl_key_state in mtgl_input_defs.h.
	int mtgl_get_mouse_button(mtglwin *win, int key);

	// Return whether or not a window has focus.
	//
	// Parameters:
	// - win: The window to check.
	//
	// Returns:
	// 1 if the window has focus, 0 otherwise.
	int mtgl_has_focus(mtglwin *win);

	// Get the time since the window was created, in seconds.
	//
	// Parameters:
	// - win: The window to get the time for.
	//
	// Returns:
	// The time since the window was created, in seconds.
	float mtgl_get_time(mtglwin *win);

	// Return the number of joysticks connected to a window.
	//
	// Parameters:
	// - win: The window to get the number of joysticks for.
	//
	// Returns:
	// The number of joysticks connected to a window.
	int mtgl_get_joystick_count(mtglwin *win);

	// Get information about a joystick connected to a window.
	//
	// Parameters:
	// - win: The window to get the joystick info for.
	// - id: The id of the joystick to get the info for. See mtgl_joystick_id
	// 		 in mtgl_input_defs.h.
	// - info: A pointer to a mtgljoystickinfo struct to store the info in.
	//
	// Returns:
	// One of mtgl_device_state in mtgl_input_defs.h.
	int mtgl_get_joystick_info(mtglwin *win, int id, mtgljoystickinfo *info);

	// Get the raw state of a joystick connected to a window.
	//
	// Parameters:
	// - win: The window to get the joystick raw state for.
	// - id: The id of the joystick to get the raw state for. See
	// 		 mtgl_joystick_id in mtgl_input_defs.h.
	// - state: A pointer to a mtglrawjoystickstate struct to store the raw
	// 		    state in.
	//
	// Returns:
	// One of mtgl_device_state in mtgl_input_defs.h.
	int mtgl_get_joystick_raw_state(mtglwin *win, int, mtglrawjoystickstate *state);

	// Get the state of a joystick connected to a window. This applies additional
	// processing to the raw state, such as deadzones.
	//
	// Parameters:
	// - win: The window to get the joystick state for.
	// - id: The id of the joystick to get the state for. See mtgl_joystick_id
	// 		 in mtgl_input_defs.h.
	// - state: A pointer to a mtgljoystickstate struct to store the state in.
	//
	// Returns:
	// One of mtgl_device_state in mtgl_input_defs.h.
	int mtgl_get_joystick_state(mtglwin *win, int id, mtgljoystickstate *state);

	// Destroy a window. All OpenGL contexts created with this window must be
	// destroyed before this function is called.
	//
	// Parameters:
	// - win: The window to destroy.
	void mtgl_win_destroy(mtglwin *win);

	//////////////////////////////////////////////////////////////////////////
	// OpenGL Context Functions
	//////////////////////////////////////////////////////////////////////////

	// Retrieve the default context init args.
	//
	// Parameters:
	// - argsp: A pointer to a mtglctxinitargs struct to store the default
	// 			init args in.
	void mtgl_ctx_get_default_init_args(mtglctxinitargs *args);

	// Create an OpenGL context for a window. This will set the main OpenGL context
	// for the window. If the window already has a main OpenGL context, the function
	// will fail. The context must be destroyed with mtgl_ctx_destroy before the
	// window is destroyed.
	//
	// Parameters:
	// - win: The window to create the context for.
	// - ver_major: The major version of OpenGL to create the context for.
	// - ver_minor: The minor version of OpenGL to create the context for.
	// - policy: The context policy. One of mtglctxpolicy. Pass 0 for default.
	// - argsp: A pointer to a mtglctxinitargs struct to use to initialize the
	// 			context. If NULL, the default init args will be used. The native
	// 			window system is free to ignore any of the init args - these
	// 			are only hints.
	//
	// Returns:
	// A pointer to the OpenGL context, or NULL if the context could not be
	// created.
	mtglctx *mtgl_ctx_create(mtglwin *win, int ver_major, int ver_minor, int policy, mtglctxinitargs *argsp);

	// Create an OpenGL context that shares resources with another OpenGL context.
	// The context must be destroyed with mtgl_ctx_destroy before the window is
	// destroyed.
	//
	// Parameters:
	// - ctx: The OpenGL context to share resources with.
	//
	// Returns:
	// A pointer to the OpenGL context, or NULL if the context could not be
	// created.
	mtglctx *mtgl_ctx_clone(mtglctx *ctx);

	// Acquire the an OpenGL context. This must be called before any OpenGL
	// functions are called on the context. The context must be released with
	// mtgl_ctx_release when it is no longer needed. If the context is already
	// acquired by another thread, this function will block until the context
	// is released. This call is reentrant, so it is safe to call this function
	// multiple times from the same thread. However, each call to this function
	// must be matched with a call to mtgl_ctx_release.
	//
	// Parameters:
	// - ctx: The OpenGL context to acquire.
	void mtgl_ctx_acquire(mtglctx *ctx);

	// Try to acquire the an OpenGL context. This is similar to mtgl_ctx_acquire,
	// except that it will only acquire the context if it is not already acquired
	// by another thread. If the context is already acquired by another thread,
	// this function will return immediately without blocking.
	//
	// Parameters:
	// - ctx: The OpenGL context to acquire.
	//
	// Returns:
	// 1 if the context was acquired, 0 if it was not.
	int mtgl_ctx_try_acquire(mtglctx *ctx);

	// Release an OpenGL context. This must be called when the context is no
	// longer needed. This call is reentrant, so it is safe to call this function
	// multiple times from the same thread. However, each call to this function
	// must be matched with a call to mtgl_ctx_acquire or a call to mtgl_ctx_try_acquire
	// that returns 1.
	//
	// Parameters:
	// - ctx: The OpenGL context to release.
	void mtgl_ctx_release(mtglctx *ctx);

	// Run the context scheduler. Only used when a context is created with
	// mtgl_ctx_policy_use_scheduler policy flag. Must be called from the thread
	// which created the context and the context cannot be a context created from
	// mtgl_ctx_clone. This function looks for any threads waiting on the OpenGL
	// context and decides whether it should be allowed to have it. This call
	// blocks if mtgl_ctx_acquire_always is passed to the acquisition parameter and
	// the context is in use. This, however, is bad practice and defeats the purpose
	// of the scheduler.
	// 
	// Parameters:
	// - ctx: The OpenGL context to run the scheduler on.
	// - acquisition: How the context should be acquired. One of mtglacquirebehavior.
	// 
	// Returns:
	// One of mtglacquireresult describing the context acquisition.
	int mtgl_ctx_sched(mtglctx *ctx, int acquisition);

	// Set the swap interval for an OpenGL context.
	//
	// Parameters:
	// - ctx: The OpenGL context to set the swap interval for.
	// - interval: The swap interval to set. A value of 1 means
	// 			   that a call to mtgl_swap_buffers will block
	//	           until the next vertical blanking period. Values
	// 			   greater than 1 mean that there will be multiple
	// 			   vertical blanking periods between calls to
	// 			   mtgl_swap_buffers. If the value is 0, no waiting
	// 			   will be done, and the call to mtgl_swap_buffers
	// 			   will immediately display the new frame. This
	// 			   cannot be less than 0.
	void mtgl_ctx_set_swap_interval(mtglctx *ctx, int interval);

	// Destroy an OpenGL context. Contexts created with mtgl_ctx_clone
	// must be destroyed before the context they were cloned from.
	//
	// Parameters:
	// - ctx: The OpenGL context to destroy.
	void mtgl_ctx_destroy(mtglctx *ctx);

	// Retrieve a pointer to an OpenGL function. This is useful
	// for using with OpenGL loaders.
	//
	// Parameters:
	// - name: The name of the OpenGL function to retrieve.
	//
	// Returns:
	// A pointer to the OpenGL function, or NULL if the function
	// could not be retrieved.
	void *mtgl_ctx_get_proc(const char *name);

	//////////////////////////////////////////////////////////////////////////
	// Synchronization Functions
	//////////////////////////////////////////////////////////////////////////

	// Create a lock.
	//
	// Returns:
	// A pointer to the lock, or NULL if the lock could not be
	// created.
	mtgllock *mtgl_lock_create();

	// Acquire a lock. The lock is reentrant, so it is safe to
	// call this function multiple times from the same thread.
	// However, each call to this function must be matched with
	// a call to mtgl_lock_release.
	//
	// Parameters:
	// - lock: The lock to acquire.
	void mtgl_lock_acquire(mtgllock *lock);

	// Try to acquire a lock. This is similar to mtgl_lock_acquire,
	// except that it will only acquire the lock if it is not already
	// acquired by another thread. If the lock is already acquired by
	// another thread, this function will return immediately without
	// blocking.
	//
	// Parameters:
	// - lock: The lock to acquire.
	//
	// Returns:
	// 1 if the lock was acquired, 0 if it was not.
	int mtgl_lock_try_acquire(mtgllock *lock);
	
	// Release a lock. The lock is reentrant, so it is safe to
	// call this function multiple times from the same thread.
	// However, each call to this function must be matched with
	// a call to mtgl_lock_acquire or a call to mtgl_lock_try_acquire
	// that returns 1.
	//
	// Parameters:
	// - lock: The lock to release.
	void mtgl_lock_release(mtgllock *lock);

	// Destroy a lock.
	//
	// Parameters:
	// - lock: The lock to destroy.
	void mtgl_lock_destroy(mtgllock *lock);

	// Create a condition variable.
	//
	// Returns:
	// A pointer to the condition variable, or NULL if the condition
	// variable could not be created.
	mtglcondition *mtgl_condition_create();

	// Wait for a condition variable to be signaled. The lock must
	// be acquired before calling this function. This function will
	// release the lock while waiting, and will reacquire the lock
	// before returning.
	//
	// Parameters:
	// - condition: The condition variable to wait for.
	// - lock: The lock to release while waiting.
	void mtgl_condition_wait(mtglcondition *condition, mtgllock *lock);
	
	// Signal a condition variable. This will wake up one thread
	// waiting on the condition variable.
	//
	// Parameters:
	// - condition: The condition variable to signal.
	void mtgl_condition_signal(mtglcondition *condition);

	// Signal a condition variable. This will wake up all threads
	// waiting on the condition variable.
	//
	// Parameters:
	// - condition: The condition variable to signal.
	void mtgl_condition_signal_all(mtglcondition *condition);

	// Destroy a condition variable.
	//
	// Parameters:
	// - condition: The condition variable to destroy.
	void mtgl_condition_destroy(mtglcondition *condition);

#ifdef __cplusplus
}
#endif