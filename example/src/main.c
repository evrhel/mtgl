#include <mtgl/mtgl.h>

#if _WIN32
#include <Windows.h>
#elif __posix__ || __linx__ || __APPLE__
#include <unistd.h>
#include <pthread.h>
#endif

#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct ctx
{
	mtglwin *win;
	mtglctx *ctx;

	struct
	{
		GLuint screen;
		GLint screen_color_unif;
	} programs;

	struct
	{
		GLuint tri_vao;
		GLuint tri_vbo;
	} objects;

#if _WIN32
	HANDLE hWorker;
#elif __posix__ || __linux__ || __APPLE__
	pthread_t worker;
#endif
};

extern GLuint compile_shader(const char *name, GLenum type);
extern GLuint link_shaders(GLuint vert, GLuint frag);

#if _WIN32
static int loader_worker(struct ctx *prog_ctx);
#elif __posix__ || __linux__ || __APPLE__
static void *loader_worker(void *ptr);
#endif

static void
window_resized(mtglwin *win, int width, int height)
{
	struct ctx *ctx = mtgl_get_user_data(win);

	mtgl_ctx_acquire(ctx->ctx);
	glViewport(0, 0, width, height);
	mtgl_ctx_release(ctx->ctx);
}

static int
load_shaders(struct ctx *prog_ctx)
{
	GLuint vert, frag;
	GLuint program;

	vert = compile_shader("shaders/screen_vert.glsl", GL_VERTEX_SHADER);
	frag = compile_shader("shaders/screen_frag.glsl", GL_FRAGMENT_SHADER);
	program = link_shaders(vert, frag);
	prog_ctx->programs.screen = program;

	return 0;
}

/* Destroy shaders */
static void
destroy_shaders(struct ctx *prog_ctx)
{
	if (prog_ctx->programs.screen)
	{
		glDeleteShader(prog_ctx->programs.screen);
		prog_ctx->programs.screen = 0;
	}
}

static void
char_callback(mtglwin *win, unsigned int code, int repeat_count, int mods)
{
	for (; repeat_count > 0; repeat_count--)
		putchar(code);
}

static void
device_event_callback(mtglwin *win, enum mtgl_device_type type, enum mtgl_device_state state, int id)
{
	if (state)
		printf("device %d disconnected\n", id);
	else
		printf("device %d connected\n", id);
}

static const char *
device_type_string(enum mtgl_device_type type)
{
	switch (type)
	{
	case mtgl_device_type_none: return "none";
	case mtgl_device_type_graphics: return "graphics";
	case mtgl_device_type_mouse: return "mouse";
	case mtgl_device_type_keyboard: return "keyboard";
	case mtgl_device_type_joystick: return "joystick";
	case mtgl_device_type_audio_out: return "audio out";
	case mtgl_device_type_audio_in: return "audio in";
	case mtgl_device_type_any: return "any";
	default: return "unknown";
	}
}

