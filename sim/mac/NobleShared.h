/****************************************************************
 
 NobleShared.h
 
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

#import <Cocoa/Cocoa.h>

#include "../noble/shared.h"
#include "../gui/gui.h"

@interface NobleShared : NSObject

- (id) initWithFrame:(NSRect)frameRect;

- (BOOL) start;

- (void) about:(NSString *)aboutName;

- (void) draw:(NSSize)size;

- (void) draw:(unsigned char *)buffer width:(NSInteger)width height:(NSInteger)height;

- (void) keyReceived:(NSUInteger)key;

- (void) mouseReceivedWithXLocation:(NSInteger)xLocation YLocation:(NSInteger)yLocation;

- (void) mouseOption:(BOOL)mouseOption;

- (void) keyUp;

- (void) mouseUp;

- (void) rotation:(float)rotationAmount;

- (void) delta_x:(n_double)delta_x delta_y:(n_double)delta_y;

- (void) zoom:(float)zoomAmount;

- (NSTimeInterval) timeInterval;

- (void) scriptDebugHandle:(NSString *)fileName;

- (void) close;

- (void) identificationBasedOnName:(NSString *)windowName;

- (void) newSimulation;

- (void) cycleWithWidth:(NSInteger)width height:(NSInteger)height;

- (BOOL) cycleDebugOutput;

- (BOOL) cycleQuit;

- (BOOL) menuPause;

- (void) menuPreviousApe;

- (void) menuNextApe;

- (void) menuClearErrors;

- (BOOL) menuNoTerritory;

- (BOOL) menuNoWeather;

- (BOOL) menuNoBrain;

- (BOOL) menuNoBrainCode;

- (BOOL) menuDaylightTide;

- (void) menuFlood;

- (void) menuHealthyCarrier;

- (void) savedFileName:(NSString*)name;

- (BOOL) openFileName:(NSString*)name isScript:(BOOL)scriptFile;

+ (void) starting:(int)argc withValues:(char **)argv;

- (void) numberThreads:(long)number;

- (NSData*) soundSimulation;
    
@property (nonatomic, assign, readonly) n_int  identification;

@end
