#include <mtgl/mtgl.h>

#if _WIN32
#include <Windows.h>

struct device_iterator_win32
{
	int current_device;
	int filter;
	int first;

	union
	{
		DWORD display_device_id;
		struct
		{
			PRAWINPUTDEVICELIST input_device_list;
			UINT num_input_devices;
			UINT input_device_idx;
		};
	};
};

#endif

static mtgllock *lock = 0;

#define rnd(dt, filter) { *(filter) &= ~(dt); return (dt); }
static inline int
next_device(int *filter)
{
	if (*filter & mtgl_device_type_graphics) rnd(mtgl_device_type_graphics, filter)
	else if (*filter & mtgl_device_type_mouse) rnd(mtgl_device_type_mouse, filter)
	else if (*filter & mtgl_device_type_keyboard) rnd(mtgl_device_type_keyboard, filter)
	else if (*filter & mtgl_device_type_joystick) rnd(mtgl_device_type_joystick, filter)
	else if (*filter & mtgl_device_type_audio_out) rnd(mtgl_device_type_audio_out, filter)
	else if (*filter & mtgl_device_type_audio_in) rnd(mtgl_device_type_audio_in, filter)
	else rnd(mtgl_device_type_none, filter)
}

int
mtgl_init()
{
	lock = mtgl_lock_create();
	if (!lock) return 0;
	return 1;
}

mtgllock *
mtgl_get_lock()
{
	return lock;
}

void *
mtgl_enumerate_devices(void *it, mtgldevice *device, int filter)
{
#if _WIN32
	struct device_iterator_win32 *dit = it;
	DISPLAY_DEVICEA display_device;
	BOOL bResult;
	UINT uiResult;
	UINT uiSize;
	RID_DEVICE_INFO di;
	PRAWINPUTDEVICELIST pridl;

	if (filter == mtgl_device_type_none)
	{
		mtgl_enumerate_devices_done(it);
		return 0;
	}

	if (!dit)
	{
		dit = malloc(sizeof(struct device_iterator_win32));
		if (!dit) return 0;

		dit->filter = filter;
		dit->first = 1;
		dit->current_device = next_device(&dit->filter);
	}

	memset(device, 0, sizeof(mtgldevice));

	switch (dit->current_device)
	{
	case mtgl_device_type_graphics: {
		if (dit->first)
		{
			dit->display_device_id = 0;
			dit->first = 0;
		}

		display_device.cb = sizeof(DISPLAY_DEVICEA);

	retry_graphics:
		bResult = EnumDisplayDevicesA(NULL, dit->display_device_id,
			&display_device, 0);

		if (!bResult)
		{
			dit->current_device = next_device(&dit->filter);
			dit->first = 1;
			return mtgl_enumerate_devices(it, device, filter);
		}

		dit->display_device_id++;
		if (!(display_device.StateFlags & DISPLAY_DEVICE_ACTIVE))
			goto retry_graphics;

		device->type = mtgl_device_type_graphics;
		device->id = dit->display_device_id;
		device->is_primary = !!(display_device.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE);
		strcpy_s(device->name, sizeof(device->name), display_device.DeviceName);
		strcpy_s(device->string, sizeof(device->string), display_device.DeviceString);

		break;
	}
	case mtgl_device_type_mouse:
	case mtgl_device_type_keyboard: {
		if (dit->first)
		{
			uiResult = GetRawInputDeviceList(NULL, &dit->num_input_devices, sizeof(RAWINPUTDEVICELIST));
			if (uiResult == -1)
			{
				dit->current_device = next_device(&dit->filter);
				dit->first = 1;
				return mtgl_enumerate_devices(it, device, filter);
			}

			dit->input_device_list = malloc(dit->num_input_devices * sizeof(RAWINPUTDEVICELIST));
			if (!dit->input_device_list)
			{
				dit->current_device = next_device(&dit->filter);
				dit->first = 1;
				return mtgl_enumerate_devices(it, device, filter);
			}

			uiResult = GetRawInputDeviceList(
				dit->input_device_list,
				&dit->num_input_devices, sizeof(RAWINPUTDEVICELIST));

			if (uiResult != dit->num_input_devices)
			{
				free(dit->input_device_list);
				dit->current_device = next_device(&dit->filter);
				return mtgl_enumerate_devices(it, device, filter);
			}

			dit->input_device_idx = 0;

			dit->first = 0;
		}

		device->type = dit->current_device;
		device->is_primary = -1;
		for (; dit->input_device_idx < dit->num_input_devices; dit->input_device_idx++)
		{
			pridl = &dit->input_device_list[dit->input_device_idx];

			memset(&di, 0, sizeof(di));

			uiSize = sizeof(di);
			uiResult = GetRawInputDeviceInfoA(
				pridl->hDevice,
				RIDI_DEVICEINFO, &di, &uiSize);
			if (!uiResult || uiResult == -1) continue;

			uiSize = sizeof(device->name) / sizeof(device->name[0]);
			uiResult = GetRawInputDeviceInfoA(
				pridl->hDevice,
				RIDI_DEVICENAME, device->name, &uiSize);
			if (!uiResult || uiResult == -1) continue;

			device->id = dit->input_device_idx;

			if (di.dwType == RIM_TYPEMOUSE && dit->current_device != mtgl_device_type_mouse) continue;
			if (di.dwType == RIM_TYPEKEYBOARD && dit->current_device != mtgl_device_type_keyboard) continue;

			dit->input_device_idx++;
			return dit;
		}

		free(dit->input_device_list);
		dit->current_device = next_device(&dit->filter);
		dit->first = 1;
		return mtgl_enumerate_devices(it, device, filter);
		break;
	}
	default: {
		mtgl_enumerate_devices_done(it);
		return 0;
	}
	}

	return dit;
#else
	return 0;
#endif
}

