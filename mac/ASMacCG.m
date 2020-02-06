/****************************************************************
 
ASMacCG.m
 
 =============================================================
 
 Copyright 1996-2020 Tom Barbalet. All rights reserved.
 
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
 
 This software is a continuing work of Tom Barbalet, begun on
 13 June 1996. No apes or cats were harmed in the writing of
 this software.
 
 ****************************************************************/

#import "ASMacCG.h"

#include <stdio.h>
#include <stdlib.h>

@interface ASMacCG()

@end

@implementation ASMacCG

- (void) drawRect:(NSRect)rect
{
    NSInteger      dim_x = (NSInteger)rect.size.width;
    NSInteger      dim_y = (NSInteger)rect.size.height;
    static unsigned char   outputBuffer[TERRAIN_WINDOW_AREA*4];
    
    [self.shared draw:outputBuffer width:dim_x height:dim_y];
    
    const float scaleFactor = 1;
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef     drawRef = CGBitmapContextCreate(outputBuffer, (size_t)(rect.size.width * scaleFactor), (size_t)(rect.size.height * scaleFactor), 8, (size_t)(rect.size.width * 4 * scaleFactor), colorSpace, (CGBitmapInfo)/*kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedFirst*/ kCGBitmapByteOrder32Big|kCGImageAlphaNoneSkipFirst);
    
    CGColorSpaceRelease( colorSpace );
    
    CGContextRef context = [[NSGraphicsContext currentContext] CGContext];
    CGContextSaveGState(context);
    
    CGContextSetBlendMode(context, kCGBlendModeCopy);
    CGContextSetShouldAntialias(context, NO);
    CGContextSetAllowsAntialiasing(context, NO);
    
    CGImageRef image = CGBitmapContextCreateImage(drawRef);
    
    CGContextDrawImage(context, rect, image);
    
    CGContextRestoreGState(context);
    
    CGImageRelease(image);
    
    CGContextRelease(drawRef);
}

@end
