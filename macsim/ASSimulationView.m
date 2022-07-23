/****************************************************************
 
 ASSimulationView.m
 
 =============================================================
 
 Copyright 1996-2022 Tom Barbalet. All rights reserved.
 
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


#ifndef    _WIN32

#include "../toolkit/toolkit.h"
#include "shared.h"
#include "../script/script.h"
#include "../sim/sim.h"

#else

#include "..\toolkit\toolkit.h"
#include "shared.h"
#include "..\script\script.h"
#include "..\sim\sim.h"

#endif

#import "ASSimulationView.h"



@implementation ASSimulationView

- (NSOpenPanel*) uniformOpenPanel
{
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    NSArray     *fileTypes = [[NSArray alloc] initWithObjects:@"txt", nil];
    [panel  setAllowedFileTypes:fileTypes];
    [panel  setCanChooseDirectories:NO];
    [panel  setAllowsMultipleSelection:NO];
    
    NSLog(@"Abtaining and returning uniform open panel");
    
    return panel;
}

- (NSSavePanel*) uniformSavePanel
{
    NSSavePanel *panel = [NSSavePanel savePanel];
    NSArray     *fileTypes = [[NSArray alloc] initWithObjects:@"txt", nil];
    [panel  setAllowedFileTypes:fileTypes];
    
    NSLog(@"Abtaining and returning uniform save panel");
    
    return panel;
}


- (void) awakeFromNib
{
    [self startView];
    [self.shared identificationBasedOnName:[[self window] title]];
    [self startEverything:(self.shared.identification == WINDOW_PROCESSING)];
    [self sharedReady];
}

- (CVReturn) renderTime:(const CVTimeStamp *)inOutputTime
{    
    __weak ASSimulationView *weakSelf = self;
    dispatch_async(dispatch_get_main_queue(), ^{
        weakSelf.needsDisplay = YES;
    });

    return kCVReturnSuccess;
}

- (void) loadUrlString:(NSString*) urlString
{
    [[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString: urlString]];
}


- (void) debugOutput
{
    dispatch_async(dispatch_get_main_queue(), ^{
        NSSavePanel *panel = [self uniformSavePanel];
        NSLog(@"Abtaining debug output");
        [panel  beginWithCompletionHandler:^(NSInteger result)
         {
             if (result == NSModalResponseOK)
             {
                 [self.shared scriptDebugHandle:[panel.URL path]];
             }
         }];
    });
}

#pragma mark ---- IB Actions ----

- (void) menuCheckMark:(id)sender check:(int)value
{
    if ([sender respondsToSelector:@selector(setState:)])
    {
        [sender setState:(value ? NSControlStateValueOn : NSControlStateValueOff)];
    }
}

- (IBAction) menuFileNew:(id) sender
{
    [self.shared newSimulation];
    NSLog(@"Finished new landscape");
}

- (IBAction) menuFileNewAgents:(id) sender
{
    [self.shared newAgents];
    NSLog(@"Finished new agents");
}

- (IBAction) menuFileOpen:(id) sender
{
    NSOpenPanel *panel = [self uniformOpenPanel];
    [panel  beginWithCompletionHandler:^(NSInteger result)
     {
         if (result == NSModalResponseOK)
         {
             if (![self.shared openFileName:[panel.URL path] isScript:NO])
             {
                 [[NSSound soundNamed:@"Pop"] play];
             }
         }
         
     }];
}


- (IBAction) menuFileOpenScript:(id) sender
{
    NSOpenPanel *panel = [self uniformOpenPanel];
    [panel  beginWithCompletionHandler:^(NSInteger result)
     {
         if (result == NSModalResponseOK)
         {
             if (![self.shared openFileName:[panel.URL path] isScript:YES])
             {
                 [[NSSound soundNamed:@"Pop"] play];
             }
         }
     }];
}

- (IBAction) menuFileSaveAs:(id) sender
{
    NSSavePanel *panel = [self uniformSavePanel];
    [panel  beginWithCompletionHandler:^(NSInteger result)
     {
         if (result == NSModalResponseOK)
         {
             [self.shared savedFileName:[panel.URL path]];
         }
         
     }];
}

-(IBAction) menuControlPause:(id) sender
{
    [self menuCheckMark:sender check:[self.shared menuPause]];
}

-(IBAction) menuControlFollow:(id) sender
{
    [self menuCheckMark:sender check:[self.shared menuFollow]];
}

-(IBAction) menuControlSocialWeb:(id) sender
{
    [self menuCheckMark:sender check:[self.shared menuSocialWeb]];
}

-(IBAction) menuControlPrevious:(id) sender
{
    [self.shared menuPreviousApe];
}

-(IBAction) menuControlNext:(id) sender
{
    [self.shared menuNextApe];
}

-(IBAction) menuControlClearErrors:(id) sender
{
    [self.shared menuClearErrors];
}

-(IBAction) menuControlNoTerritory:(id) sender
{
    [self menuCheckMark:sender check:[self.shared menuNoTerritory]];
}

-(IBAction) menuControlNoWeather:(id) sender
{
    [self menuCheckMark:sender check:[self.shared menuNoWeather]];
}

-(IBAction) menuControlNoBrain:(id) sender
{
    [self menuCheckMark:sender check:[self.shared menuNoBrain]];
}

-(IBAction) menuControlNoBrainCode:(id) sender
{
    [self menuCheckMark:sender check:[self.shared menuNoBrainCode]];
}

-(IBAction) menuControlDaylightTide:(id)sender
{
    [self menuCheckMark:sender check:[self.shared menuDaylightTide]];
}

-(IBAction) menuControlFlood:(id) sender
{
    [self.shared menuFlood];
}

-(IBAction) menuControlHealthyCarrier:(id) sender
{
    [self.shared menuHealthyCarrier];
}

-(IBAction) menuCommandLine:(id) sender
{
    [self.shared menuCommandLineExecute];
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
