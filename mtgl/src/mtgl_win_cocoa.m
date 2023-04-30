#if __APPLE__
#include "mtgl_win_cocoa.h"
#include "mtgl_ctx_cocoa.h"

#include <Cocoa/Cocoa.h>

static int
mtgl_cocoa_key_to_mtgl(int key) { return 0; }

static int
mtgl_cocoa_flags_to_mods_mtgl(int flags) { return 0; }

@interface MTGLWindowDelegate : NSObject
{
    struct mtglwin_cocoa *win;
}

- (instancetype)initWithWindow:(struct mtglwin_cocoa *)winc;

@end

@implementation MTGLWindowDelegate

- (instancetype)initWithWindow:(struct mtglwin_cocoa *)winc
{
    self = [super init];
    if (self)
        win = winc;
    return self;
}

/* window closing */
- (BOOL)windowShouldClose:(id)sender
{
    struct event event;

    win->win.should_close = 1;

    event.type = mtgl_event_window_event;
    event.data.window_event.event = mtgl_window_closing;
    event.data.window_event.param1 = 0;
    event.data.window_event.param2 = 0;
    mtgl_push_event(&win->win, &event);

    return NO; // closing controlled by user
}

/* window resized */ 
- (void)windowDidResize:(NSNotification *)notification
{
    NSRect content, fb;
    struct mtglctx_cocoa *ctx = (struct mtglctx_cocoa *)win->win.main;
    if (ctx)
        [(NSOpenGLContext *)ctx->context update];

    content = [(id)win->view frame];
    fb = [(id)win->view convertRectToBacking:content];

    win->win.width = fb.size.width;
    win->win.height = fb.size.height;

    win->win.was_resized = 1;
}

/* window moved */
- (void)windowDidMove:(NSNotification *)notification
{
    struct event event;
    struct mtglctx_cocoa *ctx = (struct mtglctx_cocoa *)win->win.main;
    NSPoint point = [(id)win->window frame].origin;

    if (ctx)
        [(NSOpenGLContext *)ctx->context update];

    event.type = mtgl_event_window_event;
    event.data.window_event.event = mtgl_window_move;
    event.data.window_event.param1 = point.x;
    event.data.window_event.param2 = point.y;
    mtgl_push_event(&win->win, &event);
}

/* window minimized */
- (void)windowDidMiniaturize:(NSNotification *)notification
{
    struct event event;
    NSRect rect = [(NSView *)win->view frame];

    event.type = mtgl_event_window_event;
    event.data.window_event.event = mtgl_window_minimize;
    event.data.window_event.param1 = rect.size.width;
    event.data.window_event.param2 = rect.size.height;
    mtgl_push_event(&win->win, &event);
}

/* window restored */
- (void)windowDidDeminiaturize:(NSNotification *)notification
{
    struct event event;
    NSRect rect = [(id)win->window frame];

    event.type = mtgl_event_window_event;
    event.data.window_event.event = mtgl_window_restore;
    event.data.window_event.param1 = rect.size.width;
    event.data.window_event.param2 = rect.size.height;
    mtgl_push_event(&win->win, &event);
}

/* window focused */
- (void)windowDidBecomeKey:(NSNotification *)notification
{
    struct event event;

    event.type = mtgl_event_window_event;
    event.data.window_event.event = mtgl_window_changefocus;
    event.data.window_event.param1 = 1;
    event.data.window_event.param2 = 0;
    mtgl_push_event(&win->win, &event);
}

/* window unfocused */
- (void)windowDidResignKey:(NSNotification *)notification
{
    struct event event;

    event.type = mtgl_event_window_event;
    event.data.window_event.event = mtgl_window_changefocus;
    event.data.window_event.param1 = 0;
    event.data.window_event.param2 = 0;
    mtgl_push_event(&win->win, &event);
}

@end

@interface MTGLView : NSView<NSTextInputClient>
{
    struct mtglwin_cocoa *win;
    NSTrackingArea *tracking_area;
    NSMutableAttributedString *text;
}

- (instancetype)initWithWindow:(struct mtglwin_cocoa *)winc;

@end

@implementation MTGLView

- (instancetype)initWithWindow:(struct mtglwin_cocoa *)winc
{
    self = [super init];
    if (self)
    {
        win = winc;
        tracking_area = nil;
        text = [[NSMutableAttributedString alloc] init];

        [self updateTrackingAreas];
        [self registerForDraggedTypes:@[NSPasteboardTypeString]];
    }
    return self;
}

- (void)dealloc
{
    [tracking_area release];
    [text release];
    [super dealloc];
}

- (BOOL)isOpaque
{
    return [(id)win->window isOpaque];
}

