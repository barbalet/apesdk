/* ViewController.h */


#import <UIKit/UIKit.h>

@interface ViewController : UIViewController

@property (nonatomic, retain) IBOutlet UIButton   * runStopButton;
@property (nonatomic, retain) IBOutlet UITextView * inputScriptTextView;
@property (nonatomic, retain) IBOutlet UITextView * outputScriptTextView;

- (IBAction)runStopButtonPressed:(id)sender;

- (void) updateText;

@end
