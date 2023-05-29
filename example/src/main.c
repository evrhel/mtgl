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
#include <memory.h>

// vertices of a cube with position and normals
const GLfloat cube_vertices[] = {
    // Positions           // Normals
    // Front face
    -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,
    
    // Back face
    -0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,
    
    // Left face
    -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,
    
    // Right face
     0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,    1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,    1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,    1.0f,  0.0f,  0.0f,
    
    // Top face
    -0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,
     0.5f,   0.5f,  0.5f,    0.0f,  1.0f,  0.0f,
     0.5f,   0.5f, -0.5f,    0.0f,  1.0f,  0.0f,
    -0.5f,   0.5f, -0.5f,    0.0f,  1.0f,  0.0f,
    
    // Bottom face
    -0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f
};

// indices of the cube triangles
const GLuint cube_indices[] = {
    // Front face
    0, 1, 2,
    2, 3, 0,
    
    // Back face
    6, 5, 4,
    4, 7, 6,
    
    // Left face
    8, 9, 10,
    10, 11, 8,
    
    // Right face
    14, 13, 12,
    12, 15, 14,
    
    // Top face
    16, 17, 18,
    18, 19, 16,
    
    // Bottom face
    22, 21, 20,
    20, 23, 22
};


struct ctx
{
	mtglwin *win;
	mtglctx *ctx;

	struct
	{
		GLuint screen;
		GLint time_unif;
		GLint model_unif;
		GLint view_unif;
		GLint proj_unif;
		GLint cam_pos_unif;
		GLint ambient_color_unif;
		GLint color_unif;
	} programs;

