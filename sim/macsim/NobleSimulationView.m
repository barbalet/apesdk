/****************************************************************
 
 NobleSimulationView.m
 
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

#import "NobleSimulationView.h"

@implementation NobleSimulationView

- (void) awakeFromNib
{
    fIdentification = NUM_VIEW;
    
    if ([[[self window] title] isEqualToString:@"Terrain"])
    {
        fIdentification = NUM_TERRAIN;
    }
    [self startEverything];
}

- (void) drawRect:(NSRect)rect
{
    n_int           dim_x = (n_int)rect.size.width;
    n_int           dim_y = (n_int)rect.size.height;
    static n_byte   outputBuffer[2048*1536*3];
    
    {
        shared_cycle_state returned_value = shared_cycle((n_uint)CFAbsoluteTimeGetCurrent (), fIdentification, dim_x, dim_y);
        if (returned_value == SHARED_CYCLE_DEBUG_OUTPUT)
        {
            [self debugOutput];
        }
        if (returned_value == SHARED_CYCLE_QUIT)
        {
            [self quitProcedure];
        }
    }
    [[self openGLContext] makeCurrentContext];
    
    shared_draw(outputBuffer, fIdentification, dim_x, dim_y);
    
    glDrawPixels((GLsizei)dim_x, (GLsizei)dim_y,GL_RGB,GL_UNSIGNED_BYTE, (const GLvoid *)outputBuffer);
    [[self openGLContext] flushBuffer];
}

-(IBAction) menuControlPause:(id) sender
{
    [self menuCheckMark:sender check:shared_menu(NA_MENU_PAUSE)];
}

-(IBAction) menuControlPrevious:(id) sender
{
    (void) shared_menu(NA_MENU_PREVIOUS_APE);
}

-(IBAction) menuControlNext:(id) sender
{
    (void) shared_menu(NA_MENU_NEXT_APE);
}

-(IBAction) menuControlClearErrors:(id) sender
{
    (void) shared_menu(NA_MENU_CLEAR_ERRORS);
}

-(IBAction) menuControlNoTerritory:(id) sender
{
    [self menuCheckMark:sender check:shared_menu(NA_MENU_TERRITORY)];
}

-(IBAction) menuControlNoWeather:(id) sender
{
    [self menuCheckMark:sender check:shared_menu(NA_MENU_WEATHER)];
}

-(IBAction) menuControlNoBrain:(id) sender
{
    [self menuCheckMark:sender check:shared_menu(NA_MENU_BRAIN)];
}

-(IBAction) menuControlNoBrainCode:(id) sender
{
    [self menuCheckMark:sender check:shared_menu(NA_MENU_BRAINCODE)];
}

-(IBAction) menuControlFlood:(id) sender
{
    (void) shared_menu(NA_MENU_FLOOD);
}

-(IBAction) menuControlHealthyCarrier:(id) sender
{
    (void) shared_menu(NA_MENU_HEALTHY_CARRIER);
}

-(IBAction) loadManual:(id) sender
{
    [self loadUrlString:@"http://www.nobleape.com/man/"];
}

-(IBAction) loadSimulationPage:(id)sender
{
    [self loadUrlString:@"http://www.nobleape.com/sim/"];
}

@end
