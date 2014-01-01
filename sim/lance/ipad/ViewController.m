/****************************************************************
 
 ViewController.m
 
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

#import "ScriptRun.h"
#import "ViewController.h"

#include "noble.h"
#include "commands.h"

@interface ViewController()

@property (nonatomic, assign) BOOL runState;
@property (nonatomic, retain) ScriptRun * script;

@end

@implementation ViewController

ViewController * single = nil;

NSTimer        * timer = nil;

@synthesize runState             = runState_;

@synthesize runStopButton        = runStopButton_;
@synthesize inputScriptTextView  = inputScriptTextView_;
@synthesize outputScriptTextView = outputScriptTextView_;

@synthesize script = _script;

- (id) init
{
    if (single)
    {
        self = single;
        return single;
    }
    self = [super init];

    single = self;
    return self;
}

- (void) updateText
{
    file_debug->data[file_debug->location] = 0;
    
    self.outputScriptTextView.text = [NSString stringWithFormat:@"%s",file_debug->data];
    
    io_file_reused(file_debug);
        
    [self.outputScriptTextView scrollRangeToVisible:NSMakeRange([self.outputScriptTextView.text length], 0)]; 
}


- (void)changeState
{
    if (self.runState)
    {   
        [self.runStopButton setTitle:@"Run" forState:UIControlStateNormal];
        
    }
    else
    {
        [self.runStopButton setTitle:@"Stop" forState:UIControlStateNormal];

        NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
        [defaults setObject:self.inputScriptTextView.text forKey:@"SavedScript"];
        [defaults synchronize];
        
        [self.script load:self.inputScriptTextView.text];        
    }
    self.runState = !self.runState;


}


- (IBAction)runStopButtonPressed:(id)sender;
{
    [self changeState];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

- (void)viewDidLoad
{
    
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSString       *savedScript = [defaults valueForKey:@"SavedScript"];

    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    
    if (savedScript != nil)
    {
        self.inputScriptTextView.text = savedScript;
    }
    
    self.script = [[ScriptRun alloc] init];
    
    // start animation timer
	timer = [NSTimer timerWithTimeInterval:(1.0f/FRACTION_OF_SECOND) target:self selector:@selector(interpreterTimer:) userInfo:nil repeats:YES];
	[[NSRunLoop currentRunLoop] addTimer:timer forMode:NSDefaultRunLoopMode];
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
}

- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
	[super viewWillDisappear:animated];
}

- (void)viewDidDisappear:(BOOL)animated
{
	[super viewDidDisappear:animated];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone) {
        return (interfaceOrientation != UIInterfaceOrientationPortraitUpsideDown);
    } else {
        return YES;
    }
}

- (void) dealloc
{
    [self.script cleanUp];
}

- (void) interpreterTimer:(NSTimer *)localTimer
{
    if (self.runState)
    {
        if (![self.script run])
        {
            [self changeState];
        }
        [self updateText];
    }
}

@end
