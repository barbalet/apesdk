/****************************************************************

 test_object_string.c

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

#include <stdio.h>

#include "../noble/noble.h"

n_int draw_error(n_constant_string error_text, n_constant_string location, n_int line_number)
{
    if (error_text)
    {
        printf("ERROR: %s @ %s %ld\n", (n_constant_string)error_text, location, line_number);
    }
    return -1;
}

void tof_gather(n_string file_in)
{
    n_file   local_file;
    n_file   *output_file;
    n_int    file_error = io_disk_read(&local_file, file_in);
    n_string file_out = io_string_copy(file_in);
    
    io_whitespace_json(&local_file);

    printf("%s --- \n", file_in);
    file_out[0]='2';
    if (file_error != -1)
    {
        n_object * returned_object = object_file_to_tree(&local_file);
        if (returned_object)
        {
            output_file = obj_json(returned_object);
            if (output_file)
            {
                file_error = io_disk_write(output_file, file_out);
                io_file_free(&output_file);
            }
            obj_free(&returned_object);
        }
    }
}

void tof_gather_string(n_string file_in)
{
    n_int    length = io_length(file_in, STRING_BLOCK_SIZE);
    n_file   local_file;

    local_file.data = (n_byte *)io_string_copy(file_in);
    local_file.location = 0;
    local_file.size = length;

    io_whitespace_json(&local_file);
    {
        n_object * returned_object = object_file_to_tree(&local_file);
        if (returned_object)
        {
            obj_free(&returned_object);
        }
    }
    memory_free((void**)&local_file.data);
}

#ifndef OBJECT_TEST

int main(int argc, const char * argv[])
{
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

    return 0;
}

#endif