int
main(int argc, char *argv[])
{
	const float one_third_2pi = 2.0943951f;
	const int max_frame_skips = 5;
	struct ctx prog_ctx = { 0 };
	int width, height;
	GLfloat r, g, b;
	int skipped_frames = 0;
	int total_skipped_frames = 0;
	int num_frames = 0;
	float start, end;
	int acquired;
	void *it = 0;
	mtgldevice device;
	mtgljoystickinfo jsinfo;
	mtglctxinitargs args;

	if (!mtgl_init())
	{
		fprintf(stderr, "Failed to initialize mtgl\n");
		return 1;
	}

	while ((it = mtgl_enumerate_devices(it, &device, mtgl_device_type_any)))
	{
		printf("%s device:\n", device_type_string(device.type));
		printf("  id = %d\n", device.id);
		printf("  name: %s\n", device.name);
		printf("  string: %s\n", device.string);
		printf("  is_primary: %d\n", device.is_primary);
	}
	mtgl_enumerate_devices_done(it);

	/* 8x MSAA on the default framebuffer */
	mtgl_ctx_get_default_init_args(&args);
	args.allow_sampling = 1;
	args.sample_count = 8;

	/* create a window */
	prog_ctx.win = mtgl_win_create("OpenGL Window", 800, 600, 0, 0, &prog_ctx);
	if (!prog_ctx.win)
	{
		fprintf(stderr, "Failed to create window\n");
		return 1;
	}
	
	printf("window created\n");

	/* create OpenGL context from window */
	prog_ctx.ctx = mtgl_ctx_create(prog_ctx.win, 3, 3, 0, &args);
	if (!prog_ctx.ctx)
	{
		fprintf(stderr, "Failed to create OpenGL context\n");
		mtgl_win_destroy(prog_ctx.win);
		return 1;
	}

	printf("context created\n");

	mtgl_get_joystick_info(prog_ctx.win, mtgl_joystick1, &jsinfo);

	mtgl_set_event_callback(prog_ctx.win, mtgl_event_resize, window_resized);
	mtgl_set_event_callback(prog_ctx.win, mtgl_event_char, char_callback);

	printf("callbacks set\n");

	mtgl_ctx_set_swap_interval(prog_ctx.ctx, 1); // enable vsync

	mtgl_ctx_acquire(prog_ctx.ctx);

	gladLoadGLLoader(mtgl_ctx_get_proc); // load OpenGL functions

	printf("OpenGL functions loaded\n");

	/* Print OpenGL info */
	printf("OpenGL:  %s\n", glGetString(GL_VERSION));
	printf("GLSL:    %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	printf("Vendor:  %s\n", glGetString(GL_VENDOR));

	glEnable(GL_MULTISAMPLE);

	mtgl_ctx_release(prog_ctx.ctx);

	printf("done opengl setup\n");

	/* create a worker thread to load OpenGL resources */
#if _WIN32
	prog_ctx.hWorker = CreateThread(0, 0, loader_worker, &prog_ctx, 0, 0);
	if (!prog_ctx.hWorker)
	{
		fprintf(stderr, "CreateThread failed");
		mtgl_ctx_destroy(prog_ctx.ctx);
		mtgl_win_destroy(prog_ctx.win);
		return 1;
	}
#elif __posix__ || __linux__ || __APPLE__
	pthread_create(&prog_ctx.worker, 0, &loader_worker, &prog_ctx);
#endif

	mtgl_show_window(prog_ctx.win, 1); // show the window

	start = mtgl_get_time(prog_ctx.win);

	printf("main loop\n");

	/* loop until the window should close */
	while (!mtgl_should_close(prog_ctx.win))
	{
		mtgl_get_size(prog_ctx.win, &width, &height);
	
		r = sinf(mtgl_get_time(prog_ctx.win)) * 0.5f + 0.5f;
		g = sinf(mtgl_get_time(prog_ctx.win) + one_third_2pi) * 0.5f + 0.5f;
		b = sinf(mtgl_get_time(prog_ctx.win) + 2.0f * one_third_2pi) * 0.5f + 0.5f;

		if (skipped_frames < max_frame_skips)
			acquired = mtgl_ctx_try_acquire(prog_ctx.ctx);
		else
		{
			skipped_frames = 0;
			mtgl_ctx_acquire(prog_ctx.ctx);
			acquired = 1;
		}

		if (acquired)
		{
			glClearColor(r, g, b, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			/* draw the triangle */
			if (prog_ctx.objects.tri_vao && prog_ctx.programs.screen)
			{
				glUseProgram(prog_ctx.programs.screen);

				glUniform3f(prog_ctx.programs.screen_color_unif, r, g, b);

				glBindVertexArray(prog_ctx.objects.tri_vao);
				glDrawArrays(GL_TRIANGLES, 0, 3);
			}

			mtgl_ctx_release(prog_ctx.ctx);
			acquired = 0;

			/* swap front and back buffers */
			mtgl_swap_buffers(prog_ctx.win);
		}
		else
		{
			skipped_frames++;
			total_skipped_frames++;
		}

		/* poll window events */
		mtgl_poll_events(prog_ctx.win);

		/* close the window if escape key is pressed */
		if (mtgl_get_key(prog_ctx.win, mtgl_escape))
			mtgl_set_should_close(prog_ctx.win, 1);

		num_frames++;
	}

	end = mtgl_get_time(prog_ctx.win);

#if _WIN32
	CloseHandle(prog_ctx.hWorker);
	prog_ctx.hWorker = 0;
#elif __posix__ || __linux__ || __APPLE__
	pthread_detach(prog_ctx.worker);
	prog_ctx.worker = 0;
#endif

	/* cleanup OpenGL resources */
	mtgl_ctx_acquire(prog_ctx.ctx);

	glDeleteBuffers(1, &prog_ctx.objects.tri_vbo);
	glDeleteVertexArrays(1, &prog_ctx.objects.tri_vao);

	destroy_shaders(&prog_ctx);

	mtgl_ctx_release(prog_ctx.ctx);

	/* destroy the OpenGL context and window */
	mtgl_ctx_destroy(prog_ctx.ctx);
	mtgl_win_destroy(prog_ctx.win);

	mtgl_done();

	printf("frames:    %d\n", num_frames);
	printf("skipped:   %d\n", total_skipped_frames);
	printf("avg fps:   %f\n", (double)(num_frames / (end - start)));

	return 0;
}

#if _WIN32
static int
loader_worker(struct ctx *prog_ctx)
{
#elif __posix__ || __linux__ || __APPLE__
static void *
loader_worker(void *ptr)
{
	struct ctx *prog_ctx = ptr;
#endif	
	mtglwin *win = prog_ctx->win;
	mtglctx *ctx = prog_ctx->ctx;

	/* triangle vertex data */
	GLfloat vbo_data[] = {
		// pos			// color
		-1.0f, -1.0f,	1.0f, 0.0f, 0.0f,
		0.0f, 1.0f,		0.0f, 1.0f, 0.0f,
		1.0f, -1.0f,	0.0f, 0.0f, 1.0f
	};

	mtgl_ctx_acquire(ctx);

	printf("Loading shaders\n");
	load_shaders(prog_ctx);
	prog_ctx->programs.screen_color_unif = glGetUniformLocation(prog_ctx->programs.screen, "uColor");
	printf("Done loading\n");

	mtgl_ctx_release(ctx);

	// some long cpu work (maybe something to do with the triangle created below, but not on GPU)
	printf("Doing some 'intensive' cpu work\n");
#if _WIN32
	Sleep(3000);
#elif __posix__ || __linux__ || __APPLE__
	sleep(3);
#endif
	printf("Work done\n");

	mtgl_ctx_acquire(ctx);

	printf("Loading triangle on another thread!\n");

	/* Create the triangle */
	glGenVertexArrays(1, &prog_ctx->objects.tri_vao);
	glGenBuffers(1, &prog_ctx->objects.tri_vbo);

	glBindVertexArray(prog_ctx->objects.tri_vao);

	glBindBuffer(GL_ARRAY_BUFFER, prog_ctx->objects.tri_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vbo_data), vbo_data, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)(0 * sizeof(GLfloat)));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	printf("Triangle done loading!\n");

	mtgl_ctx_release(ctx);

	return 0;
}
