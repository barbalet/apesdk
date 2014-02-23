/****************************************************************
 
 NobleMacView.m
 
 =============================================================
 
 Copyright 1996-2014 Tom Barbalet. All rights reserved.
 
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

#include "../gui/shared.h"
#include "../graph/graph.h"

#import "NobleMacView.h"

#include <stdio.h>
#include <stdlib.h>

#pragma mark ---- OpenGL Utils ----

@implementation NobleMacView

/* pixel format definition */
+ (NSOpenGLPixelFormat*) basicPixelFormat
{
    NSOpenGLPixelFormatAttribute attributes [] = {
        NSOpenGLPFAWindow,
        NSOpenGLPFADoubleBuffer,	/* double buffered */
        NSOpenGLPFADepthSize,
        (NSOpenGLPixelFormatAttribute)16, /* 16 bit depth buffer */
        (NSOpenGLPixelFormatAttribute)0 /*nil*/
    };
    return [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
}

- (void)debugOutput
{
    NSSavePanel *panel = [NSSavePanel savePanel];
    NSArray     *fileTypes = [[NSArray alloc] initWithObjects:@"txt", nil];
	[panel  setAllowedFileTypes:fileTypes];
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

- (void) drawRect:(NSRect)rect
{
    n_int         dimensionX = (n_int)rect.size.width;
    n_int         dimensionY = (n_int)rect.size.height;
    n_byte        * index = shared_draw(fIdentification);
    
    shared_cycle_state returned_value = shared_cycle((n_uint)CFAbsoluteTimeGetCurrent (), fIdentification, dimensionX, dimensionY);
    if (returned_value == SHARED_CYCLE_DEBUG_OUTPUT)
    {
        [self debugOutput];
    }
    if (returned_value == SHARED_CYCLE_QUIT)
    {
        [self quitProcedure];
    }
    
    if (index == 0L) return;
    
    do{
    }while (shared_draw(fIdentification) == 0);
    
    [[self openGLContext] makeCurrentContext];

    {
        n_int           ly = 0;
        n_int           loop = 0;
        n_int			loopColors = 0;
        n_byte2         fit[256*3];
        
        shared_timeForColor(fit, fIdentification);
        
        while(loopColors < 256)
        {
            colorTable[loopColors][0] = fit[loop++] >> 8;
            colorTable[loopColors][1] = fit[loop++] >> 8;
            colorTable[loopColors][2] = fit[loop++] >> 8;
            loopColors++;
        }
        loop = 0;
        while(ly < dimensionY)
        {
            n_int    lx = 0;
            n_byte * indexLocalX = &index[(dimensionY-ly-1)*dimensionX];
            while(lx < dimensionX)
            {
                unsigned char value = indexLocalX[lx++] ;
                outputBuffer[loop++] = colorTable[value][0];
                outputBuffer[loop++] = colorTable[value][1];
                outputBuffer[loop++] = colorTable[value][2];
            }
            ly++;
        }
    }

    glDrawPixels((GLsizei)dimensionX, (GLsizei)dimensionY,GL_RGB,GL_UNSIGNED_BYTE, (const GLvoid *)outputBuffer);
    [[self openGLContext] flushBuffer];
}

- (void)quitProcedure
{
    shared_close();
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

- (void) awakeFromNib
{
    NSSize increments;
    n_byte  window_value = NUM_TERRAIN;
    
    increments.height = 4;
    increments.width = 4;
    
    if ([[[self window] title] isEqualToString:@"View"])
    {
        window_value = NUM_VIEW;
    }

    execute_threads([[NSProcessInfo processInfo] processorCount]);
    
    {
        n_int shared_response = shared_init(window_value, (n_uint)CFAbsoluteTimeGetCurrent());
        
        if (shared_response == -1)
        {
            [self quitProcedure];
            return;
        }
        else
        {
            fIdentification = (n_byte)shared_response;
        }
    }
    
    [[self window] setContentResizeIncrements:increments];
        
    [[self window] setLevel:kCGMaximumWindowLevel];

    /* start animation timer */
	timerAnimation = [NSTimer timerWithTimeInterval:(1.0f/60.0f) target:self selector:@selector(animationTimer:) userInfo:nil repeats:YES];
    
    [[NSRunLoop currentRunLoop] addTimer:timerAnimation forMode:NSDefaultRunLoopMode];
}

#pragma mark ---- IB Actions ----

-(IBAction) aboutDialog:(id) sender
{
    shared_about("Macintosh INTEL Cocoa");
}

-(IBAction) menuControlPause:(id) sender
{
    (void)shared_notPause(); /* should handle check mark */
}

-(IBAction) menuControlPrevious:(id) sender
{
    shared_previousApe();
}

-(IBAction) menuControlNext:(id) sender
{
    shared_nextApe();
}

-(IBAction) menuControlClearErrors:(id) sender
{
    shared_clearErrors();
}

-(IBAction) menuControlNoTerritory:(id) sender
{
    shared_notTerritory();
}

-(IBAction) menuControlNoWeather:(id) sender
{
    shared_notWeather();
}

-(IBAction) menuControlNoBrain:(id) sender
{
    shared_notBrain();
}

-(IBAction) menuControlNoBrainCode:(id) sender
{
    shared_notBrainCode();
}

-(IBAction) menuControlFlood:(id) sender
{
    shared_flood();
}

-(IBAction) menuControlHealthyCarrier:(id) sender
{
    shared_healthy_carrier();
}

-(IBAction) menuQuit:(id) sender
{
    [self quitProcedure];
}

-(IBAction) menuFileNew:(id) sender
{
	if (shared_new((n_uint)CFAbsoluteTimeGetCurrent()) != 0)
    {
        [self quitProcedure];
    }
}

-(IBAction) menuFileOpen:(id) sender
{
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    NSArray     *fileTypes = [[NSArray alloc] initWithObjects:@"txt", nil];
	[panel setAllowedFileTypes:fileTypes];
    [panel setCanChooseDirectories:NO];
    [panel setAllowsMultipleSelection:NO];
    [panel  beginWithCompletionHandler:^(NSInteger result)
     {
         if (result == NSFileHandlingPanelOKButton)
         {
             NSString *name = [panel.URL path];
             char * cStringFileName = (char *)[name UTF8String];
             if (!shared_openFileName(cStringFileName,0))
             {
                 [[NSSound soundNamed:@"Pop"] play];
             }
         }
         
     }];
}


-(IBAction) menuFileOpenScript:(id) sender
{
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    NSArray     *fileTypes = [[NSArray alloc] initWithObjects:@"txt", nil];
	[panel      setAllowedFileTypes:fileTypes];
    [panel  setCanChooseDirectories:NO];
    [panel  setAllowsMultipleSelection:NO];
    [panel  beginWithCompletionHandler:^(NSInteger result)
     {
         if (result == NSFileHandlingPanelOKButton)
         {
             NSString *name = [panel.URL path];
             char * cStringFileName = (char *)[name UTF8String];
             if (!shared_openFileName(cStringFileName,1))
             {
                 [[NSSound soundNamed:@"Pop"] play];
             }
         }
         
     }];
}


-(IBAction) menuFileSaveAs:
(id) sender
{
    NSSavePanel *panel = [NSSavePanel savePanel];
    NSArray     *fileTypes = [[NSArray alloc] initWithObjects:@"txt", nil];
	[panel  setAllowedFileTypes:fileTypes];
    [panel  beginWithCompletionHandler:^(NSInteger result)
     {
         if (result == NSFileHandlingPanelOKButton)
         {
             NSString *name = [panel.URL path];
             char * cStringFileName = (char *)[name UTF8String];
             shared_saveFileName(cStringFileName);
         }
         
     }];
}

-(IBAction) loadManual:(id) sender
{
    [[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString: @"http://www.nobleape.com/man/"]];
}

-(IBAction) loadSimulationPage:(id)sender
{
    [[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString: @"http://www.nobleape.com/sim/"]];
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
			shared_keyReceived(local_key, fIdentification);
			[super keyDown:theEvent];
        }
    }
	[super keyDown:theEvent];
}

- (void)mouseDown:(NSEvent *)theEvent
{
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    n_int location_x = (n_int)location.x;
    n_int location_y = (n_int)location.y;
    if (([theEvent modifierFlags] & NSControlKeyMask) || ([theEvent modifierFlags] & NSAlternateKeyMask))
    {
        shared_mouseOption(1);
    }
    else
    {
        shared_mouseOption(0);
    }
	shared_mouseReceived(location_x, (n_int)([self bounds].size.height) - location_y, fIdentification);
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

@end
