#include "mtgl_ctx_cocoa.h"
#include "mtgl_win_cocoa.h"

#include <Cocoa/Cocoa.h>
#include <Opengl/Opengl.h>
#include <stdlib.h>

#include <assert.h>

#include <stdio.h>

static CFBundleRef nsgl_framework = 0;

/* convert major and minor version to profile enum */
static int
mtgl_get_cocoa_gl_version(int ver_major, int ver_minor)
{
    int ver = 0;

    if (ver_major == 4)
    {
        if (ver_minor == 1)
            ver = NSOpenGLProfileVersion4_1Core;
        else if (ver_minor == 0)
            ver = NSOpenGLProfileVersion4_1Core;
    }
    else if (ver_major == 3)
    {
        if (ver_minor == 2)
            ver = NSOpenGLProfileVersion3_2Core;
        else if (ver_minor == 1)
            ver = NSOpenGLProfileVersion3_2Core;
        else if (ver_minor == 0)
            ver = NSOpenGLProfileVersionLegacy;
    }

    return ver;
}

struct mtglctx_cocoa *mtgl_ctx_create_cocoa(struct mtglwin_cocoa *win, int ver_major, int ver_minor, mtglctxinitargs *args)
{
    int version;
    int attr_idx = 0;
    int color_bits;
    int alpha_bits;
    int accum_bits;
    int depth_bits;
    int stencil_bits;
    NSOpenGLPixelFormatAttribute attrs[64] = { 0 };
    id pixel_format;
    struct mtglctx_cocoa *ctx;

    printf("createingcontext\n");

    @autoreleasepool
    {
        mtgl_lock_acquire(win->win.lock);

        /* check if main context already exists */
        if (win->win.main)
        {
            mtgl_lock_release(win->win.lock);
            return 0;
        }

        /* load opengl framework */
        if (nsgl_framework)
        {
            nsgl_framework = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.opengl"));
            if (!nsgl_framework)
            {
                mtgl_lock_release(win->win.lock);
                return 0;
            }
        }

        printf("framework loaded\n");

        version = mtgl_get_cocoa_gl_version(ver_major, ver_minor);
        if (!version)
        {
            mtgl_lock_release(win->win.lock);
            return 0;
        }

        attrs[attr_idx++] = NSOpenGLPFAAccelerated;
        attrs[attr_idx++] = NSOpenGLPFAClosestPolicy;

        /* set version */ 
        attrs[attr_idx++] = NSOpenGLPFAOpenGLProfile;
        attrs[attr_idx++] = version;

        /* set double buffer */
        if (args->double_buffer)
            attrs[attr_idx++] = NSOpenGLPFADoubleBuffer;

        /* set color bits */
        color_bits = args->red_bits + args->green_bits + args->blue_bits;
        if (color_bits)
        {
            attrs[attr_idx++] = NSOpenGLPFAColorSize;
            attrs[attr_idx++] = color_bits;
        }

        /* set alpha bits */    
        if (args->alpha_bits)
        {
            attrs[attr_idx++] = NSOpenGLPFAAlphaSize;
            attrs[attr_idx++] = args->alpha_bits;
        }

        /* set accum bits */
        accum_bits = args->accum_red_bits + args->accum_green_bits + args->accum_blue_bits + args->accum_alpha_bits;
        if (accum_bits)
        {
            attrs[attr_idx++] = NSOpenGLPFAAccumSize;
            attrs[attr_idx++] = accum_bits;
        }

        /* set depth bits */
        if (args->depth_bits)
        {
            attrs[attr_idx++] = NSOpenGLPFADepthSize;
            attrs[attr_idx++] = args->depth_bits;
        }

        /* set stencil bits */
        if (args->stencil_bits)
        {
            attrs[attr_idx++] = NSOpenGLPFAStencilSize;
            attrs[attr_idx++] = args->stencil_bits;
        }

        /* set multisample */
        if (args->allow_sampling && args->sample_count)
        {
            attrs[attr_idx++] = NSOpenGLPFASampleBuffers;
            attrs[attr_idx++] = 1;
            attrs[attr_idx++] = NSOpenGLPFASamples;
            attrs[attr_idx++] = args->sample_count;
        }

        attrs[attr_idx++] = 0;

        printf("attributes set\n");

        /* choose pixel format */
        pixel_format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
        if (!pixel_format)
        {
            mtgl_lock_release(win->win.lock);
            return 0;
        }

        printf("pixel format created\n");

        ctx = calloc(1, sizeof(struct mtglctx_cocoa));
        if (!ctx)
        {
            mtgl_lock_release(win->win.lock);
            return 0;
        }

        printf("context wrapper created\n");

        ctx->ctx.lock = mtgl_lock_create();
        if (!ctx->ctx.lock)
        {
            free(ctx);
            return 0;
        }

        printf("lock created\n");

        /* create context */
        ctx->context = [[NSOpenGLContext alloc]
            initWithFormat:pixel_format shareContext:nil];
        if (!ctx->context)
        {
            free(ctx);
            mtgl_lock_release(win->win.lock);
            return 0;
        }

        printf("opengl context created\n");

        ctx->pixel_format = pixel_format;
        ctx->ctx.type = ctxtype_root;
        ctx->ctx.win = &win->win;
        ctx->ctx.ver_major = ver_major;
        ctx->ctx.ver_minor = ver_minor;
        ctx->ctx.profile = version == NSOpenGLProfileVersionLegacy ?
                    mtgl_profile_compatability : mtgl_profile_core;
        ctx->ctx.nesting = 0;

        /* set main context */
        ctx->ctx.win->main = &ctx->ctx;

        [(id)win->view setWantsBestResolutionOpenGLSurface:YES];

        mtgl_lock_release(win->win.lock);

        printf("finished!\n");
        return ctx;
    }
}