- (BOOL)canBecomeKeyView
{
    return YES;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (BOOL)wantsUpdateLayer
{
    return YES;
}

- (void)updateLayer
{
    struct mtglctx_cocoa *ctx = (struct mtglctx_cocoa *)win->win.main;
    if (ctx)
        [(NSOpenGLContext *)ctx->context update];
}

- (void)cursorUpdate:(NSEvent *)event
{

}

- (BOOL)acceptsFirstMouse:(NSEvent *)event
{
    return YES;
}

- (void)mouseMoved:(NSEvent *)event
{
    win->win.dmx += [event deltaX];
    win->win.dmy += [event deltaY];
}

- (void)mouseDown:(NSEvent *)event
{
    struct event evt;
    mtgl_push_button_event(&win->win, &evt, mtgl_pressed, mtgl_mouse1);
}

- (void)mouseUp:(NSEvent *)event
{
    struct event evt;
    mtgl_push_button_event(&win->win, &evt, mtgl_released, mtgl_mouse1);
}

- (void)mouseDragged:(NSEvent *)event
{
    [self mouseMoved: event];
}

- (void)mouseRightDown:(NSEvent *)event
{
    struct event evt;
    mtgl_push_button_event(&win->win, &evt, mtgl_pressed, mtgl_mouse2);
}

- (void)mouseRightUp:(NSEvent *)event
{
    struct event evt;
    mtgl_push_button_event(&win->win, &evt, mtgl_released, mtgl_mouse2);
}

- (void)mouseRightDragged:(NSEvent *)event
{
    [self mouseMoved: event];
}

- (void)otherMouseDown:(NSEvent *)event
{
    struct event evt;
    mtgl_push_button_event(&win->win, &evt, mtgl_pressed, mtgl_mouse3);
}

- (void)otherMouseUp:(NSEvent *)event
{
    struct event evt;
    mtgl_push_button_event(&win->win, &evt, mtgl_released, mtgl_mouse3);
}

- (void)otherMouseDragged:(NSEvent *)event
{
    
}

- (void)mouseEntered:(NSEvent *)event
{

}

- (void)mouseExited:(NSEvent *)event
{
    
}

- (void)viewDidChangeBackingProperties
{
    NSRect content, fb;
    float scale_x, scale_y;

    content = [(id)win->view frame];
    fb = [(id)win->view convertRectToBacking:content];
    scale_x = fb.size.width / content.size.width;
    scale_y = fb.size.height / content.size.height;

    [(id)win->layer setContentsScale:[(id)win->window backingScaleFactor]];
}

- (void)drawRect:(NSRect)rect
{

}

- (void)updateTrackingAreas
{
    NSTrackingAreaOptions options = NSTrackingMouseEnteredAndExited |
                                    NSTrackingActiveInKeyWindow |
                                    NSTrackingEnabledDuringMouseDrag |
                                    NSTrackingCursorUpdate |
                                    NSTrackingInVisibleRect |
                                    NSTrackingAssumeInside;

    if (tracking_area)
    {
        [self removeTrackingArea:tracking_area];
        [tracking_area release];
    }

    tracking_area = [[NSTrackingArea alloc] initWithRect:[self bounds]
                                                 options:options
                                                   owner:self
                                                userInfo:nil];

    [self addTrackingArea:tracking_area];
    [super updateTrackingAreas];
}

- (void)keyDown:(NSEvent *)event
{
    struct event evt;
    int key = mtgl_cocoa_key_to_mtgl([event keyCode]);

    evt.type = mtgl_event_key;
    evt.data.key.key = key;
    evt.data.key.mods = win->win.mods;
    evt.data.key.action = win->win.key_states[key] = mtgl_pressed;
    mtgl_push_event(&win->win, &evt);

    [self interpretKeyEvents:@[event]];
}

- (void)keyUp:(NSEvent *)event
{
    struct event evt;
    int key = mtgl_cocoa_key_to_mtgl([event keyCode]);

    evt.type = mtgl_event_key;
    evt.data.key.key = key;
    evt.data.key.mods = win->win.mods;
    evt.data.key.action = win->win.key_states[key] = mtgl_released;
    mtgl_push_event(&win->win, &evt);
}

- (void)flagsChanged:(NSEvent *)event
{
    win->win.flags = mtgl_cocoa_flags_to_mods_mtgl([event modifierFlags]);
}

- (void)scrollWheel:(NSEvent *)event
{
    double scroll;
    scroll = [event deltaY];

    if ([event hasPreciseScrollingDeltas])
        scroll *= 0.1;

    win->win.wheel += scroll;
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender
{
    return NSDragOperationGeneric;
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
    return YES;
}

- (BOOL)hasMarkedText
{
    return [text length] > 0;
}

- (NSRange)markedRange
{
    if ([text length] > 0)
        return NSMakeRange(0, [text length] - 1);
    return NSMakeRange(NSNotFound, 0);
}

- (NSRange)selectedRange
{
    return NSMakeRange(NSNotFound, 0);
}

- (void)setMarkedText:(id)string selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange
{
    [text release];
    if ([string isKindOfClass:[NSAttributedString class]])
        text = [[NSMutableAttributedString alloc] initWithAttributedString:string];
    else
        text = [[NSMutableAttributedString alloc] initWithString:string];
}

- (void)unmarkText
{
    [[text mutableString] setString:@""];
}

- (NSArray *)validAttributesForMarkedText
{
    return [NSArray array];
}

- (NSAttributedString *)attributedSubstringForProposedRange:(NSRange)range actualRange:(NSRangePointer)actualRange
{
    return nil;
}

- (NSUInteger)characterIndexForPoint:(NSPoint)point
{
    return 0;
}

- (NSRect)firstRectForCharacterRange:(NSRange)range actualRange:(NSRangePointer)actualRange
{
    NSRect frame = [(MTGLView *)win->view frame];
    return NSMakeRect(frame.origin.x, frame.origin.y, 0, 0);
}

- (void)insertText:(id)string replacementRange:(NSRange)replacementRange
{
    [text replaceCharactersInRange:replacementRange withString:string];
}

- (void)doCommandBySelector:(SEL)selector
{

}

@end

@interface MTGLWindow: NSWindow { }
@end

@implementation MTGLWindow

- (BOOL)canBecomeKeyWindow
{
    return YES;
}

- (BOOL)canBecomeMainWindow
{
    return YES;
}

@end

static int
mtgl_init_cocoa_window(struct mtglwin_cocoa *win, int width, int height, int flags, int device)
{
    NSRect rect;
    NSUInteger style;
    NSWindowCollectionBehavior behavior;

    @autoreleasepool
    {
        win->delegate = [[MTGLWindowDelegate alloc] initWithWindow:win];
        if (!win->delegate) return 0;

        rect = NSMakeRect(0, 0, width, height);

        style = NSWindowStyleMaskMiniaturizable |
                NSWindowStyleMaskTitled |
                NSWindowStyleMaskClosable |
                NSWindowStyleMaskResizable;

        win->window = [[MTGLWindow alloc]
            initWithContentRect:rect
            styleMask:style
            backing:NSBackingStoreBuffered
            defer:NO];

        if (!win->window) return 0;

        [(MTGLWindow *)win->window center];

        behavior = NSWindowCollectionBehaviorFullScreenPrimary |
                NSWindowCollectionBehaviorManaged;
        [(MTGLWindow *)win->window setCollectionBehavior:behavior];

        [(MTGLWindow *)win->window setLevel:NSFloatingWindowLevel];
        [(MTGLWindow *)win->window setFrameAutosaveName:@"MTGL"];

        win->view = [[MTGLView alloc] initWithWindow:win];
        if (!win->view) return 0;

        [(MTGLWindow *)win->window setContentView:win->view];
        [(MTGLWindow *)win->window makeFirstResponder:win->view];
        [(MTGLWindow *)win->window setTitle:@"MTGL"];
        [(MTGLWindow *)win->window setDelegate:win->delegate];
        [(MTGLWindow *)win->window setAcceptsMouseMovedEvents:YES];
        [(MTGLWindow *)win->window setRestorable:NO];

        if ([(MTGLWindow *)win->window respondsToSelector:@selector(setTabbingMode)])
            [(MTGLWindow *)win->window setTabbingMode:NSWindowTabbingModeDisallowed];

        return 1;
    }
}

struct mtglwin_cocoa *
mtgl_win_create_cocoa(const char *title, int width, int height, int flags, int device, void *user_data)
{
    mtglwin *win;
    struct mtglwin_cocoa *winc;
    NSRect rect;

    mtgl_lock_acquire(mtgl_get_lock());
    
    winc = calloc(1, sizeof(struct mtglwin_cocoa));
    if (!winc)
    { 
        mtgl_lock_release(mtgl_get_lock());
        return 0;
    }
    win = &winc->win;

    /* create lock */
    win->lock = mtgl_lock_create();
    if (!win->lock) goto failure;

    /* initialize event queue */
    win->events = calloc(EVENT_QUEUE_SIZE, sizeof(struct event));
    if (!win->events) goto failure;

    /* initialize callbacks */
    win->callbacks = calloc(EVT_CB_COUNT, sizeof(union callback));
    if (!win->callbacks) goto failure;

    /* initialize joysticks */
    win->joysticks = calloc(JOY_COUNT, sizeof(struct joystick));
    if (!win->joysticks) goto failure;

    win->user_data = user_data;

    if (!mtgl_init_cocoa_window(winc, width, height, flags, device))
        goto failure;

    @autoreleasepool
    {
        rect = [(MTGLWindow *)winc->window frame];
        win->x = rect.origin.x;
        win->y = rect.origin.y;
        win->width = rect.size.width;
        win->height = rect.size.height;
    }

    mtgl_lock_release(mtgl_get_lock());

    return winc;

failure:

    if (win->lock) mtgl_lock_destroy(win->lock);
    if (win->events) free(win->events);
    if (win->callbacks) free(win->callbacks);
    if (win->joysticks) free(win->joysticks);

    free(winc);

    mtgl_lock_release(mtgl_get_lock());
    return 0;
}

void
mtgl_set_title_cocoa(struct mtglwin_cocoa *win, const char *title)
{
    NSString *string;
    @autoreleasepool
    {
        string = [NSString stringWithUTF8String:title];
        [(MTGLWindow *)win->window setTitle:string];
        [(MTGLWindow *)win->window setMiniwindowTitle:string];
    }
}

void
mtgl_show_window_cocoa(struct mtglwin_cocoa *win, int shown)
{
    @autoreleasepool
    {
        if (shown)
            [(MTGLWindow *)win->window makeKeyAndOrderFront:nil];
        else
            [(MTGLWindow *)win->window orderOut:nil];
    }
}

void
mtgl_poll_events_cocoa(struct mtglwin_cocoa *win)
{
    struct event evt;

    @autoreleasepool
    {
        NSEvent *event;
        NSRect rect;

        mtgl_lock_acquire(win->win.lock);

        win->win.was_resized = 0;

        win->win.dmx = 0;
        win->win.dmy = 0;
        win->win.wheel = 0;

        for (;;)
        {
            event = [NSApp nextEventMatchingMask:NSEventMaskAny
                        untilDate:[NSDate distantPast]
                        inMode:NSDefaultRunLoopMode
                        dequeue:YES];

            if (!event) break;

            [NSApp sendEvent:event];
        }

        if (win->win.was_resized)
        {
            evt.type = mtgl_event_resize;
            evt.win = &win->win;
            mtgl_push_event(&win->win, &evt);
        }

        mtgl_dispatch_events(&win->win);

        mtgl_lock_release(win->win.lock);
    }
}

void
mtgl_swap_buffers_cocoa(struct mtglwin_cocoa *win)
{
    struct mtglctx_cocoa *ctx = (struct mtglctx_cocoa *)win->win.main;
    int interval;

    @autoreleasepool
    {
        if (win->win.main)
        {
            // test if window occluded
            if ([(MTGLView *)win->view isHiddenOrHasHiddenAncestor])
            {
                interval = 0;
                [(NSOpenGLContext *)ctx->context getValues:&interval
                    forParameter:NSOpenGLCPSwapInterval];

                if (interval > 0)
                    usleep(1000);
            }

            [(NSOpenGLContext *)ctx->context flushBuffer];
        }
    }
}

void
mtgl_get_full_size_cocoa(struct mtglwin_cocoa *win, int *width, int *height)
{
    @autoreleasepool
    {
        NSRect rect = [(MTGLWindow *)win->window contentRectForFrameRect:[(MTGLWindow *)win->window frame]];
        *width = rect.size.width;
        *height = rect.size.height;
    }
}

void
mtgl_set_size_cocoa(struct mtglwin_cocoa *win, int width, int height)
{
    @autoreleasepool
    {
        NSRect rect = [(MTGLWindow *)win->window contentRectForFrameRect:[(MTGLWindow *)win->window frame]];
        rect.origin.y += rect.size.height - height;
        rect.size = NSMakeSize(width, height);
        [(MTGLWindow *)win->window setFrame:[(MTGLWindow *)win->window frameRectForContentRect:rect] display:YES];
    }
}

void
mtgl_set_pos_cocoa(struct mtglwin_cocoa *win, int x, int y)
{

}

float
mtgl_get_time_cocoa(struct mtglwin_cocoa *win)
{
    struct timespec current;
    float cur_sec;
    float start_sec;

    clock_gettime(CLOCK_MONOTONIC, &current);
    cur_sec = (float)current.tv_sec + (float)current.tv_nsec / 1000000000.0f;
    start_sec = (float)win->start.tv_sec + (float)win->start.tv_nsec / 1000000000.0f;

    return cur_sec - start_sec;
}

void
mtgl_win_destroy_cocoa(struct mtglwin_cocoa *win)
{
    mtgl_lock_acquire(mtgl_get_lock());

    free(win->win.joysticks);
    free(win->win.callbacks);
    free(win->win.events);

    mtgl_lock_destroy(win->win.lock);

    @autoreleasepool
    {
        [(MTGLWindow *)win->window setDelegate:nil];

        [(MTGLWindowDelegate *)win->delegate release];
        [(MTGLView *)win->view release];
        [(MTGLWindow *)win->window close];

        mtgl_poll_events_cocoa(win);
    }

    free(win);

    mtgl_lock_release(mtgl_get_lock());
}

#endif