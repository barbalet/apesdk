/* ScriptRun.m */

#import "ScriptRun.h"

#include "noble.h"
#include "commands.h"

static n_interpret * interpret = 0L;

n_int draw_error(n_string error_text)
{
    NSLog(@"ERROR: %s",error_text);
    
    io_write(file_debug,"ERROR: ",0);
    io_write(file_debug,error_text,1);
    return -1;
}


@implementation ScriptRun

- (void) cleanUp
{
    interpret_cleanup(interpret);
    interpret = 0L;
}

- (void) load:(NSString *) scriptCode
{
    if (interpret)
    {
        [self cleanUp];
    }
    
    n_file local;
    
    local.size = [scriptCode length];
    local.location = 0;
    local.data = (n_byte *)[scriptCode UTF8String];
    interpret = parse_convert(&local, VARIABLE_MAIN, (variable_string *) variable_codes);
    
    if (interpret == 0L)
        NSLog(@"Initial Script Load Failed");
    else
    {
        SC_DEBUG_ON;
        
        interpret->sc_input  = &commands_input;
        interpret->sc_output = &commands_output;
        
        interpret->input_greater = VARIABLE_READWRITE;
        
        interpret->location = 0;
        interpret->leave = 0;
        interpret->localized_leave = 0;

        interpret->specific = 0;
    }
}

/* next step - move back into ApeScript */
-(BOOL)run
{
    n_int value = interpret_cycle(interpret, VARIABLE_EXIT - VARIABLE_FIRST_REAL_ONE, 0L,0,0L,0L);
    
    if (value < 1)
    {
        if (value == -1)
        {
            NSLog(@"Script Run Failed");
        }
        return NO;
    }
    return YES;
}

- (NSString *) outputString
{
    return nil;
}

@end