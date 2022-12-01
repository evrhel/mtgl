#pragma once

typedef struct glwin glwin;
typedef struct glctx glctx;
typedef struct glthread glthread;
typedef struct gllock gllock;

typedef int(*glthread_fn)(void *);

int mtgl_init();
gllock *mtgl_get_lock();
void mtgl_done();

glwin *glwin_create(int width, int height);
void glwin_show_window(glwin *win, int shown);
int glwin_should_close(glwin *win);
void glwin_poll_events(glwin *win);
void glwin_swap_buffers(glwin *win);
int glwin_was_resized(glwin *win);
void glwin_get_size(glwin *win, int *const width, int *const height);
float glwin_get_time(glwin *win);
void glwin_destroy(glwin *win);

glctx *glctx_create(glwin *win, int ver_major, int ver_minor);
glctx *glctx_clone(glctx *ctx);
void glctx_acquire(glctx *ctx);
void glctx_release(glctx *ctx);
void glctx_set_swap_interval(glctx *ctx, int interval);
void glctx_destroy(glctx *ctx);
void *glctx_get_proc(const char *name);

glthread *glthread_create(glctx *ctx, glthread_fn entry, void *param1);
void glthread_detach(glthread *thread);
int glthread_join(glthread *thread);

gllock *gllock_create();
void gllock_acquire(gllock *lock);
void gllock_release(gllock *lock);
void gllock_destroy(gllock *lock);