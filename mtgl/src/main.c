#include "mtgl.h"

#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static GLuint screen_program = 0;

/* Read a text file. Adds a null terminator, not included in *size. */
static char *
read_file(const char *filename, long *const size)
{
	FILE *file;
	char *buf;
	size_t read;

	fopen_s(&file, filename, "r");
	if (!file) return 0;

	fseek(file, 0, SEEK_END);
	*size = ftell(file);
	fseek(file, 0, SEEK_SET);

	buf = malloc(((size_t)*size) + 1);
	if (!buf)
	{
		fclose(file);
		return 0;
	}

	read = fread_s(buf, *size, 1, *size, file);

	fclose(file);

	buf[read] = 0;
	return buf;
}

/* Compile a shader from its source file */
static GLuint
compile_shader(const char *name, GLenum type)
{
	GLuint result;
	char *buf;
	long bufsize;
	GLint success;
	GLchar error_log[1024];

	buf = read_file(name, &bufsize);
	result = glCreateShader(type);
	if (!result)
	{
		free(buf);
		return 0;
	}

	glShaderSource(result, 1, &buf, NULL);
	glCompileShader(result);

	glGetShaderiv(result, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		printf("'%s':\n", name);
		glGetShaderInfoLog(result, sizeof(error_log), NULL, error_log);
		puts(error_log);

		glDeleteShader(result);
		return 0;
	}

	free(buf);

	return result;
}

/* Link a vertex and fragment shader and return a program */
static GLuint
link_shaders(GLuint vert, GLuint frag)
{
	GLuint program;
	GLint success;
	GLchar error_log[1024];

	if (!vert || !frag)
	{
		glDeleteShader(vert);
		glDeleteShader(frag);
		return 0;
	}

	program = glCreateProgram();
	if (!program) return 0;

	glAttachShader(program, vert);
	glAttachShader(program, frag);

	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		printf("error linking '%u' and '%u'\n", vert, frag);
		glGetProgramInfoLog(program, sizeof(error_log), NULL, error_log);
		puts(error_log);

		glDeleteProgram(program);
		glDeleteShader(vert);
		glDeleteShader(frag);
		return 0;
	}

	glDeleteShader(vert);
	glDeleteShader(frag);
	return program;
}

static int
load_shaders()
{
	GLuint vert, frag;
	GLuint program;

	vert = compile_shader("shaders/screen_vert.glsl", GL_VERTEX_SHADER);
	frag = compile_shader("shaders/screen_frag.glsl", GL_FRAGMENT_SHADER);
	program = link_shaders(vert, frag);
	if (program) screen_program = program;

	return 0;
}

/* Destroy shaders */
static void
destroy_shaders()
{
	if (screen_program)
	{
		glDeleteShader(screen_program);
		screen_program = 0;
	}
}

int
main(int argc, char *argv[])
{
	const float one_third_2pi = 2.0943951f;
	glwin *win;
	glctx *ctx;
	int width, height;
	GLfloat r, g, b;
	GLint color_unif;

	/* triangle vertex data */
	GLuint tri_vao, tri_vbo;
	GLfloat vbo_data[] = {
		// pos			// color
		-1.0f, -1.0f,	1.0f, 0.0f, 0.0f,
		0.0f, 1.0f,		0.0f, 1.0f, 0.0f,
		1.0f, -1.0f,	0.0f, 0.0f, 1.0f
	};

	mtgl_init();

	/* create a window and OpenGL context */
	win = glwin_create(800, 600);
	ctx = glctx_create(win, 3, 3);

	glctx_set_swap_interval(ctx, 1); // enable vsync

	glctx_acquire(ctx);

	/* Print OpenGL info */
	printf("OpenGL:  %s\n", glGetString(GL_VERSION));
	printf("GLSL:    %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	load_shaders();
	color_unif = glGetUniformLocation(screen_program, "uColor");

	/* Create the triangle */
	glGenVertexArrays(1, &tri_vao);
	glGenBuffers(1, &tri_vbo);

	glBindVertexArray(tri_vao);

	glBindBuffer(GL_ARRAY_BUFFER, tri_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vbo_data), vbo_data, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)(0 * sizeof(GLfloat)));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glClearColor(0, 0, 0, 1);

	glctx_release(ctx);

	glwin_show_window(win, 1); // show the window

	/* loop until the window should close */
	while (!glwin_should_close(win))
	{
		glwin_get_size(win, &width, &height);

		glctx_acquire(ctx);

		/* update viewport to match window size*/
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		/* draw the triangle */
		if (screen_program)
		{
			glUseProgram(screen_program);

			r = sinf(glwin_get_time(win)) * 0.5f + 0.5f;
			g = sinf(glwin_get_time(win) + one_third_2pi) * 0.5f + 0.5f;
			b = sinf(glwin_get_time(win) + 2.0f * one_third_2pi) * 0.5f + 0.5f;
			glUniform3f(color_unif, r, g, b);

			glBindVertexArray(tri_vao);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}

		glctx_release(ctx);

		/* swap front and back buffers and poll window events */
		glwin_swap_buffers(win);
		glwin_poll_events(win);
	}

	/* cleanup OpenGL resources */
	glctx_acquire(ctx);

	glDeleteBuffers(1, &tri_vbo);
	glDeleteVertexArrays(1, &tri_vao);

	destroy_shaders();
	glctx_release(ctx);

	/* destroy the OpenGL context and window */
	glctx_destroy(ctx);
	glwin_destroy(win);

	mtgl_done();

	return 0;
}
