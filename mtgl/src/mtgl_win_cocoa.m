#if __APPLE__
#include "mtgl_win_cocoa.h"

#include <stdio.h>

static int
mtgl_cocoa_key_to_mtgl(int key) { return 0; }

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

- (BOOL)windowShouldClose:(id)sender
{
    return NO_ADDRESS;
}

- (void)windowDidResize:(NSNotification *)notification
{
    // TODO: this
}

- (void)windowDidMove:(NSNotification *)notification
{
    // TODO: this
}

- (void)windowDidMiniaturize:(NSNotification *)notification
{
    // TODO: this
}

- (void)windowDidDeminiaturize:(NSNotification *)notification
{
    // TODO: this
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    // TODO: this
}

- (void)windowDidResignKey:(NSNotification *)notification
{
    // TODO: this
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
        [self addTrackingArea:tracking_area];
        text = [[NSMutableAttributedString alloc] init];

        [self updateTrackingAreas];
        [self registerForDraggedTypes:@[NSPasteboardTypeString]];
    }
    return self;
}

- (void)dealloc
{
    [text release];
    [tracking_area release];
    [super dealloc];
}

- (BOOL)isOpaque
{
    return [(NSWindow *)win->window isOpaque];
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

- (void)cursorUpdate:(NSEvent *)event
{

}

- (BOOL)acceptsFirstMouse:(NSEvent *)event
{
    return YES;
}

- (void)mouseDragged:(NSEvent *)event
{

}

- (void)mouseDown:(NSEvent *)event
{

}

- (void)mouseUp:(NSEvent *)event
{

}

- (void)mouseMoved:(NSEvent *)event
{

}

- (void)mouseRightDown:(NSEvent *)event
{

}

- (void)mouseRightDragged:(NSEvent *)event
{

}

- (void)mouseRightUp:(NSEvent *)event
{

}

- (void)otherMouseDown:(NSEvent *)event
{

}

- (void)otherMouseDragged:(NSEvent *)event
{

}

- (void)otherMouseUp:(NSEvent *)event
{
    
}

- (void)mouseExited:(NSEvent *)event
{

}

- (void)mouseEntered:(NSEvent *)event
{

}

- (void)viewDidChangeBackingProperties
{

}

- (void)drawRect:(NSRect)rect
{

}

- (void)updateTrackingAreas
{

}

- (void)keyDown:(NSEvent *)event
{
    int key;
    key = mtgl_cocoa_key_to_mtgl([event keyCode]);
    win->win.key_states[key] = mtgl_pressed;
}

- (void)keyUp:(NSEvent *)event
{
    int key;
    key = mtgl_cocoa_key_to_mtgl([event keyCode]);
    win->win.key_states[key] = mtgl_released;
}

- (void)flagsChanged:(NSEvent *)event
{

}

- (void)scrollWheel:(NSEvent *)event
{
    short scroll;
    scroll = [event deltaY];
    win->win.wheel += scroll;
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender
{
    return NSDragOperationNone;
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
    return NO;
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
    return NSZeroRect;
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

    win->delegate = [[MTGLWindowDelegate alloc] initWithWindow:win];
    if (!win->delegate) return 0;

    printf("delegate: %p\n", win->delegate);

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

    printf("window: %p\n", win->window);

    [(NSWindow *)win->window center];

    win->view = [[MTGLView alloc] initWithWindow:win];
    if (!win->view) return 0;

    printf("view: %p\n", win->view);

    [(NSWindow *)win->window setContentView:win->view];
    [(NSWindow *)win->window setDelegate:win->delegate];
    [(NSWindow *)win->window makeFirstResponder:win->view];
    [(NSWindow *)win->window setTitle:@"MTGL"];
    [(NSWindow *)win->window setAcceptsMouseMovedEvents:YES];
    [(NSWindow *)win->window setRestorable:NO];

    return 1;
}

struct mtglwin_cocoa *
mtgl_win_create_cocoa(const char *title, int width, int height, int flags, int device, void *user_data)
{
    struct mtglwin_cocoa *winc = calloc(1, sizeof(struct mtglwin_cocoa));

    @autoreleasepool
    {
        if (!mtgl_init_cocoa_window(winc, width, height, flags, device))
        {
            free(winc);
            return 0;
        }
    }

    return winc;
}

void
mtgl_set_title_cocoa(struct mtglwin_cocoa *win, const char *title)
{
    NSString *string;
    @autoreleasepool
    {
        string = [NSString stringWithUTF8String:title];
        [(NSWindow *)win->window setTitle:string];
        [(NSWindow *)win->window setMiniwindowTitle:string];
    }
}

void
mtgl_show_window_cocoa(struct mtglwin_cocoa *win, int shown)
{
    @autoreleasepool
    {
        if (shown)
            [(NSWindow *)win->window makeKeyAndOrderFront:nil];
        else
            [(NSWindow *)win->window orderOut:nil];
    }
}

void
mtgl_poll_events_cocoa(struct mtglwin_cocoa *win)
{
    @autoreleasepool
    {
        NSEvent *event;
        NSRect rect;

        mtgl_lock_acquire(win->win.lock);

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
            rect = [(NSWindow *)win->window contentRectForFrameRect:[(NSWindow *)win->window frame]];
            win->win.width = rect.size.width;
            win->win.height = rect.size.height;
        }

        mtgl_dispatch_events(&win->win);

        mtgl_lock_release(win->win.lock);
    }
}

void
mtgl_swap_buffers_cocoa(struct mtglwin_cocoa *win)
{

}

void
mtgl_get_full_size_cocoa(struct mtglwin_cocoa *win, int *width, int *height)
{
    @autoreleasepool
    {
        NSRect rect = [(NSWindow *)win->window contentRectForFrameRect:[(NSWindow *)win->window frame]];
        *width = rect.size.width;
        *height = rect.size.height;
    }
}

void
mtgl_set_size_cocoa(struct mtglwin_cocoa *win, int width, int height)
{
    @autoreleasepool
    {
        NSRect rect = [(NSWindow *)win->window contentRectForFrameRect:[(NSWindow *)win->window frame]];
        rect.origin.y += rect.size.height - height;
        rect.size = NSMakeSize(width, height);
        [(NSWindow *)win->window setFrame:[(NSWindow *)win->window frameRectForContentRect:rect] display:YES];
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
    @autoreleasepool
    {
        [(NSWindow *)win->window release];
        [(id)win->view release];
        [(id)win->delegate release];
    }

    free(win);
}

#endif