void
mtgl_enumerate_devices_done(void *it)
{
#if _WIN32
	struct device_iterator_win32 *dit = it;

	if (it)
	{
		if (dit->current_device & (mtgl_device_type_mouse | mtgl_device_type_keyboard))
			free(dit->input_device_list);

		free(it);
	}
#endif
}
/*int
mtgl_get_joystick_count()
{
	return joyGetNumDevs();
}*/

/*int
mtgl_get_joystick_info(enum glwin_joystick_id id, glwinjoystickinfo *info)
{
	JOYCAPSA caps;
	MMRESULT result;

	if (id == -1) return mtgl_device_disconnected;

	result = joyGetDevCapsA(id, &caps, sizeof(JOYCAPS2));
	if (result != JOYERR_NOERROR) return mtgl_device_disconnected;

	memset(info, 0, sizeof(*info));

	info->xmin = caps.wXmin; info->xmax = caps.wXmax;
	info->ymin = caps.wYmin; info->ymax = caps.wYmax;
	info->zmin = caps.wZmin; info->zmax = caps.wZmax;

	info->rmin = caps.wRmin; info->rmax = caps.wRmax;
	info->umin = caps.wUmin; info->umax = caps.wUmax;
	info->vmin = caps.wVmin; info->vmax = caps.wVmax;
	
	info->poll_min = caps.wPeriodMin; info->poll_max = caps.wPeriodMax;

	info->button_count = caps.wNumButtons; info->max_buttons = caps.wMaxButtons;
	info->num_axes = caps.wNumAxes; info->max_axes = caps.wMaxAxes;

	info->manufacturer_id = caps.wMid;
	info->product_id = caps.wPid;

	strcpy_s(info->manufacturer, sizeof(info->manufacturer), caps.szOEMVxD);
	strcpy_s(info->product, sizeof(info->product), caps.szPname);

	return mtgl_device_connected;
}*/
/*
int
glwin_get_joystick_raw_state(glwin *win, enum glwin_joystick_id id, mtglrawjoystickstate *state)
{
	JOYINFOEX info;
	MMRESULT result;

	info.dwSize = sizeof(info);
	info.dwFlags = JOY_RETURNALL;

	result = joyGetPosEx(id, &info);
	switch (result)
	{
	case JOYERR_NOERROR: break;
	case JOYERR_UNPLUGGED:
	case MMSYSERR_INVALPARAM:
	case MMSYSERR_BADDEVICEID: return mtgl_device_disconnected;
	case MMSYSERR_NODRIVER: return mtgl_device_driver_error;
	default: return mtgl_device_error;
	}

	state->buttons = info.dwButtons;
	state->xpos = info.dwXpos;
	state->ypos = info.dwYpos;
	state->zpos = info.dwZpos;

	return mtgl_device_connected;
}
*/
void
mtgl_done()
{
	if (lock) mtgl_lock_destroy(lock);
	lock = 0;
}