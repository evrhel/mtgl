# mtgl

mtgl is a framework for using OpenGL in multithreaded programs. It
has support for creating multiple shared OpenGL contexts as well as
synchronizing OpenGL operations on the same context across different
threads.

## Usage

### Functions

Below are a list of significant functions pertaining directly to
OpenGL contexts, see `mtgl/include/mtgl.h` for all functions.

* `mtglctx *mtgl_ctx_create(glwin *win, int ver_major, int ver_minor, mtglctxinitargs *argsp)` - 
Create an OpenGL context from a window with the OpenGL version
`ver_major.ver_minor`. Can optionally specify additional context
creation options through the `argsp` parameter. Can only be done
once per window.

* `mtglctx *mtgl_ctx_clone(mtglctx *ctx)` - Clones an OpenGL context. The
new context will have all its resources shared with a context created
from `mtgl_ctx_create` or `mtgl_ctx_clone`. The context must be cloned
on the thread on which it was created.

* `void mtgl_ctx_acquire(mtglctx *ctx)` - Acquires an OpenGL context so
OpenGL calls can be subsequently made. This allows the same context
to be passed around to different threads. The call will block until
another thread calls `mtgl_ctx_release`. The call is reentrant,
meaning two calls to `mtgl_ctx_acquire` will not cause deadlock, but
the number of calls to `mtgl_ctx_acquire` must match the number of
calls to `mtgl_ctx_release`.

* `void mtgl_ctx_release(mtglctx *ctx)` - Releases a context acquired
from `mtgl_ctx_acquire` so another thread can use the context.

Behavior of OpenGL calls outside the bounds of matching
`mtgl_ctx_acquire` and `mtgl_ctx_release` calls is undefined.

### Usage in Another Project

Run `./autoconf.sh` to fetch dependencies before use.

To use mtgl, build its sources inside of your project directly
(everything in the `mtgl/src` directory). Have `mtgl/include`
somewhere in your include path so you may access `mtgl.h`.

## Examples

Example main thread usage, using [Glad](https://glad.dav1d.de/) as
a loader:
```c
#include <glad/glad.h>
#include <mtgl/mtgl.h>

int main(int argc, char *argv[])
{
	mtglwin *win;
	mtglctx *ctx;
	int width, height;

	mtgl_init(); // initialize mtgl

	win = mtgl_win_create(800, 600); // create 800x600 window
	ctx = mtgl_ctx_create(win, 3, 3, NULL); // create OpenGL 3.3 context

	mtgl_ctx_acquire(ctx); // acquire the OpenGL context
	gladLoadGLLoader(&mtgl_ctx_get_proc); // load OpenGL functions
	mtgl_ctx_release(ctx); // release the OpenGL context

	mtgl_show_window(win, 1); // show the window

	// loop until the user closes the window
	while (!glwin_should_close(win))
	{
		// get size of window
		mtgl_get_size(win, &width, &height);

		mtgl_ctx_acquire(ctx); // acquire the OpenGL context

		glClearColor(0, 0, 0, 1); // set clear color to black
		glViewport(0, 0, width, height); // set viewport to window size
		glClear(GL_COLOR_BUFFER_BIT); // clear screen

		// do some other drawing...

		mtgl_ctx_release(ctx); // release the OpenGL context

		mtgl_swap_buffers(win); // swap front and back buffers
		mtgl_poll_events(win); // poll and handle window events
	}

	mtgl_ctx_destroy(ctx);
	mtgl_win_destroy(win);

	mtgl_done(); // done with mtgl
	return 0;
}
```

`example/src/main.c` is an example program using the framework.
It creates a window, and immediately launches a worker thread to
load OpenGL resources asynchronously and simulates a long CPU task.
This program results in the render thread running without issue
while a triangle's loading time is being artificially slowed down.

<p align="center">
  <img src="images/tri.png">
</p>

### Building the Example Prorgam

Run `autoconf.sh` to fetch dependencies.

#### Windows

The program can be built from within Visual Studio 2019. Simply open
`mtgl.sln` build and run the `example` project.

#### Mac

From within the `example` directory, run `make`, then run the program
using `./bin/example`.

#### Linux

No support yet!