struct mtglctx_cocoa *mtgl_ctx_clone_cocoa(struct mtglwin_cocoa *ctx)
{
    // TODO
    return 0;
}

void mtgl_ctx_acquire_cocoa(struct mtglctx_cocoa *ctx)
{
    mtgl_lock_acquire(ctx->ctx.lock);
    ctx->ctx.nesting++;
    if (ctx->ctx.nesting == 1)
    {
        @autoreleasepool
        {
            [(id)ctx->context makeCurrentContext];
        }
    }
}

int mtgl_ctx_try_acquire_cocoa(struct mtglctx_cocoa *ctx)
{
    if (!mtgl_lock_try_acquire(ctx->ctx.lock))
        return 0;

    ctx->ctx.nesting++;
    if (ctx->ctx.nesting == 1)
    {
        @autoreleasepool
        {
            [(id)ctx->context makeCurrentContext];
        }
    }

    return 1;
}

void mtgl_ctx_release_cocoa(struct mtglctx_cocoa *ctx)
{
    ctx->ctx.nesting--;
    if (ctx->ctx.nesting == 0)
    {
        @autoreleasepool
        {
            [(id)ctx->context clearCurrentContext];
        }
    }
    mtgl_lock_release(ctx->ctx.lock);
}

void mtgl_ctx_set_swap_interval_cocoa(struct mtglctx_cocoa *ctx, int interval)
{
    mtgl_lock_acquire(ctx->ctx.lock);
    @autoreleasepool
    {
        [(id)ctx->context setValues:&interval
            forParameter:NSOpenGLCPSwapInterval];
    }
    mtgl_lock_release(ctx->ctx.lock);
}

void mtgl_ctx_destroy_cocoa(struct mtglctx_cocoa *ctx)
{
    struct mtglwin_cocoa *win;

    assert(ctx->ctx.nesting == 0);
    
    win = (struct mtglwin_cocoa *)&ctx->ctx.win;
    mtgl_lock_acquire(win->win.lock);

    /* remove from window */
    if (ctx->ctx.type == ctxtype_root)
        win->win.main = 0;

    @autoreleasepool
    {
        mtgl_lock_destroy(ctx->ctx.lock);
        [(id)ctx->context release];
        [(id)ctx->pixel_format release];
        free(ctx);
    }

    mtgl_lock_release(win->win.lock);
}

void *mtgl_ctx_get_proc_cocoa(const char *name)
{
    CFStringRef symbol_name;
    void *proc;

    symbol_name = CFStringCreateWithCString(
                    NULL,
                    name,
                    kCFStringEncodingASCII
                );

    proc = CFBundleGetFunctionPointerForName(
        nsgl_framework,
        symbol_name
    );

    CFRelease(symbol_name);

    return proc;
}