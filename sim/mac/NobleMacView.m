/****************************************************************
 
 NobleMacView.m
 
 =============================================================
 
 Copyright 1996-2015 Tom Barbalet. All rights reserved.
 
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

#import "NobleMacView.h"

#include <stdio.h>
#include <stdlib.h>

#pragma mark ---- OpenGL Utils ----

@implementation NobleMacView

static NSString * sharedString_LastSaved       = @"LastSaved";
static NSString * sharedString_LastOpen        = @"LastOpen";
static NSString * sharedString_LastOpenScript  = @"LastOpenScript";

/* pixel format definition */
+ (NSOpenGLPixelFormat*) basicPixelFormat
{
    NSOpenGLPixelFormatAttribute attributes [] = {
        NSOpenGLPFADoubleBuffer,	/* double buffered */
        NSOpenGLPFADepthSize,
        (NSOpenGLPixelFormatAttribute)16, /* 16 bit depth buffer */
        (NSOpenGLPixelFormatAttribute)0 /*nil*/
    };
    return [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
}

-(NSOpenPanel*) uniformOpenPanel
{
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    NSArray     *fileTypes = [[NSArray alloc] initWithObjects:@"txt", nil];
	[panel  setAllowedFileTypes:fileTypes];
    [panel  setCanChooseDirectories:NO];
    [panel  setAllowsMultipleSelection:NO];
    
    NSLog(@"Abtaining and returning uniform open panel");
    
    return panel;
}


-(NSSavePanel*) uniformSavePanel
{
    NSSavePanel *panel = [NSSavePanel savePanel];
    NSArray     *fileTypes = [[NSArray alloc] initWithObjects:@"txt", nil];
	[panel  setAllowedFileTypes:fileTypes];
    
    NSLog(@"Abtaining and returning uniform save panel");

    return panel;
}

- (void)debugOutput
{
    NSSavePanel *panel = [self uniformSavePanel];

    NSLog(@"Abtaining debug output");

    [panel  beginWithCompletionHandler:^(NSInteger result)
     {
         if (result == NSFileHandlingPanelOKButton)
         {
             NSString *name = [panel.URL path];
             char * cStringFileName = (char *)[name UTF8String];
             shared_script_debug_handle(cStringFileName);
         }
         else
         {
             shared_script_debug_handle(0L);
         }
     }];
}

/* per-window timer function, basic time based animation preformed here */
- (void) animationTimer:(NSTimer *)localTimer
{
	[self drawRect:[self bounds]]; /* redraw now instead dirty to enable updates during live resize */
}


- (void) awakeFromNib
{
    NSSize size = [[self window] frame].size;
    
    NSLog(@"Starting up");
    
    fIdentification = 0;
    
    [self startEverything];
    
    glViewport(0, 0, (GLsizei)size.width, (GLsizei)size.height);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, size.width, 0, size.height, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

- (void) drawRect:(NSRect)rect
{
    NSSize size = rect.size;
    
    [[self openGLContext] makeCurrentContext];
    
    shared_draw(0L, fIdentification, (n_int)size.width, (n_int)size.height);
    
    [[self openGLContext] flushBuffer];
}

- (void)quitProcedure
{
    NSLog(@"Quitting");

    shared_close();
    
    NSLog(@"Quit");

    exit(0);
}

-(id) initWithFrame:(NSRect) frameRect
{
    NSOpenGLPixelFormat * pf = [NobleMacView basicPixelFormat];
	self = [super initWithFrame: frameRect pixelFormat: pf];
    return self;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (BOOL)becomeFirstResponder
{
    return YES;
}

- (BOOL)resignFirstResponder
{
    return YES;
}

- (void)startEverything
{
    NSUInteger processors = [[NSProcessInfo processInfo] processorCount];
    NSSize increments;
    
    randomizing_agent = (n_uint)CFAbsoluteTimeGetCurrent();
    
    increments.height = 4;
    increments.width = 4;
    [[self window] setContentResizeIncrements:increments];
    
    NSLog(@"%@ window setup started", (fIdentification == NUM_TERRAIN ? @"Terrain" : @"Map"));
    
    execute_threads([[NSProcessInfo processInfo] processorCount]);
    
    NSLog(@"We have %ld processors", processors);
    
    if (fIdentification == NUM_TERRAIN)
    {
        NSLog(@"Initialization landscape with randomizing: %lx", randomizing_agent);
    }
    {
        n_int shared_response = shared_init(fIdentification, randomizing_agent);
        if (shared_response == -1)
        {
            NSLog(@"Simulation initialization failed");
            [self quitProcedure];
            return;
        }
        else
        {
            fIdentification = (n_byte)shared_response;
        }
    }
    
    /* start animation timer */
    {
        NSTimeInterval interval = 1.0f/((NSTimeInterval)shared_max_fps());
        
        timerAnimation = [NSTimer timerWithTimeInterval:interval target:self selector:@selector(animationTimer:) userInfo:nil repeats:YES];
    }
    
    [[NSRunLoop currentRunLoop] addTimer:timerAnimation forMode:NSDefaultRunLoopMode];
    
    [[self window] makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];
    NSLog(@"%@ window setup done", (fIdentification == NUM_TERRAIN ? @"Terrain" : @"Map"));
}

- (void)loadUrlString:(NSString*) urlString
{
    [[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString: urlString]];
}

#pragma mark ---- IB Actions ----

-(IBAction) aboutDialog:(id) sender
{
    shared_about("Macintosh INTEL Cocoa");
}

-(void)menuCheckMark:(id)sender check:(n_int)value
{
    if ([sender respondsToSelector:@selector(setState:)])
    {
        [sender setState:(value ? NSOnState : NSOffState)];
    }
}

-(IBAction) menuQuit:(id) sender
{
    NSLog(@"Quit from menu");
    [self quitProcedure];
}

-(IBAction) menuFileNew:(id) sender
{
    n_uint loop = 0;
    n_byte2 *ra_in_2_bytes = (n_byte2*)&randomizing_agent;
    
    randomizing_agent ^= (n_uint)CFAbsoluteTimeGetCurrent();
    
    NSLog(@"New landscape with randomizing: %lx", randomizing_agent);
    
    while (loop < (sizeof(n_uint)/2))
    {
        math_random(&ra_in_2_bytes[loop*2]);
        loop++;
    }    
	if (shared_new(randomizing_agent) != 0)
    {
        [self quitProcedure];
    }
    NSLog(@"Finished new landscape");
}

-(IBAction) menuFileOpen:(id) sender
{
    NSOpenPanel *panel = [self uniformOpenPanel];
    [panel  beginWithCompletionHandler:^(NSInteger result)
     {
         if (result == NSFileHandlingPanelOKButton)
         {
             NSString *name = [panel.URL path];
             char * cStringFileName = (char *)[name UTF8String];
             NSUserDefaults *prefs = [NSUserDefaults standardUserDefaults];

             if (!shared_openFileName(cStringFileName,0))
             {
                 [[NSSound soundNamed:@"Pop"] play];
                 [prefs removeObjectForKey:sharedString_LastOpen];
             }
             else
             {
                 [prefs setObject:name forKey:sharedString_LastOpen];
             }
             [prefs synchronize];
         }
         
     }];
}


-(IBAction) menuFileOpenScript:(id) sender
{
    NSOpenPanel *panel = [self uniformOpenPanel];
    [panel  beginWithCompletionHandler:^(NSInteger result)
     {
         if (result == NSFileHandlingPanelOKButton)
         {
             NSString *name = [panel.URL path];
             char * cStringFileName = (char *)[name UTF8String];
             NSUserDefaults *prefs = [NSUserDefaults standardUserDefaults];
             if (!shared_openFileName(cStringFileName,1))
             {
                 [[NSSound soundNamed:@"Pop"] play];
                 [prefs removeObjectForKey:sharedString_LastOpenScript];
             }
             else
             {
                 [prefs setObject:name forKey:sharedString_LastOpenScript];
             }
             [prefs synchronize];
         }
     }];
}


-(IBAction) menuFileSaveAs:(id) sender
{
    NSSavePanel *panel = [self uniformSavePanel];
    [panel  beginWithCompletionHandler:^(NSInteger result)
     {
         if (result == NSFileHandlingPanelOKButton)
         {
             NSString *name = [panel.URL path];
             char * cStringFileName = (char *)[name UTF8String];
             NSUserDefaults *prefs = [NSUserDefaults standardUserDefaults];
             [prefs setObject:name forKey:sharedString_LastSaved];
             [prefs synchronize];
             shared_saveFileName(cStringFileName);
         }
         
     }];
}

#pragma mark ---- Method Overrides ----

- (void)keyUp:(NSEvent *)theEvent
{
    shared_keyUp();
}

- (void)keyDown:(NSEvent *)theEvent
{
    n_byte2  local_key = 0;
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
            
            NSLog(@"%@ window has key pressed value: %d", (fIdentification == NUM_TERRAIN ? @"Terrain" : @"Map"), local_key);
            
			shared_keyReceived(local_key, fIdentification);
        }
    }
}

- (BOOL)acceptsFirstMouse
{
    return YES;
}

- (void)mouseDown:(NSEvent *)theEvent
{
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    n_int location_x = (n_int)location.x;
    n_int location_y = (n_int)([self bounds].size.height - location.y);
    if (([theEvent modifierFlags] & NSControlKeyMask) || ([theEvent modifierFlags] & NSAlternateKeyMask))
    {
        shared_mouseOption(1);
        NSLog(@"Mouse option pressed");
    }
    else
    {
        shared_mouseOption(0);
        NSLog(@"No mouse option pressed");
    }
	shared_mouseReceived(location_x, location_y, fIdentification);
    
    NSLog(@"%@ window has mouse pressed: %ld, %ld", (fIdentification == NUM_TERRAIN ? @"Terrain" : @"Map"), location_x, location_y);
}

- (void)rightMouseDown:(NSEvent *)theEvent
{
	[self mouseDown:theEvent];
    shared_mouseOption(1);
}

- (void)otherMouseDown:(NSEvent *)theEvent
{
	[self rightMouseDown:theEvent];
}

- (void)mouseUp:(NSEvent *)theEvent
{
    shared_mouseUp();
}

- (void)rightMouseUp:(NSEvent *)theEvent
{
	[self mouseUp:theEvent];
}

- (void)otherMouseUp:(NSEvent *)theEvent
{
	[self mouseUp:theEvent];
}

- (void)mouseDragged:(NSEvent *)theEvent
{
	[self mouseDown:theEvent];
}

- (void)rightMouseDragged:(NSEvent *)theEvent
{
	[self rightMouseDown:theEvent];
}

- (void)otherMouseDragged:(NSEvent *)theEvent
{
	[self rightMouseDown:theEvent];
}

- (void)scrollWheel:(NSEvent *)theEvent
{
    
}

- (void)beginGestureWithEvent:(NSEvent *)event
{
    /* NSLog(@"New behavior: beginGestureWithEvent"); */
}

- (void)endGestureWithEvent:(NSEvent *)event
{
    /* NSLog(@"New behavior: endGestureWithEvent"); */
}

- (void)magnifyWithEvent:(NSEvent *)event
{
    /* NSLog(@"New behavior: magnifyWithEvent %f %@", [event magnification], event); */
    /* (1.0 + [event magnification]) */
}

- (void)swipeWithEvent:(NSEvent *)event
{
    /* NSLog(@"New behavior: swipeWithEvent %f %f %@", [event deltaX], [event deltaY], event); */
}

- (void)rotateWithEvent:(NSEvent *)event
{
    shared_rotate((n_double)[event rotation], fIdentification);
}

@end
