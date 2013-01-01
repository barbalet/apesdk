/****************************************************************

 NoblePadView.m

 =============================================================

 Copyright 1996-2013 Tom Barbalet. All rights reserved.

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
#include "../gui/shared.h"

#import "NoblePadView.h"

// ===================================

@implementation NoblePadView

// ---------------------------------

// per-window timer function, basic time based animation preformed here
- (void)animationTimer
{
    shared_cycle(CFAbsoluteTimeGetCurrent(), fIdentification);
    [self setNeedsDisplay];
}

- (void) drawRect:(CGRect)rect;
{
    CGContextRef  context = UIGraphicsGetCurrentContext();
    int             dimX = 512;
    int             dimY = 512;
    int             dimension1 = 511;
    int             dimensionBin = 9;
    int             ly = 0;
    int             loop = 0;
    unsigned char * indexLocalX;
    unsigned char * index;
    unsigned short fit[256 * 3];
    int            loopColors=0;
    int localHeight = [self bounds].size.height;

    CGContextSaveGState(context);

    index = shared_draw(fIdentification);

    if (index == 0L) return;

    shared_timeForColor(fit, fIdentification);

    while(loopColors < 256)
    {
        unsigned char colR = fit[loop++] >> 8;
        unsigned char colG = fit[loop++] >> 8;
        unsigned char colB = fit[loop++] >> 8;

        colorLookUp[ loopColors ] = (colB << 24) | (colG << 16) | (colR << 8);

        loopColors++;
    }

    if (localHeight == 256)
    {
        dimX = 256;
        dimY = 256;
        dimension1 = 255;
        dimensionBin = 8;
    }

    if (localHeight == 768)
    {
        dimX = 1024;
        dimY = 768;
        dimension1 = 767;
        dimensionBin = 10;
    }
    
    if (localHeight == 1024)
    {
        dimX = 1024;
        dimY = 768;
        dimension1 = 767;
        dimensionBin = 10;
        
    }
    
    loop = 0;
    while(ly < dimY)
    {
        int lx = 0;
        indexLocalX = &index[((dimension1 - ly)<<dimensionBin)];
        while(lx < dimX)
        {
            offscreenBuffer[loop++] = colorLookUp[ indexLocalX[ lx++ ] ];
        }
        ly++;
    }
    CGImageRef local_image = CGBitmapContextCreateImage( drawRef );
    if ( local_image )
    {
        CGContextDrawImage( context, bounds, local_image );
        CGImageRelease( local_image );
    }
    CGContextRestoreGState(context);
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
    shared_mouseReceived(location.x, location.y, fIdentification);
}

// Handles the end of a touch event when the touch is a tap.
- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    shared_mouseUp();
}

- (void)buttonNextApe:(id)sender
{
    shared_nextApe();
}

- (void)buttonPreviousApe:(id)sender
{
    shared_previousApe();
}

- (void)buttonClearErrors:(id)sender
{
    shared_clearErrors();
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

    int localHeight = [self bounds].size.height;
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();

    fIdentification = shared_init(localHeight, 256, 768, CFAbsoluteTimeGetCurrent());
    
    switch(localHeight)
    {
        case 256:
            bounds = CGRectMake(0, 0, 256, 256);
            drawRef = CGBitmapContextCreate( offscreenBuffer, 256, 256, 8, 256 * 4, colorSpace, kCGImageAlphaNoneSkipFirst );
            break;
        case 512:
            bounds = CGRectMake(0, 0, 512, 512);
            drawRef = CGBitmapContextCreate( offscreenBuffer, 512, 512, 8, 512 * 4, colorSpace, kCGImageAlphaNoneSkipFirst );
            break;
        case 768:
            bounds = CGRectMake(0, 0, 1024, 768);
            drawRef = CGBitmapContextCreate( offscreenBuffer, 1024, 768, 8, 1024 * 4, colorSpace, kCGImageAlphaNoneSkipFirst );
            break;
    }

    CGColorSpaceRelease( colorSpace );

	displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(animationTimer)];
	[displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];

}


@end
