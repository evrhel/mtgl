#include "mtgl_win_x11.h"

#include <stdlib.h>

#if __linux__

struct mtglwin_x11 *mtgl_win_create_x11(const char *title, int width, int height, int flags, int device, void *user_data)
{
    struct mtglwin_x11 *winx11 = 0;

    return winx11;
failure:

    free(winx11);

    return 0;
}

#endif