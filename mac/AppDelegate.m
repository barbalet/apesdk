/****************************************************************
 
 AppDelegate.m
 
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

#import "AppDelegate.h"

@interface AppDelegate ()
#ifdef WARFARE
@property (strong) NSString* selectedFileName;
@property (strong) ASShared* shared;
#endif
@end

@implementation AppDelegate

#ifdef WARFARE

- (NSOpenPanel*) uniformOpenPanel
{
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    NSArray     *fileTypes = [[NSArray alloc] initWithObjects:@"json", nil];
    [panel  setAllowedFileTypes:fileTypes];
    [panel  setCanChooseDirectories:NO];
    [panel  setAllowsMultipleSelection:NO];
    
    NSLog(@"Abtaining and returning uniform open panel");
    
    return panel;
}

- (IBAction)openFileManually:(id)sender;
{
    NSOpenPanel *panel = [self uniformOpenPanel];
    [panel  beginWithCompletionHandler:^(NSInteger result)
     {
         if (result == NSModalResponseOK)
         {
             [self processFile:[panel.URL path]];
         }
         
     }];
}

- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
    return [self processFile:filename];
}

- (BOOL)processFile:(NSString *)file
{
    NSLog(@"The following file has been dropped or selected: %@",file);
    // Process file here
    self.selectedFileName = [file copy];
    
    NSLog(@"processFile %@", self.selectedFileName);
    
    [self.shared openFileName:file isScript:NO];
    
    return  YES; // Return YES when file processed succesfull, else return NO.
}

- (void)addShared:(ASShared*)shared
{
    self.shared = shared;
}

#endif

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    NSLog(@"applicationDidFinishLaunching");

}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
    NSLog(@"applicationWillTerminate");

}


@end