	struct
	{
		GLuint cube_vao;
		GLuint cube_vbo;
		GLuint cube_ebo;
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

int viewport_width = 800, viewport_height = 600;

static void
window_resized(mtglwin *win, int width, int height)
{
	struct ctx *ctx = mtgl_get_user_data(win);

	viewport_width = width;
	viewport_height = height;

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

static void
print_matrix(const char *name, const GLfloat *m)
{
	int i, j;

	printf("%s:\n", name);

	for (i = 0; i < 4; i++)
	{
		printf("| ");
		for (j = 0; j < 4; j++)
			printf("%.2f ", (double)m[i * 4 + j]);
		printf("|\n");
	}
}

static GLfloat *
mmul(const GLfloat *a, const GLfloat *b, GLfloat *out)
{
	GLfloat tmp[16];

	tmp[0] = a[0] * b[0] + a[4] * b[1] + a[8]  * b[2] + a[12] * b[3];
	tmp[1] = a[1] * b[0] + a[5] * b[1] + a[9]  * b[2] + a[13] * b[3];
	tmp[2] = a[2] * b[0] + a[6] * b[1] + a[10] * b[2] + a[14] * b[3];
	tmp[3] = a[3] * b[0] + a[7] * b[1] + a[11] * b[2] + a[15] * b[3];

	tmp[4] = a[0] * b[4] + a[4] * b[5] + a[8]  * b[6] + a[12] * b[7];
	tmp[5] = a[1] * b[4] + a[5] * b[5] + a[9]  * b[6] + a[13] * b[7];
	tmp[6] = a[2] * b[4] + a[6] * b[5] + a[10] * b[6] + a[14] * b[7];
	tmp[7] = a[3] * b[4] + a[7] * b[5] + a[11] * b[6] + a[15] * b[7];

	tmp[8]  = a[0] * b[8] + a[4] * b[9] + a[8]  * b[10] + a[12] * b[11];
	tmp[9]  = a[1] * b[8] + a[5] * b[9] + a[9]  * b[10] + a[13] * b[11];
	tmp[10] = a[2] * b[8] + a[6] * b[9] + a[10] * b[10] + a[14] * b[11];
	tmp[11] = a[3] * b[8] + a[7] * b[9] + a[11] * b[10] + a[15] * b[11];

	tmp[12] = a[0] * b[12] + a[4] * b[13] + a[8]  * b[14] + a[12] * b[15];
	tmp[13] = a[1] * b[12] + a[5] * b[13] + a[9]  * b[14] + a[13] * b[15];
	tmp[14] = a[2] * b[12] + a[6] * b[13] + a[10] * b[14] + a[14] * b[15];
	tmp[15] = a[3] * b[12] + a[7] * b[13] + a[11] * b[14] + a[15] * b[15];

	memcpy(out, tmp, sizeof(GLfloat) * 16);

	return out;
}

static GLfloat *
ident(GLfloat *out)
{
	GLfloat tmp[16] = {
		1.0f, 0.0f, 0.0f, 0.0f, 
		0.0f, 1.0f, 0.0f, 0.0f, 
		0.0f, 0.0f, 1.0f, 0.0f, 
		0.0f, 0.0f, 0.0f, 1.0f
	};

	memcpy(out, tmp, sizeof(GLfloat) * 16);

	return out;
}

static GLfloat *
translate(const GLfloat *m, const GLfloat *pos, GLfloat *out)
{
	memcpy(out, m, sizeof(GLfloat) * 16);
	out[12] += pos[0];
	out[13] += pos[1];
	out[14] += pos[2];

	return out;
}

static GLfloat *
rotate(const GLfloat *m, const GLfloat *axis, float deg, GLfloat *out)
{
	float rad = deg * (float)M_PI / 180.0f;

	GLfloat tmp[16] = {
		axis[0] * axis[0] * (1.0f - cosf(rad)) + cosf(rad),
		axis[0] * axis[1] * (1.0f - cosf(rad)) - axis[2] * sinf(rad),
		axis[0] * axis[2] * (1.0f - cosf(rad)) + axis[1] * sinf(rad),
		0.0f,
		axis[1] * axis[0] * (1.0f - cosf(rad)) + axis[2] * sinf(rad),
		axis[1] * axis[1] * (1.0f - cosf(rad)) + cosf(rad),
		axis[1] * axis[2] * (1.0f - cosf(rad)) - axis[0] * sinf(rad),
		0.0f,
		axis[2] * axis[0] * (1.0f - cosf(rad)) - axis[1] * sinf(rad),
		axis[2] * axis[1] * (1.0f - cosf(rad)) + axis[0] * sinf(rad),
		axis[2] * axis[2] * (1.0f - cosf(rad)) + cosf(rad),
		0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	return mmul(m, tmp, out);
}

static GLfloat *
scale(const GLfloat *m, const GLfloat *scl, GLfloat *out)
{
	GLfloat tmp[16] = {
		scl[0], 0.0f, 0.0f, 0.0f,
		0.0f, scl[1], 0.0f, 0.0f,
		0.0f, 0.0f, scl[2], 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	return mmul(m, tmp, out);
}

static GLfloat *
make_model(struct ctx *prog_ctx, GLfloat *model, const GLfloat *pos, const GLfloat *rot, const GLfloat *scl)
{
	const GLfloat x_axis[] = { 1.0f, 0.0f, 0.0f };
	const GLfloat y_axis[] = { 0.0f, 1.0f, 0.0f };
	const GLfloat z_axis[] = { 0.0f, 0.0f, 1.0f };

	GLfloat tmp[16];

	ident(tmp);

	rotate(tmp, x_axis, rot[0], tmp);
	rotate(tmp, y_axis, rot[1], tmp);
	rotate(tmp, z_axis, rot[2], tmp);

	scale(tmp, scl, tmp);

	translate(tmp, pos, model);

	return model;
}

static GLfloat *
make_view(struct ctx *prog_ctx, GLfloat *view, const GLfloat *pos)
{
	memset(view, 0, sizeof(GLfloat) * 16);
	view[0] = 1.0f;
	view[5] = 1.0f;
	view[10] = 1.0f;
	view[12] = -pos[0];
	view[13] = -pos[1];
	view[14] = -pos[2];
	view[15] = 1.0f;

	return view;
}

static GLfloat *
make_projection(struct ctx *prog_ctx, GLfloat *proj)
{
	float aspect = (float)viewport_width / (float)viewport_height;
	float fov = 60.0f * (float)M_PI / 180.0f;
	float near = 0.1f;
	float far = 100.0f;

	float tan_half_fov = tanf(fov / 2.0f);

	memset(proj, 0, sizeof(GLfloat) * 16);
	proj[0] = 1.0f / (aspect * tan_half_fov);
	proj[5] = 1.0f / tan_half_fov;
	proj[10] = -(far + near) / (far - near);
	proj[11] = -1.0f;
	proj[14] = -(2.0f * far * near) / (far - near);

	return proj;
}

static void
draw_cube(struct ctx *prog_ctx, const GLfloat *pos, const GLfloat *rot, const GLfloat *scl)
{
	GLfloat model[16];
	make_model(prog_ctx, model, pos, rot, scl);
	glUniformMatrix4fv(prog_ctx->programs.model_unif, 1, GL_FALSE, model);

	glBindVertexArray(prog_ctx->objects.cube_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prog_ctx->objects.cube_ebo);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

static void
make_vec(GLfloat *const out, GLfloat x, GLfloat y, GLfloat z)
{
	out[0] = x;
	out[1] = y;
	out[2] = z;
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
	GLfloat view[16];
	GLfloat proj[16];
	GLfloat camera_pos[3] = { 0.0f, 0.0f, 7.0f };
	GLfloat cube_pos[3];
	GLfloat cube_rot[3];
	GLfloat cube_scl[3];
	float rot_offset;

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
	args.allow_sampling = 0;
	args.sample_count = 1;

	/* create a window */
	prog_ctx.win = mtgl_win_create("OpenGL Window", viewport_width, viewport_height, 0, 0, &prog_ctx);
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

	//glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

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
	
		//r = sinf(mtgl_get_time(prog_ctx.win)) * 0.5f + 0.5f;
		//g = sinf(mtgl_get_time(prog_ctx.win) + one_third_2pi) * 0.5f + 0.5f;
		//b = sinf(mtgl_get_time(prog_ctx.win) + 2.0f * one_third_2pi) * 0.5f + 0.5f;

		r = 0.2f;
		g = 0.2f;
		b = 0.4f;


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
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			if (prog_ctx.objects.cube_vao && prog_ctx.programs.screen)
			{
				glUseProgram(prog_ctx.programs.screen);

				glUniform1f(prog_ctx.programs.time_unif, mtgl_get_time(prog_ctx.win));
				glUniform3f(prog_ctx.programs.cam_pos_unif, camera_pos[0], camera_pos[1], camera_pos[2]);
				glUniform3f(prog_ctx.programs.ambient_color_unif, r, g, b);
				
				make_view(&prog_ctx, view, camera_pos);
				make_projection(&prog_ctx, proj);

				glUniformMatrix4fv(prog_ctx.programs.view_unif, 1, GL_FALSE, view);
				glUniformMatrix4fv(prog_ctx.programs.proj_unif, 1, GL_FALSE, proj);

				rot_offset = mtgl_get_time(prog_ctx.win) * 5.0f;

				make_vec(cube_pos, -2, -0.5, 2);
				make_vec(cube_rot, 45 + rot_offset, 0 + rot_offset, 0 + rot_offset);
				make_vec(cube_scl, 1, 1, 1);
				glUniform3f(prog_ctx.programs.color_unif, 0.5f, 0.2f, 0.4f);
				draw_cube(&prog_ctx, cube_pos, cube_rot, cube_scl);

				make_vec(cube_pos, 2, 1, 1);
				make_vec(cube_rot, 15 + rot_offset, 0 + rot_offset, 45 + rot_offset);
				make_vec(cube_scl, 1, 1, 1);
				glUniform3f(prog_ctx.programs.color_unif, 0.1f, 0.7f, 0.3f);
				draw_cube(&prog_ctx, cube_pos, cube_rot, cube_scl);

				make_vec(cube_pos, -1, 0, -3);
				make_vec(cube_rot, 0 + rot_offset, 30 + rot_offset, 15 + rot_offset);
				make_vec(cube_scl, 1, 1, 1);
				glUniform3f(prog_ctx.programs.color_unif, 0.3f, 0.4f, 0.8f);
				draw_cube(&prog_ctx, cube_pos, cube_rot, cube_scl);

				make_vec(cube_pos, 0, -2, 0);
				make_vec(cube_rot, 0, 0, 0);
				make_vec(cube_scl, 10, 1, 10);
				glUniform3f(prog_ctx.programs.color_unif, 0.2f, 0.2f, 0.2f);
				draw_cube(&prog_ctx, cube_pos, cube_rot, cube_scl);
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

	glDeleteBuffers(1, &prog_ctx.objects.cube_ebo);
	glDeleteBuffers(1, &prog_ctx.objects.cube_vbo);
	glDeleteVertexArrays(1, &prog_ctx.objects.cube_vao);

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

	mtgl_ctx_acquire(ctx);

	printf("Loading shaders on another thread\n");
	load_shaders(prog_ctx);
	prog_ctx->programs.time_unif = glGetUniformLocation(prog_ctx->programs.screen, "uTime");
	prog_ctx->programs.model_unif = glGetUniformLocation(prog_ctx->programs.screen, "uModel");
	prog_ctx->programs.view_unif = glGetUniformLocation(prog_ctx->programs.screen, "uView");
	prog_ctx->programs.proj_unif = glGetUniformLocation(prog_ctx->programs.screen, "uProj");
	prog_ctx->programs.cam_pos_unif = glGetUniformLocation(prog_ctx->programs.screen, "uCameraPos");
	prog_ctx->programs.ambient_color_unif = glGetUniformLocation(prog_ctx->programs.screen, "uAmbientColor");
	prog_ctx->programs.color_unif = glGetUniformLocation(prog_ctx->programs.screen, "uColor");
	printf("Done loading\n");

	printf("Loading cube on another thread!\n");

	/* create the cube */
	glGenVertexArrays(1, &prog_ctx->objects.cube_vao);
	glGenBuffers(1, &prog_ctx->objects.cube_vbo);
	glGenBuffers(1, &prog_ctx->objects.cube_ebo);

	glBindVertexArray(prog_ctx->objects.cube_vao);

	glBindBuffer(GL_ARRAY_BUFFER, prog_ctx->objects.cube_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prog_ctx->objects.cube_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void *)(0 * sizeof(GLfloat)));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void *)(3 * sizeof(GLfloat)));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	printf("Cube done loading!\n");

	mtgl_ctx_release(ctx);

	return 0;
}
