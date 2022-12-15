#include <mtgl/mtgl.h>

#include <Windows.h>
#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct ctx
{
	glwin *win;
	glctx *ctx;

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

	glthread *worker;
};

extern GLuint compile_shader(const char *name, GLenum type);
extern GLuint link_shaders(GLuint vert, GLuint frag);

static int loader_worker(struct ctx *prog_ctx);

static void
window_resized(glwin *win, int width, int height)
{
	struct ctx *ctx = glwin_get_user_data(win);

	glctx_acquire(ctx->ctx);
	glViewport(0, 0, width, height);
	glctx_release(ctx->ctx);
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
char_callback(glwin *win, unsigned int code, int repeat_count, int mods)
{
	for (; repeat_count > 0; repeat_count--)
		putchar(code);
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

	mtgl_init();

	/* create a window and OpenGL context */
	prog_ctx.win = glwin_create("OpenGL Window", 800, 600, &prog_ctx);
	prog_ctx.ctx = glctx_create(prog_ctx.win, 3, 3);

	glwin_set_event_callback(prog_ctx.win, glwin_event_resize, window_resized);
	glwin_set_event_callback(prog_ctx.win, glwin_event_char, char_callback);

	glctx_set_swap_interval(prog_ctx.ctx, 1); // enable vsync

	glctx_acquire(prog_ctx.ctx);

	gladLoadGLLoader(glctx_get_proc); // load OpenGL functions

	/* Print OpenGL info */
	printf("OpenGL:  %s\n", glGetString(GL_VERSION));
	printf("GLSL:    %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	glctx_release(prog_ctx.ctx);

	/* create a worker thread to load OpenGL resources */
	prog_ctx.worker = glthread_create(prog_ctx.ctx, loader_worker, &prog_ctx);

	glwin_show_window(prog_ctx.win, 1); // show the window

	start = glwin_get_time(prog_ctx.win);

	/* loop until the window should close */
	while (!glwin_should_close(prog_ctx.win))
	{
		glwin_get_size(prog_ctx.win, &width, &height);

		r = sinf(glwin_get_time(prog_ctx.win)) * 0.5f + 0.5f;
		g = sinf(glwin_get_time(prog_ctx.win) + one_third_2pi) * 0.5f + 0.5f;
		b = sinf(glwin_get_time(prog_ctx.win) + 2.0f * one_third_2pi) * 0.5f + 0.5f;

		if (skipped_frames < max_frame_skips)
			acquired = glctx_try_acquire(prog_ctx.ctx);
		else
		{
			skipped_frames = 0;
			glctx_acquire(prog_ctx.ctx);
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

			glctx_release(prog_ctx.ctx);
			acquired = 0;

			/* swap front and back buffers */
			glwin_swap_buffers(prog_ctx.win);
		}
		else
		{
			skipped_frames++;
			total_skipped_frames++;
		}

		/* poll window events */
		glwin_poll_events(prog_ctx.win);

		/* close the window if escape key is pressed */
		if (glwin_get_key(prog_ctx.win, glwin_escape))
			glwin_set_should_close(prog_ctx.win, 1);

		num_frames++;
	}

	end = glwin_get_time(prog_ctx.win);

	glthread_detach(prog_ctx.worker);
	prog_ctx.worker = 0;

	/* cleanup OpenGL resources */
	glctx_acquire(prog_ctx.ctx);

	glDeleteBuffers(1, &prog_ctx.objects.tri_vbo);
	glDeleteVertexArrays(1, &prog_ctx.objects.tri_vao);

	destroy_shaders(&prog_ctx);
	glctx_release(prog_ctx.ctx);

	/* destroy the OpenGL context and window */
	glctx_destroy(prog_ctx.ctx);
	glwin_destroy(prog_ctx.win);

	mtgl_done();

	printf("frames:    %d\n", num_frames);
	printf("skipped:   %d\n", total_skipped_frames);
	printf("avg fps:   %f\n", (double)(num_frames / (end - start)));

	return 0;
}

static int
loader_worker(struct ctx *prog_ctx)
{
	glwin *win = prog_ctx->win;
	glctx *ctx = prog_ctx->ctx;

	/* triangle vertex data */
	GLfloat vbo_data[] = {
		// pos			// color
		-1.0f, -1.0f,	1.0f, 0.0f, 0.0f,
		0.0f, 1.0f,		0.0f, 1.0f, 0.0f,
		1.0f, -1.0f,	0.0f, 0.0f, 1.0f
	};

	glctx_acquire(ctx);

	printf("Loading shaders\n");
	load_shaders(prog_ctx);
	prog_ctx->programs.screen_color_unif = glGetUniformLocation(prog_ctx->programs.screen, "uColor");
	printf("Done loading\n");

	glctx_release(ctx);

	// some long cpu work (maybe something to do with the triangle created below, but not on GPU)
	printf("Doing some 'intensive' cpu work\n");
	Sleep(3000);
	printf("Work done\n");

	glctx_acquire(ctx);

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

	glctx_release(ctx);

	return 0;
}
