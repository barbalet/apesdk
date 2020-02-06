/****************************************************************
 
ASMacCG.h
 
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

#import <Cocoa/Cocoa.h>

#ifdef MUSHROOM
#import "ASShared.h"
#else
#ifndef SIMULATED_PLANET
#ifdef WARFARE
#import "ASShared.h"
#else
#import "Simulated_Ape-Swift.h"
#endif
#else
#import "ASShared.h"
#ifndef SIMULATED_PLANET
#import "../apple/ASDefaults.h"
#define USE_SIMULATED_DEFAULTS
#endif
#endif
#endif

#include "../toolkit/toolkit.h"
#include "../sim/sim.h"

@interface ASMacCG : NSView

@property (nonatomic, strong) ASShared* shared;

- (void) drawRect:(NSRect)rect;

@end
