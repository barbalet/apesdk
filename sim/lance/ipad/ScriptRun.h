/* ScriptRun.h */

@interface ScriptRun: NSObject

- (void) cleanUp;

- (void) load:(NSString *) scriptCode;

- (BOOL)run;

- (NSString *) outputString;

@end