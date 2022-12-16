#include <mtgl/mtgl.h>

#include <Windows.h>

struct device_iterator
{
	enum mtgl_device_type current_device;
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

static gllock *lock;

#define rnd(dt, filter) { *(filter) &= ~(dt); return (dt); }
static inline enum mtgl_device_type next_device(int *filter)
{
	if (*filter & mtgl_device_type_graphics) rnd(mtgl_device_type_graphics, filter)
	else if (*filter & mtgl_device_type_mouse) rnd(mtgl_device_type_mouse, filter)
	else if (*filter & mtgl_device_type_keyboard) rnd(mtgl_device_type_keyboard, filter)
	else if (*filter & mtgl_device_type_gamepad) rnd(mtgl_device_type_gamepad, filter)
	else if (*filter & mtgl_device_type_audio_out) rnd(mtgl_device_type_audio_out, filter)
	else if (*filter & mtgl_device_type_audio_in) rnd(mtgl_device_type_audio_in, filter)
	else rnd(mtgl_device_type_none, filter)
}

int
mtgl_init()
{
	lock = gllock_create();
	if (!lock) return 0;
	return 1;
}

gllock *
mtgl_get_lock()
{
	return lock;
}

void *
mtgl_enumerate_devices(void *it, mtgldevice *device, int filter)
{
	struct device_iterator *dit = it;
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
		dit = malloc(sizeof(struct device_iterator));
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
}

void
mtgl_enumerate_devices_done(void *it)
{
	struct device_iterator *dit = it;

	if (it)
	{
		if (dit->current_device & (mtgl_device_type_mouse | mtgl_device_type_keyboard))
			free(dit->input_device_list);

		free(it);
	}
}

void
mtgl_done()
{
	if (lock) gllock_destroy(lock);
	lock = 0;
}