/****************************************************************

 NoblePadView.m

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


#include "../noble/noble.h"
#include "../universe/universe.h"
#include "shared.h"

#import "NoblePadView.h"

// ===================================

@implementation NoblePadView

// ---------------------------------

// per-window timer function, basic time based animation preformed here
- (void)animationTimer
{
    [self setNeedsDisplay];
}

- (float)screenScale
{
    return 1; /*[[UIScreen mainScreen] scale];*/
}

- (void) drawRect:(CGRect)rect;
{
    float scaleFactor = [self screenScale];
    
    CGContextRef   context = UIGraphicsGetCurrentContext();
    n_int          dimensionX = rect.size.width * scaleFactor;
    n_int          dimensionY = rect.size.height * scaleFactor;
    static n_int   oldDimensionX = -1;
    n_c_uint *     localOffBuffer;
    
    (void)shared_cycle(CFAbsoluteTimeGetCurrent(), NUM_TERRAIN, dimensionX, dimensionY);

    localOffBuffer = shared_draw(NUM_TERRAIN, dimensionX, dimensionY);
    
    if (drawRef == nil || (oldDimensionX != dimensionX))
    {
        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
        drawRef = CGBitmapContextCreate(localOffBuffer, rect.size.width * scaleFactor, rect.size.height * scaleFactor, 8, rect.size.width * 4 * scaleFactor, colorSpace, (CGBitmapInfo)/*kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedFirst*/ kCGBitmapByteOrder32Little|kCGImageAlphaNoneSkipFirst);
        CGColorSpaceRelease( colorSpace );
        
        oldDimensionX = dimensionX;
    }
        
    CGContextSaveGState(context);
    
    CGImageRef local_image = CGBitmapContextCreateImage( drawRef );

    if ( local_image )
    {
        CGContextSetBlendMode(context, kCGBlendModeCopy);
        CGContextDrawImage(context, rect, local_image);
        CGImageRelease(local_image);
    }
    
    CGContextRestoreGState(context);
    CGContextScaleCTM(context, scaleFactor, scaleFactor); /* new */
}

// Handles the start of a touch
- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	[self touchesMoved:touches withEvent:event];
}

// Handles the continuation of a touch.
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	UITouch*	touch = [[event touchesForView:self] anyObject];
	CGPoint location = [touch locationInView:self];
    float scaleFactor = [self screenScale];

    
    shared_mouseReceived(location.x * scaleFactor, location.y * scaleFactor, NUM_TERRAIN);
}

// Handles the end of a touch event when the touch is a tap.
- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    shared_mouseUp();
}

- (void)motionEnded:(UIEventSubtype)motion withEvent:(UIEvent *)event
{
    if (motion == UIEventSubtypeMotionShake)
    {
        shared_menu(NA_MENU_NEXT_APE);
    }
}

- (void)buttonNextApe:(id)sender
{
    (void) shared_menu(NA_MENU_NEXT_APE);
}

- (void)buttonPreviousApe:(id)sender
{
    (void) shared_menu(NA_MENU_PREVIOUS_APE);
}

- (void)buttonClearErrors:(id)sender
{
    shared_menu(NA_MENU_CLEAR_ERRORS);
}

- (void)buttonNewSimulation:(id)sender
{
    shared_new(CFAbsoluteTimeGetCurrent ());
}


// ---------------------------------

-(id)initWithFrame:(CGRect)frame
{
	self = [super initWithFrame:frame];
    return self;
}
// ---------------------------------


// ---------------------------------

- (void) awakeFromNib
{    
    (void)shared_init(NUM_TERRAIN, CFAbsoluteTimeGetCurrent());
        
	displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(animationTimer)];
	[displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];

}


@end
