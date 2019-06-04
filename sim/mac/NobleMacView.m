/****************************************************************
 
 NobleMacView.m
 
 =============================================================
 
 Copyright 1996-2019 Tom Barbalet. All rights reserved.
 
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

#import "NobleMacView.h"

#ifdef MUSHROOM
#import "NobleShared.h"
#else
#ifdef WARFARE
#import "Noble_Warfare-Swift.h"
#else
#import "Noble_Ape-Swift.h"
#endif
#endif


#include <stdio.h>
#include <stdlib.h>

@interface NobleMacView()


@end

@implementation NobleMacView
{
    NobleMTKRenderer *_renderer;
}


- (void) sharedReady
{
    // Set the view to use the default device
    self.device = MTLCreateSystemDefaultDevice();
    
    if(!self.device)
    {
        NSLog(@"Metal is not supported on this device");
        return;
    }
    
    _renderer = [[NobleMTKRenderer alloc] initWithMetalKitView:self NobleShared:self.shared];
    
    if(!_renderer)
    {
        NSLog(@"Renderer failed initialization");
        return;
    }
    
    // Initialize our renderer with the view size
    [_renderer mtkView:self drawableSizeWillChange:self.drawableSize];
    
    self.delegate = _renderer;
}

static CVReturn renderCallback(CVDisplayLinkRef displayLink,
                               const CVTimeStamp *inNow,
                               const CVTimeStamp *inOutputTime,
                               CVOptionFlags flagsIn,
                               CVOptionFlags *flagsOut,
                               void *displayLinkContext)
{
    return [(__bridge NobleMacView *)displayLinkContext renderTime:inOutputTime];
}

- (CVReturn) renderTime:(const CVTimeStamp *)inOutputTime
{
    [self.shared cycle];
    
    dispatch_async(dispatch_get_main_queue(), ^{
        self.needsDisplay = YES;
    });
    return kCVReturnSuccess;
}

- (void) startView
{
    self.shared = [[NobleShared alloc] initWithFrame:[self bounds]];
    
    NSLog(@"NobleMacUpdate startView %@", self.shared);
    
    CGDirectDisplayID   displayID = CGMainDisplayID();
    CVReturn            error = kCVReturnSuccess;
    error = CVDisplayLinkCreateWithCGDisplay(displayID, &displayLink);
    if (error)
    {
        NSLog(@"DisplayLink created with error:%d", error);
        displayLink = NULL;
    }
    CVDisplayLinkSetOutputCallback(displayLink, renderCallback, (__bridge void *)self);
    CVDisplayLinkStart(displayLink);
}

- (void) awakeFromNib
{    
    NSLog(@"Starting up");
    [self startView];
    [self startEverything:YES];
    [self sharedReady];
}



- (void) quitProcedure
{
    CVDisplayLinkStop(displayLink);

    NSLog(@"Quitting");

    [self.shared close];
    
    NSLog(@"Quit");

    exit(0);
}

- (IBAction) menuQuit:(id) sender
{
    NSLog(@"Quit from menu");
    [self quitProcedure];
}

- (IBAction) aboutDialog:(id) sender
{
    [self.shared about:@"Macintosh INTEL Cocoa"];
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
        if ([self.shared start] == NO)
        {
            NSLog(@"Simulation initialization failed");
            [self quitProcedure];
            return;
        }
    }
    [[self window] makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];
}

#pragma mark ---- Method Overrides ----

- (void) keyUp:(NSEvent *)theEvent
{
    [self.shared keyUp];
}

- (void) keyDown:(NSEvent *)theEvent
{
    NSUInteger  local_key = 0;
    if (([theEvent modifierFlags] & NSEventModifierFlagControl) || ([theEvent modifierFlags] & NSEventModifierFlagOption))
    {
        local_key = 2048;
    }
    
    if ([theEvent modifierFlags] & NSEventModifierFlagNumericPad)
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
            
            [self.shared keyReceived:local_key];
        }
    }
    if ([theEvent characters])
    {
        NSRange first = [[theEvent characters] rangeOfComposedCharacterSequenceAtIndex:0];
        NSRange match = [[theEvent characters] rangeOfCharacterFromSet:[NSCharacterSet letterCharacterSet] options:0 range:first];
        if (match.location != NSNotFound) {
            unichar firstChar = [[theEvent characters] characterAtIndex:0];
            NSCharacterSet *letters = [NSCharacterSet letterCharacterSet];
            if ([letters characterIsMember:firstChar]) {
                // The first character is a letter in some alphabet
                [self.shared keyReceived:firstChar];
            }
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
    [self.shared mouseOption:(([theEvent modifierFlags] & NSEventModifierFlagControl) || ([theEvent modifierFlags] & NSEventModifierFlagOption))];
    [self.shared mouseReceivedWithXLocation:location.x yLocation:[self bounds].size.height - location.y];
}

- (void) rightMouseDown:(NSEvent *)theEvent
{
	[self mouseDown:theEvent];
    [self.shared mouseOption:YES];
}

- (void) otherMouseDown:(NSEvent *)theEvent
{
	[self rightMouseDown:theEvent];
}

- (void) mouseUp:(NSEvent *)theEvent
{
    [self.shared mouseUp];
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
    [self.shared delta_x:[theEvent deltaX] delta_y:[theEvent deltaY]];
}

- (void) magnifyWithEvent:(NSEvent *)event
{
    [self.shared zoom:[event magnification]];
}

- (void) rotateWithEvent:(NSEvent *)event
{
    [self.shared rotation:[event rotation]];
}

@end
