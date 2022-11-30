#include "mtgl.h"

#include <glad/glad.h>

int main(int argc, char *argv[])
{
	glwin *win;
	glctx *ctx;
	int width, height;

	mtgl_init();

	win = glwin_create(600, 800);
	ctx = glctx_create(win, 3, 3);

	glwin_show_window(win, 1);

	while (!glwin_should_close(win))
	{
		glwin_get_size(win, &width, &height);

		glctx_acquire(ctx);
		glViewport(0, 0, width, height);
		glClearColor(1, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		glctx_release(ctx);

		glwin_swap_buffers(win);
		glwin_poll_events(win);
	}

	glctx_destroy(ctx);
	glwin_destroy(win);

	mtgl_done();

	return 0;
}