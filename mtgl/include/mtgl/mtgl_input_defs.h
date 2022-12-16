#pragma once

#ifdef __cplusplus
extern "C" {
#endif

	enum mtgl_device_type
	{
		mtgl_device_type_none = 0x00,

		mtgl_device_type_graphics = 0x01,
		mtgl_device_type_mouse = 0x02,
		mtgl_device_type_keyboard = 0x04,
		mtgl_device_type_gamepad = 0x08,
		mtgl_device_type_audio_out = 0x10,
		mtgl_device_type_audio_in = 0x20,

		mtgl_device_type_any = -1
	};

	enum win_event_type
	{
		glwin_event_resize,
		glwin_event_mouse_move,
		glwin_event_key,
		glwin_event_char,
		glwin_event_mouse_button,
		glwin_event_window_event,

		glwin_event_last = 16
	};

	enum glwin_window_flag
	{
		glwin_wf_raw_keyboard_input = 0x1,
		glwin_wf_raw_mouse_input = 0x2,
		glwin_wf_raw_input_mask =
			glwin_wf_raw_keyboard_input | glwin_wf_raw_mouse_input
	};

	enum glwin_window_event
	{
		glwin_window_maximize,
		glwin_window_restore,
		glwin_window_minimize,
		glwin_window_closing,
		glwin_window_move,
		glwin_window_changefocus
	};

	enum glwin_key_state
	{
		glwin_released,
		glwin_pressed
	};

	enum glwin_mouse_button
	{
		glwin_mouse1,
		glwin_mouse2,
		glwin_mouse3,
		glwin_mouse4,
		glwin_mouse5
	};

	enum glwin_key
	{
		glwin_lbutton = 0x01,
		glwin_rbutton,
		glwin_cancel,
		glwin_mbutton,
		glwin_xbutton1,
		glwin_xbutton2,

		glwin_backspace = 0x08,
		glwin_tab,

		glwin_clear = 0x0c,
		glwin_enter, glwin_return = glwin_enter,

		glwin_shift = 0x10,
		glwin_control,
		glwin_menu,
		glwin_pause,
		glwin_caps,
		glwin_kana, glwin_hanguel = glwin_kana, glwin_hangul = glwin_kana,
		glwin_ime_on,
		glwin_junja,
		glwin_final,
		glwin_hanja, glwin_kanji = glwin_hanja,
		glwin_ime_off,
		glwin_escape,
		glwin_convert,
		glwin_nonconvert,
		glwin_accept,
		glwin_modechange,
		glwin_space,
		glwin_prior,
		glwin_next,
		glwin_end,
		glwin_home,
		glwin_left,
		glwin_up,
		glwin_right,
		glwin_down,
		glwin_select,
		glwin_print,
		glwin_execute,
		glwin_snapshot,
		glwin_insert,
		glwin_delete,
		glwin_help,
		glwin_0,
		glwin_1,
		glwin_2,
		glwin_3,
		glwin_4,
		glwin_5,
		glwin_6,
		glwin_7,
		glwin_8,
		glwin_9,

		glwin_A = 0x41,
		glwin_B,
		glwin_C,
		glwin_D,
		glwin_E,
		glwin_F,
		glwin_G,
		glwin_H,
		glwin_I,
		glwin_J,
		glwin_K,
		glwin_L,
		glwin_M,
		glwin_N,
		glwin_O,
		glwin_P,
		glwin_Q,
		glwin_R,
		glwin_S,
		glwin_T,
		glwin_U,
		glwin_V,
		glwin_W,
		glwin_X,
		glwin_Y,
		glwin_Z,
		glwin_lwin,
		glwin_rwin,
		glwin_apps,

		glwin_sleep = 0x5f,
		glwin_np0,
		glwin_np1,
		glwin_np2,
		glwin_np3,
		glwin_np4,
		glwin_np5,
		glwin_np6,
		glwin_np7,
		glwin_np8,
		glwin_np9,
		glwin_multiply,
		glwin_add,
		glwin_separator,
		glwin_subtract,
		glwin_decimal,
		glwin_divide,
		glwin_f1,
		glwin_f2,
		glwin_f3,
		glwin_f4,
		glwin_f5,
		glwin_f6,
		glwin_f7,
		glwin_f8,
		glwin_f9,
		glwin_f10,
		glwin_f11,
		glwin_f12,
		glwin_f13,
		glwin_f14,
		glwin_f15,
		glwin_f16,
		glwin_f17,
		glwin_f18,
		glwin_f19,
		glwin_f20,
		glwin_f21,
		glwin_f22,
		glwin_f23,
		glwin_f24,

		glwin_numlock = 0x90,
		glwin_scroll,

		glwin_lshift = 0xa0,
		glwin_rshift,
		glwin_lcontrol,
		glwin_rcontrol,
		glwin_lmenu,
		glwin_rmenu,
		glwin_browser_back,
		glwin_browser_forward,
		glwin_browser_refresh,
		glwin_browser_stop,
		glwin_browser_search,
		glwin_browser_favorites,
		glwin_browser_home,
		glwin_volume_mute,
		glwin_volume_down,
		glwin_volume_up,
		glwin_media_next_track,
		glwin_media_prev_track,
		glwin_media_stop,
		glwin_media_play_pause,
		glwin_launch_mail,
		glwin_launch_media_select,
		glwin_launch_app1,
		glwin_launch_app2,

		glwin_oem_1 = 0xba,
		glwin_semicolon = glwin_oem_1,
		glwin_oem_plus,
		glwin_plus = glwin_oem_plus,
		glwin_oem_comma,
		glwin_comma = glwin_oem_comma,
		glwin_oem_minus,
		glwin_minus = glwin_oem_minus,
		glwin_oem_period,
		glwin_period = glwin_oem_period,
		glwin_oem2,
		glwin_forward_slash = glwin_oem2,
		glwin_oem3,
		glwin_grave = glwin_oem3,

		glwin_oem4 = 0xdb,
		glwin_lsq_bracket = glwin_oem4,
		glwin_oem5,
		glwin_backslash = glwin_oem5,
		glwin_oem6,
		glwin_rsq_bracket = glwin_oem6,
		glwin_oem7,
		glwin_quote = glwin_oem7,
		glwin_oem8,

		glwin_oem_102 = 0xe2,

		glwin_processkey = 0xe5,

		glwin_attn = 0xf6,
		glwin_crsel,
		glwin_exsel,
		glwin_ereof,
		glwin_play,
		glwin_zoom,
		glwin_pa1,
		glwin_oem_clear
	};

#ifdef __cplusplus
}
#endif