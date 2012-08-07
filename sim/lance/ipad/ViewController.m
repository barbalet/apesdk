/* ViewController.m */


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
