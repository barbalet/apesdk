/****************************************************************
 
 NobleMacView.m
 
 =============================================================
 
 Copyright 1996-2017 Tom Barbalet. All rights reserved.
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or
 sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
 
 This software and Noble Ape are a continuing work of Tom Barbalet,
 begun on 13 June 1996. No apes or cats were harmed in the writing
 of this software.
 
 ****************************************************************/

#include "../noble/noble.h"

#ifdef NEW_OPENGL_ENVIRONMENT
#include "../ogl/ogl.h"
#endif

#import "NobleMacView.h"

#include <stdio.h>
#include <stdlib.h>

#pragma mark ---- OpenGL Utils ----

@interface NobleMacView()

@property (nonatomic, strong) NSTimer* timerAnimation;
@property (nonatomic, strong, readwrite) NobleShared* shared;
@property (nonatomic, strong) NSSound* timedAudio;

@end

@implementation NobleMacView

/* pixel format definition */
+ (NSOpenGLPixelFormat*) basicPixelFormat
{
    
#ifndef NEW_OPENGL_ENVIRONMENT
    NSOpenGLPixelFormatAttribute attributes [] = {
        NSOpenGLPFADoubleBuffer,	/* double buffered */
        NSOpenGLPFADepthSize,
        (NSOpenGLPixelFormatAttribute)16, /* 16 bit depth buffer */
        (NSOpenGLPixelFormatAttribute)0 /*nil*/
    };
#else
    NSOpenGLPixelFormatAttribute attributes [] = {
        NSOpenGLPFANoRecovery,
        NSOpenGLPFAColorSize, 32,
        NSOpenGLPFADepthSize, 32,
        NSOpenGLPFAMaximumPolicy,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAAccelerated,
        0
    };
#endif

    return [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
}

/* per-window timer function, basic time based animation preformed here */
- (void) animationTimer:(NSTimer *)localTimer
{
    NSData* localSound = [_shared soundSimulation];
    
    if (localSound)
    {
        if (_timedAudio)
        {
            [_timedAudio stop];
        }
        _timedAudio = [[NSSound alloc] initWithData:localSound];
        _timedAudio.loops = YES;
        [_timedAudio play];
    }
	[self drawRect:[self bounds]]; /* redraw now instead dirty to enable updates during live resize */
}

- (void) awakeFromNib
{    
    NSLog(@"Starting up");
    [self startEverything:YES];
}

- (void) drawRect:(NSRect)rect
{
    NSSize size = rect.size;

    glViewport(0, 0, (GLsizei)size.width, (GLsizei)size.height);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, size.width, 0, size.height, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_LINE_SMOOTH);

    [[self openGLContext] makeCurrentContext];
    [_shared draw:size];
    [[self openGLContext] flushBuffer];
}

- (void) quitProcedure
{
#ifdef NEW_OPENGL_ENVIRONMENT
    polygonal_close();
#endif
    
    NSLog(@"Quitting");

    [_shared close];
    
    NSLog(@"Quit");

    exit(0);
}

- (IBAction) aboutDialog:(id) sender
{
    [self.shared about:@"Macintosh INTEL Cocoa"];
}

- (id) initWithFrame:(NSRect) frameRect
{
    NSOpenGLPixelFormat * pf = [NobleMacView basicPixelFormat];
	self = [super initWithFrame:frameRect pixelFormat: pf];
    
    _shared = [[NobleShared alloc] initWithFrame:frameRect];
    return self;
}

- (BOOL) acceptsFirstResponder
{
    return YES;
}

- (BOOL) becomeFirstResponder
{
    return YES;
}

- (BOOL) resignFirstResponder
{
    return YES;
}

- (void) startEverything:(BOOL)headyLifting
{
    NSSize increments;
    
    increments.height = 4;
    increments.width = 4;
    [[self window] setContentResizeIncrements:increments];

    if (headyLifting)
    {
        NSUInteger processors = [[NSProcessInfo processInfo] processorCount];
        [_shared numberThreads:processors];
        NSLog(@"We have %ld processors", processors);

        if ([_shared start] == NO)
        {
            NSLog(@"Simulation initialization failed");
            [self quitProcedure];
            return;
        }
    }
    /* start animation timer */
    _timerAnimation = [NSTimer timerWithTimeInterval:[_shared timeInterval] target:self selector:@selector(animationTimer:) userInfo:nil repeats:YES];
    
    [[NSRunLoop currentRunLoop] addTimer:_timerAnimation forMode:NSDefaultRunLoopMode];
    
    [[self window] makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];
}

#pragma mark ---- Method Overrides ----

- (void) keyUp:(NSEvent *)theEvent
{
    [_shared keyUp];
}

- (void) keyDown:(NSEvent *)theEvent
{
    NSUInteger  local_key = 0;
    if (([theEvent modifierFlags] & NSControlKeyMask) || ([theEvent modifierFlags] & NSAlternateKeyMask))
    {
        local_key = 2048;
    }
    if ([theEvent modifierFlags] & NSNumericPadKeyMask)
    {
        /* arrow keys have this mask */
        NSString *theArrow = [theEvent charactersIgnoringModifiers];
        unichar keyChar = 0;
        if ( [theArrow length] == 0 )
            return;            /* reject dead keys */
        if ( [theArrow length] == 1 )
        {
            keyChar = [theArrow characterAtIndex:0];
            if ( keyChar == NSLeftArrowFunctionKey )
            {
                local_key += 28;
            }
            else if ( keyChar == NSRightArrowFunctionKey )
            {
                local_key += 29;
            }
            else if ( keyChar == NSUpArrowFunctionKey )
            {
                local_key += 30;
            }
            else if ( keyChar == NSDownArrowFunctionKey )
            {
                local_key += 31;
            }
            
            [_shared keyReceived:local_key];
        }
    }
}

- (BOOL) acceptsFirstMouse
{
    return YES;
}

- (void) mouseDown:(NSEvent *)theEvent
{
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    NSInteger location_x = (NSInteger)location.x;
    NSInteger location_y = (NSInteger)([self bounds].size.height - location.y);
    
    [_shared mouseOption:(([theEvent modifierFlags] & NSControlKeyMask) || ([theEvent modifierFlags] & NSAlternateKeyMask))];
    [_shared mouseReceivedWithXLocation:location_x YLocation:location_y];
}

- (void) rightMouseDown:(NSEvent *)theEvent
{
	[self mouseDown:theEvent];
    [_shared mouseOption:YES];
}

- (void) otherMouseDown:(NSEvent *)theEvent
{
	[self rightMouseDown:theEvent];
}

- (void) mouseUp:(NSEvent *)theEvent
{
    [_shared mouseUp];
}

- (void) rightMouseUp:(NSEvent *)theEvent
{
	[self mouseUp:theEvent];
}

- (void) otherMouseUp:(NSEvent *)theEvent
{
	[self mouseUp:theEvent];
}

- (void) mouseDragged:(NSEvent *)theEvent
{
	[self mouseDown:theEvent];
}

- (void) rightMouseDragged:(NSEvent *)theEvent
{
	[self rightMouseDown:theEvent];
}

- (void) otherMouseDragged:(NSEvent *)theEvent
{
	[self rightMouseDown:theEvent];
}

- (void) scrollWheel:(NSEvent *)theEvent
{
    [_shared delta_x:[theEvent deltaX] delta_y:[theEvent deltaY]];
}

- (void) magnifyWithEvent:(NSEvent *)event
{
    [_shared zoom:[event magnification]];
}

- (void) rotateWithEvent:(NSEvent *)event
{
    [_shared rotation:[event rotation]];
}

@end
