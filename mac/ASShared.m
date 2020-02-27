/****************************************************************
 
 ASShared.m
 
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

#import "ASShared.h"

@interface ASShared()

@property (nonatomic, assign, readwrite) n_int  identification;
@property (nonatomic, assign) n_uint randomizing_agent;
@property (nonatomic, assign) shared_cycle_state returned_value;

@property (nonatomic, assign) NSInteger old_size_width;
@property (nonatomic, assign) NSInteger old_size_height;

@end

@implementation ASShared

- (id) init
{
    if (self)
    {
        _identification = 0;
        _old_size_width = 0;
        _old_size_height = 0;
    }
    return self;
}

- (id) initWithFrame:(NSRect)frameRect
{
    if (self)
    {
        _identification = 0;
        _old_size_width = (NSInteger)frameRect.size.width;
        _old_size_height = (NSInteger)frameRect.size.height;
    }
    return self;
}

- (void) about
{
    shared_about();
}


- (void) updateRandomizingAgent
{
    n_uint loop = 0;
    _randomizing_agent ^= (n_uint)CFAbsoluteTimeGetCurrent();
    n_byte2 *ra_in_2_bytes = (n_byte2*)&_randomizing_agent;
    while (loop < (sizeof(n_uint)/2))
    {
        math_random(&ra_in_2_bytes[loop*2]);
        loop++;
    }
}

- (void) newSimulation
{
    [self updateRandomizingAgent];
    (void)shared_new(_randomizing_agent);
}

- (void) newAgents
{
    [self updateRandomizingAgent];
    (void)shared_new_agents(_randomizing_agent);
}

- (BOOL) start
{
    NSLog(@"Randomizing element...");
    _randomizing_agent  = (n_uint)CFAbsoluteTimeGetCurrent();
    _randomizing_agent ^= (n_uint)CFAbsoluteTimeGetCurrent()>>8;
    _randomizing_agent ^= (n_uint)CFAbsoluteTimeGetCurrent()>>16;
    _randomizing_agent ^= (n_uint)CFAbsoluteTimeGetCurrent()>>24;
    
    n_int shared_response = shared_init((n_byte)_identification, _randomizing_agent);
    if (shared_response == -1)
    {
        return NO;
    }
    _identification = (n_byte)shared_response;
    return YES;
}

- (void) scriptDebugHandle:(NSString *)fileName
{
    char * cStringFileName = 0L;
    if (fileName)
    {
        cStringFileName = (char *)[fileName UTF8String];
    }
    shared_script_debug_handle(cStringFileName);
}

- (void) draw:(unsigned char *)buffer width:(NSInteger)width height:(NSInteger)height
{
    BOOL size_changed = (width != _old_size_width) || (height != _old_size_height);
    _old_size_width = width;
    _old_size_height = height;
    shared_draw(buffer, (n_byte)_identification, width, height, (n_byte)size_changed);
}

- (void) keyReceived:(NSUInteger)key
{
    shared_keyReceived((n_byte2)key, (n_byte)_identification);
}

- (void) keyUp
{
    shared_keyUp();
}

- (void) mouseUp
{
    shared_mouseUp();
}

- (void) rotation:(double)rotationAmount
{
    shared_rotate((n_double)rotationAmount, (n_byte)_identification);
}

- (void) delta_x:(double)delta_x delta_y:(double)delta_y
{
    shared_delta(delta_x, delta_y, (n_byte)_identification);
}

- (void) zoom:(double)zoomAmount;
{
    shared_zoom((n_double)zoomAmount, (n_byte)_identification);
}

- (NSTimeInterval) timeInterval
{
    return 1.0f/((NSTimeInterval)shared_max_fps());
}

- (void) mouseReceivedWithXLocation:(n_double)xLocation yLocation:(n_double)yLocation
{
    shared_mouseReceived(xLocation, yLocation, (n_byte)_identification);;
}

- (void) mouseOption:(BOOL)mouseOption
{
    if (mouseOption)
    {
        shared_mouseOption(1);
    }
    else
    {
        shared_mouseOption(0);
    }
}

- (void) close
{
    shared_close();
}

- (void) identificationBasedOnName:(NSString *)windowName
{
    _identification = NUM_VIEW;
    
    if ([windowName isEqualToString:@"Terrain"])
    {
        _identification = NUM_TERRAIN;
    }
}

- (void) cycle
{
    _returned_value = shared_cycle((n_uint)CFAbsoluteTimeGetCurrent(), (n_byte)_identification);
}

- (BOOL) cycleDebugOutput
{
    return (_returned_value == SHARED_CYCLE_DEBUG_OUTPUT);
}

- (BOOL) cycleNewApes
{
    return (_returned_value == SHARED_CYCLE_NEW_APES);
}

- (BOOL) cycleQuit
{
    return (_returned_value == SHARED_CYCLE_QUIT);
}

- (BOOL) menuPause
{
    return shared_menu(NA_MENU_PAUSE) ? YES : NO;
}

- (void) menuPreviousApe
{
    (void) shared_menu(NA_MENU_PREVIOUS_APE);
}

- (void) menuNextApe
{
    (void) shared_menu(NA_MENU_NEXT_APE);
}

- (void) menuClearErrors
{
    (void) shared_menu(NA_MENU_CLEAR_ERRORS);
}

- (BOOL) menuNoTerritory
{
    return shared_menu(NA_MENU_TERRITORY) ? YES : NO;
}

- (BOOL) menuNoWeather
{
    return shared_menu(NA_MENU_WEATHER) ? YES : NO;
}

- (BOOL) menuNoBrain
{
    return shared_menu(NA_MENU_BRAIN) ? YES : NO;
}

- (BOOL) menuNoBrainCode
{
    return shared_menu(NA_MENU_BRAINCODE) ? YES : NO;
}

- (BOOL) menuDaylightTide
{
    return shared_menu(NA_MENU_TIDEDAYLIGHT) ? YES : NO;
}

- (void) menuFlood
{
    (void) shared_menu(NA_MENU_FLOOD);
}

- (void) menuHealthyCarrier
{
    (void) shared_menu(NA_MENU_HEALTHY_CARRIER);
}

- (void) savedFileName:(NSString*)name
{
     char * cStringFileName = (char *)[name UTF8String];
     shared_saveFileName(cStringFileName);
}

- (BOOL) openFileName:(NSString*)name isScript:(BOOL)scriptFile
{
    char * cStringFileName = (char *)[name UTF8String];
    return shared_openFileName(cStringFileName,(unsigned char)scriptFile) != 0;
}

@end
