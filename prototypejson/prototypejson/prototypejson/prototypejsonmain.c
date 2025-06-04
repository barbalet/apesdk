/****************************************************************

 prototypejsonmain.c

 =============================================================

 Copyright 1996-2025 Tom Barbalet. All rights reserved.

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

#include <stdio.h>
#include "prototypejson.h"
#include "toolkit.h"

n_int draw_error( n_constant_string error_text, n_constant_string location, n_int line_number )
{
    printf( "ERROR: %s @ %s %ld\n", ( const n_string ) error_text, location, line_number );
    return -1;
}

void copyright_to_file(n_file * file)
{
    io_file_string(0, file, "Copyright 1996-2025 Tom Barbalet. All rights reserved.\n\n");
    
    io_file_string(0, file, "Permission is hereby granted, free of charge, to any person\n");
    io_file_string(0, file, "obtaining a copy of this software and associated documentation\n");
    io_file_string(0, file, "files (the \"Software\"), to deal in the Software without\n");
    io_file_string(0, file, "restriction, including without limitation the rights to use,\n");
    io_file_string(0, file, "copy, modify, merge, publish, distribute, sublicense, and/or\n");
    
    io_file_string(0, file, "copy, modify, merge, publish, distribute, sublicense, and/or\n");
    io_file_string(0, file, "sell copies of the Software, and to permit persons to whom the\n");
    io_file_string(0, file, "copy, modify, merge, publish, distribute, sublicense, and/or\n");
    io_file_string(0, file, "Software is furnished to do so, subject to the following\n");
    io_file_string(0, file, "conditions:\n\n");

    io_file_string(0, file, "The above copyright notice and this permission notice shall be\n");
    io_file_string(0, file, "included in all copies or substantial portions of the Software.\n\n");

    io_file_string(0, file, "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,\n");
    io_file_string(0, file, "EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES\n");
    io_file_string(0, file, "OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND\n");
    io_file_string(0, file, "NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT\n");
    io_file_string(0, file, "HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,\n");
    io_file_string(0, file, "WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING\n");
    io_file_string(0, file, "FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR\n");
    io_file_string(0, file, "OTHER DEALINGS IN THE SOFTWARE.\n\n");

    io_file_string(0, file, "This software is a continuing work of Tom Barbalet, begun on\n");
    io_file_string(0, file, "13 June 1996. No apes or cats were harmed in the writing of\n");
    io_file_string(0, file, "this software.\n\n");
}

void convert_simulated_file_definition(simulated_file_definition * sfd)
{
    n_uint loop = 0;
    do{
        simulated_file_definition* lsfd = &sfd[loop];
        loop++;
    }while (sfd[loop].value != 0L);
}

int main(int argc, const char * argv[]) {
    convert_simulated_file_definition(being_delta_json);
    return 0;
}
