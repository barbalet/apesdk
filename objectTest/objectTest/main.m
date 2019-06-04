/****************************************************************
 
 main.m
 
 =============================================================
 
 Copyright 1996-2019 Tom Barbalet. All rights reserved.
 
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
#import <Foundation/Foundation.h>

#include "noble.h"

extern void tof_gather(n_string file_in);
extern void tof_gather_string(n_string file_in);
extern void check_object(void);

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        do{
#if 0
            n_string example = "{\"general_variables\":\"test\",\"general_variables2\":{\"general_variables\":[1,2,4,5,9],\"general_variables2\":-12345,\"general_variables3\":{\"general_variables\":\"test\",\"general_variables2\":[\"hat\",\"cat\",\"horse\",\"rat\"],\"general_variables3\":\"test\"}},\"general_variables3\":\"test\"}";
            
            n_string example = "{\"general_variables\":\"general_test\"}";/* good */
            
            n_string example = "{\"general_variables\":[\"0\",\"1\",\"2\",\"3\"]}"; /* good*/

            n_string example = "{\"general_variables\":[1,2,3,4]}"; /* good */

            n_string example = "{\"general_variables\":[{\"agent\":\"yellow\"},{\"agent\":\"blue\"}]}"; /* good */

            
            n_string example = "{\"general_variables2\":{\"general_variables\":[1,2,4,5,9],\"general_variables2\":-12345,\"general_variables3\":{\"general_variables\":\"test\",\"general_variables2\":[\"hat\",\"cat\",\"horse\",\"rat\"],\"general_variables3\":\"test\"}}}"; /*bad */
            
            n_string example = "{\"general_variables2\":{\"general_variables\":[1,2,4,5,9],\"general_variables3\":{\"general_variables\":\"test\",\"general_variables2\":[\"hat\",\"cat\",\"horse\",\"rat\"]}}}";/* bad */
            
            
            n_string example = "{\"general_variables2\":{\"general_variables\":[1,2,4,5,9],\"general_variables3\":{\"general_variables2\":[\"hat\",\"cat\",\"horse\",\"rat\"]}}}"; /* bad */
            
            n_string example = "{\"general_variables2\":{\"general_variables3\":{\"general_variables2\":[\"hat\",\"cat\",\"horse\",\"rat\"]}}}"; /* good */

            n_string example = "{\"general_variables2\":{\"general_variables\":[1,2,4,5,9],\"general_variables3\":{\"general_variables2\":[\"hat\",\"cat\",\"horse\",\"rat\"]}}}"; /* bad */
            
            n_string example = "{\"general_variables\":[1,2,4,5,9],\"general_variables3\":{\"general_variables2\":[\"hat\",\"cat\",\"horse\",\"rat\"]}}"; /* bad */
            
            n_string example = "{\"general_variables\":[1,2,4,5,9],\"general_variables3\":{\"general_variables2\":0}}"; /* bad */
            
            n_string example = "{\"general_variables\":[1,2,4,5,9],\"general_variables3\":\"general_variables2\"}"; /* bad */

            n_string example = "{\"general_variables\":2,\"general_variables3\":\"general_variables2\"}"; /* bad */

            n_string example = "{\"general_variables\":2,\"general_variables3\":2}"; /* good */

#else
            n_string example = "{\"general_variables\":\"test\",\"general_variables2\":{\"general_variables\":[1,2,4,5,9],\"general_variables2\":-12345,\"general_variables3\":{\"general_variables\":\"test\",\"general_variables2\":[\"hat\",\"cat\",\"horse\",\"rat\"],\"general_variables3\":\"test\"}},\"general_variables3\":\"test\"}";
            

#endif
            
            tof_gather_string(example);
            //check_object();
        }while(1 == 1);
    }
    return 0;
}
