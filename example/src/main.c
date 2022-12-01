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

int
main(int argc, char *argv[])
{
	const float one_third_2pi = 2.0943951f;
	struct ctx prog_ctx = { 0 };
	int width, height;
	GLfloat r, g, b;
	GLfloat back;

	mtgl_init();

	/* create a window and OpenGL context */
	prog_ctx.win = glwin_create(800, 600);
	prog_ctx.ctx = glctx_create(prog_ctx.win, 3, 3);

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

	/* loop until the window should close */
	while (!glwin_should_close(prog_ctx.win))
	{
		glwin_get_size(prog_ctx.win, &width, &height);

		r = sinf(glwin_get_time(prog_ctx.win)) * 0.5f + 0.5f;
		g = sinf(glwin_get_time(prog_ctx.win) + one_third_2pi) * 0.5f + 0.5f;
		b = sinf(glwin_get_time(prog_ctx.win) + 2.0f * one_third_2pi) * 0.5f + 0.5f;

		glctx_acquire(prog_ctx.ctx);

		/* update viewport to match window size*/
		glViewport(0, 0, width, height);

		glClearColor(r,g,b, 1);
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

		/* swap front and back buffers and poll window events */
		glwin_swap_buffers(prog_ctx.win);
		glwin_poll_events(prog_ctx.win);
	}

	glthread_join(prog_ctx.worker);
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

	return 0;
}

static int
loader_worker(struct ctx *prog_ctx)
{
	const float one_third_2pi = 2.0943951f;

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

	load_shaders(prog_ctx);

	prog_ctx->programs.screen_color_unif = glGetUniformLocation(prog_ctx->programs.screen, "uColor");

	glctx_release(ctx);

	// some long cpu work
	Sleep(3000);

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
