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

-(IBAction) loadManual:(id) sender
{
    [[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString: @"http://www.nobleape.com/man/"]];
}

-(IBAction) loadSimulationPage:(id)sender
{
    [[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString: @"http://www.nobleape.com/sim/"]];
}

@end
