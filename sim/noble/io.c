/****************************************************************

 io.c

 =============================================================

 Copyright 1996-2013 Tom Barbalet. All rights reserved.

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

/*! \file   io.c
 *  \brief  Covers the low level input and output relating to memory and files.
    In addition to memory and file handling, io has typically been a place holder
    for new functionality.
 */

/*NOBLEMAKE DEL=""*/
#include "noble.h"
/*NOBLEMAKE END=""*/
/*NOBLEMAKE VAR=""*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
/*NOBLEMAKE END=""*/

static n_int command_line_execution;
static n_int command_line_external_exit = 0;

void  io_command_line_execution_set(void)
{
    command_line_execution = 1;
}

n_int io_command_line_execution(void)
{
    return command_line_execution;
}

void io_entry_execution(n_int argc, n_string * argv)
{
    if (argv)
    {
        if ((argc == 2) && (argv[1][0] == 'c'))
        {
            io_command_line_execution_set();
        }
    }
}

static void io_aiff_uint(n_byte * buffer, n_uint value)
{
    buffer[0] = (value & 0xff000000) >> 24;
    buffer[1] = (value & 0x00ff0000) >> 16;
    buffer[2] = (value & 0x0000ff00) >> 8;
    buffer[3] = (value & 0x000000ff) >> 0;
}

void io_file_aiff_body(void * fptr, n_audio *samples, n_uint number_samples)
{
    fwrite(samples,number_samples,sizeof(n_audio),(FILE*)fptr);
}

static void io_aiff_header(n_byte * header)
{
    header[0] =  'F';
    header[1] =  'O';
    header[2] =  'R';
    header[3] =  'M';

    header[8]  = 'A';
    header[9]  = 'I';
    header[10] = 'F';
    header[11] = 'F';

    header[12] = 'C';
    header[13] = 'O';
    header[14] = 'M';
    header[15] = 'M';

    header[19] = 18;

    header[21] = 1;

    header[27] = 16;

    header[28] = 0x40;
    header[29] = 0x0e;
    header[30] = 0xac;
    header[31] = 0x44;

    header[38] = 'S';
    header[39] = 'S';
    header[40] = 'N';
    header[41] = 'D';
}

static n_uint io_aiff_total_size(n_uint total_samples)
{
    return 4 + 8 + 18 + 8 + (sizeof(n_audio) * total_samples) + 8;
}

static n_uint io_aiff_sound_size(n_uint total_samples)
{
    return (2 * total_samples) + 8;
}

void io_file_aiff_header(void * fptr, n_uint total_samples)
{
    n_byte header[54] = {0};
    io_aiff_header(header);
    io_aiff_uint(&header[4],  io_aiff_total_size(total_samples));
    io_aiff_uint(&header[22], total_samples);
    io_aiff_uint(&header[42], io_aiff_sound_size(total_samples));
    fwrite(header, 54, 1, (FILE*)fptr);
}

/**
 * This is a historical legacy function as all platforms now use memcpy. Although in the future this may change.
 * @param from pointer to copy from.
 * @param to pointer to copy to.
 * @param number the number of bytes to copy.
 */
void io_copy(n_byte * from, n_byte * to, n_uint number)
{
    memcpy(to, from, number);
}

/**
 * This is a historical legacy function as all platforms now use malloc. Although in the future this may change.
 * @param bytes number of bytes to allocate.
 * @return a void* pointer of the allocated bytes.
 */
void *	io_new(n_uint bytes)
{
    void *	tmp = 0L;
    if (bytes)
    {
        tmp = (void *) malloc(bytes);
    }
    return (tmp);
}

/**
 * This is a historical legacy function as all platforms now use free. Although in the future this may change.
 * @param ptr the void * pointer to be freed. Should really be a void ** to catch the 0L-ing.
 */
void io_free(void ** ptr)
{
    if (*ptr != 0L)
    {
        free(*ptr);
        *ptr = 0L;
    }
}

/**
 * This is allocates a range of memory depending on availability.
 * @param memory_min the minimum possible allocated memory before returning 0L.
 * @param memory_allocated the starting value for memory size and returning the actual size.
 * @return a void* pointer of the allocated bytes.
 */
void * io_new_range(n_uint memory_min, n_uint *memory_allocated)
{
    void * memory_buffer = 0L;
    do
    {
        memory_buffer = (void *) malloc(*memory_allocated);
        if (memory_buffer == 0L)
        {
            *memory_allocated = ((*memory_allocated) * 3) >> 2;
        }
    }
    while((memory_buffer == 0L) && ((*memory_allocated)>memory_min));
    return memory_buffer;
}

/**
 * Moves the string to lower case for the length shown.
 * @param value the string with the case to be lowered.
 * @param length the number of bytes to be lowered.
 * @return a void* pointer of the allocated bytes.
 */
void io_lower(n_string value, n_int length)
{
    n_int loop = 0;
    while (loop < length)
    {
        IO_LOWER_CHAR(value[loop]);
        loop++;
    }
}

/**
 * Allocates a new file.
 * @return a 4k worth of data file pointer.
 */
n_file * io_file_new(void)
{
    n_file * output = io_new(sizeof(n_file));
    if (output == 0L)
    {
        return 0L;
    }
    output->size = 4096;
    output->data = io_new(4096);
    if (output->data == 0L)
    {
        io_free((void **)&output);
        return 0L;
    }
    output->location = 0;
    return output;
}


/**
 * Frees the file pointer
 * @param file the pointer to be freed.
 */
void io_file_free(n_file ** file)
{
    if (file != 0L)
    {
        io_free((void **)&((*file)->data));
    }
    io_free((void **)file);
}

void io_int_to_bytes(n_int value, n_byte * bytes)
{
    io_copy((n_byte *)&value, bytes, sizeof(n_int));
}

n_int io_bytes_to_int(n_byte * bytes)
{
    /*n_uint unsigned_value;*/
    n_int return_value;
    
    io_copy(bytes, (n_byte *)&return_value, sizeof(n_int));
    return return_value;
}

/**
 * Reads a file from disk.
 * @param local_file the pointer to the n_file data that will have the file stored in it.
 * @param file_name the name of the file to be read.
 * @return FILE_ERROR if there is a problem and FILE_OKAY if it is successful.
 */
n_int io_disk_read(n_file * local_file, n_string file_name)
{
#ifndef _WIN32
    FILE * in_file = fopen(file_name,"rb");
#else
    FILE * in_file = 0L;
    fopen_s(&in_file,file_name,"rb");
#endif
    if (in_file == 0L)
    {
        return SHOW_ERROR("File does not exist");
    }
    while (!feof(in_file))
    {
        n_byte local_char;
        if (fread(&local_char,1,1,in_file)>0)
        {
            if (!feof(in_file))
            {
                n_int error = io_file_write(local_file, local_char);
                if (error != 0)
                {
                    return FILE_ERROR;
                }
            }
        }
    }
    fclose(in_file);
    return FILE_OKAY;
}

/**
 * Writes a file to disk.
 * @param local_file the pointer to the n_file data that is written to disk.
 * @param file_name the name of the file to be written.
 * @return FILE_ERROR if there is a problem and FILE_OKAY if it is successful.
 */
n_int io_disk_write(n_file * local_file, n_string file_name)
{
    n_uint written_length;
    FILE * out_file = 0L;
#ifndef _WIN32
    out_file = fopen(file_name,"wb");
#else
    fopen_s(&out_file,file_name,"wb");
#endif
    if (out_file == 0L)
    {
        return SHOW_ERROR("Error opening file to write");
    }
    if (local_file->data == 0L)
    {
        return SHOW_ERROR("No data in file to be written");
    }

    written_length = fwrite(local_file->data,1,local_file->location, out_file);

    if (fclose(out_file) != 0)
    {
        return SHOW_ERROR("File could not be closed");
    }

    if (written_length != local_file->location)
    {
        return SHOW_ERROR("File did not complete write");
    }
    return FILE_OKAY;
}

/**
 * Appends a file to disk.
 * @param local_file the pointer to the n_file data that is written to disk.
 * @param file_name the name of the file to be appended.
 * @return FILE_ERROR if there is a problem and FILE_OKAY if it is successful.
 */
n_int io_disk_check(n_constant_string file_name)
{
#ifndef _WIN32
    FILE * local_check_file = fopen(file_name,"rb");
#else
    FILE * local_check_file = 0L;

    fopen_s(&local_check_file,file_name,"rb");
#endif

    if (local_check_file == 0L)
        return 0;

    if (fclose(local_check_file) != 0)
    {
        return 0;
    }
    return 1;
}

/**
 * Reads binary data from the file pointer.
 * @param fil the pointer to the n_file data that is read from.
 * @param local_byte the single byte.
 * @return FILE_ERROR if there is a problem and FILE_OKAY if it is successful.
 */
n_int io_read_bin(n_file * fil, n_byte * local_byte)
{
    n_uint	file_location = fil -> location;
    if (file_location >= (fil -> size))
    {
        return -1;
    }

    *local_byte = fil -> data[file_location];
    fil->location = (file_location + 1);
    return 0;
}

/**
 * Reads a character - white-space and comments have already been removed
 * @param fil the pointer to the n_file data that is read from.
 * @return CHAR_EOF if there is a problem and the byte value if it is successful.
 */
n_byte	io_read(n_file * fil)
{
    n_byte val = 0;
    if (io_read_bin(fil, &val) == -1)
    {
        return(CHAR_EOF);
    }
    return (val);
}

/**
 * Converts a tab delimited file to a series of string pointers
 * @param tab_file the pointer to the n_file data that is read from.
 * @param size_value the pointer to the total number.
 * @param row_value the pointer to the number of columns per row.
 * @return string pointers.
 */
n_string * io_tab_delimit_to_n_string_ptr(n_file * tab_file, n_int * size_value, n_int * row_value)
{
    n_int              string_point_location = 1;
    n_int              first_value = 0;
    n_uint             loop = 0;
    n_string          *string_point;
    n_int              local_row_value = 0;
    n_byte             resultant_value;

    if (tab_file->location != 0)
    {
        while (loop < tab_file->location)
        {
            resultant_value = tab_file->data[loop];
            if (IS_TAB(resultant_value) || IS_RETURN(resultant_value))
            {
                tab_file->data[loop] = 0;
                first_value = 1;
                if ((local_row_value == 0) && IS_RETURN(resultant_value))
                {
                    local_row_value = string_point_location;
                }
            }
            else
            {
                if (first_value)
                {
                    string_point_location++;
                    first_value = 0;
                }
            }
            loop ++;
        }
    }

    if (size_value != 0L)
    {
        *size_value = string_point_location;
    }

    if (row_value != 0L)
    {
        *row_value = local_row_value;
    }

    if (tab_file->location == 0)
    {
        return 0L;
    }

    if ((string_point = (n_string *)io_new(string_point_location*sizeof(n_string *))) == 0L)
    {
        return 0L;
    }

    string_point_location = 0;
    first_value = 0;
    loop = 0;

    string_point[string_point_location++] = (n_string)&(tab_file->data[0]);
    while (loop < tab_file->location)
    {
        resultant_value = tab_file->data[loop];
        if ((first_value) && (resultant_value != 0))
        {
            string_point[string_point_location++] = (n_string)&(tab_file->data[loop]);
            first_value = 0;
        }
        if (resultant_value == 0)
        {
            first_value = 1;
        }
        loop ++;
    }

    return string_point;
}

/**
 * Read a four byte value from n_file
 * @param fil the pointer to the n_file data that is read from.
 * @param actual_value the actual value read.
 * @param final_char the final character (after the last number).
 * @return number of characters read in condition of success and -1 in condition of failure.
 */
n_int io_read_byte4(n_file * fil, n_uint * actual_value, n_byte * final_char)
{
    n_uint  temp = 0;
    n_int   ten_power_place = 0;
    while (1)
    {
        n_byte  value = io_read(fil);
        n_uint mod_ten;
        if (!ASCII_NUMBER(value))
        {
            *actual_value = temp;
            *final_char = value;
            return ten_power_place;
        }
        mod_ten = value - '0';
        if ((temp == 429496729) && (mod_ten > 5))
        {
            return -1;
        }
        if (temp > 429496729)
        {
            return -1;
        }
        temp = (temp * 10) + mod_ten;
        ten_power_place++;
    }
}

/**
 * Read a number from a string.
 * @param number_string the string to be read from.
 * @param actual_value the actual value read.
 * @param decimal_divisor the value required to provide a decimal version.
 * @return number of characters read in condition of success and -1 in condition of failure.
 */
n_int io_number(n_string number_string, n_int * actual_value, n_int * decimal_divisor)
{
    n_uint  temp = 0;
    n_int   divisor = 0;
    n_int   ten_power_place = 0;
    n_int   string_point = 0;
    n_byte  negative = 0;

    if (number_string == 0L) return -1;
    if (number_string[0] == 0) return -1;

    if (number_string[0] == '-')
    {
        negative = 1;
        string_point++;
    }

    while (1)
    {
        n_byte  value = number_string[string_point++];
        n_uint mod_ten;
        if (value == 0)
        {
            n_int   translate;

            if (negative == 1)
            {
                translate = 0 - temp;
            }
            else
            {
                translate = temp;
            }

            *actual_value = translate;

            if (divisor > 0)
            {
                divisor--;
            }

            *decimal_divisor = divisor;
            return ten_power_place;
        }
        if (value == '.')
        {
            if (divisor != 0)
            {
                return -1;
            }
            divisor = 1;
        }
        else
        {
            if (!ASCII_NUMBER(value))
            {
                return -1;
            }

            mod_ten = value - '0';
            if (temp == 214748364)
            {
                if (negative == 1)
                {
                    if (mod_ten > 8)
                    {
                        return -1;
                    }
                }
                else
                {
                    if (mod_ten > 7)
                    {
                        return -1;
                    }
                }
            }
            if (temp > 214748364)
            {
                return -1;
            }
            if (divisor != 0)
            {
                divisor++;
            }
            temp = (temp * 10) + mod_ten;
            ten_power_place++;
        }
    }
}

#define	ASCII_WHITESPACE(num) ((((num)>8)&&((num)<14))||((num)==32))

/**
 * Removes the whitespace from the initial file - CRs, LFs, tabs and spaces.
 * @param input the file pointer that will have the white space removed.
 */
void io_whitespace(n_file * input)
{
    n_uint	loop = 0, out_loop = 0;
    n_uint	end_loop = input->size;
    n_byte	*local_data = input->data;
    while(loop<end_loop)
    {
        n_byte	temp = local_data[loop++];

        if((temp == '/') && (loop != end_loop))
        {
            n_byte	check_twice[2]= {'/', 0};
            check_twice[1] = local_data[loop++];
            if(check_twice[1] != '*')
            {
                local_data[out_loop++] = '/';
                if(ASCII_WHITESPACE(check_twice[1]) == 0)
                {
                    local_data[out_loop++] = check_twice[1];
                }
            }
            else
            {
                check_twice[0] = 0;
                check_twice[1] = 0;
                do
                {
                    check_twice[0] = check_twice[1];
                    check_twice[1] = local_data[loop++];
                }
                while((loop != end_loop) && !((check_twice[0]=='*')&&(check_twice[1]=='/')));
            }
        }
        else if(ASCII_WHITESPACE(temp) == 0)
        {
            local_data[out_loop++] = temp;
        }
    }
    loop = out_loop;
    while (loop < end_loop)
    {
        local_data[loop++] = 0;
    }
    input->size = out_loop;
}

/**
 This is a dynamic write to file function which will increase the file size and
 allocated a larger data buffer if the original end of the file is reached. It
 is very useful for a number of dynamic file applications through the simulation.
 @param fil The file data to be written to.
 @param byte The byte/character to be written.
 @return Whether the parsing was successful or -1 on failure.
 */
n_int io_file_write(n_file * fil, n_byte byte)
{
    n_uint local_size = fil->size;
    if((fil->location + 1) == local_size)
    {
        /* This logic had to be changed for large file handling.*/
        n_uint	temp_size;
        n_byte *temp_data;
        if (local_size <= (256*1024))
        {
            temp_size = local_size * 4;
        }
        else
        {
            if (local_size <= (512*1024*1024))
            {
                temp_size = local_size * 2;
            }
            else
            {
                temp_size = (local_size * 3) >> 1;
            }
        }
        temp_data = io_new(temp_size);
        if (temp_data == 0L)
        {
            return(SHOW_ERROR("Attempted file overwrite"));
        }
        io_copy(fil->data, temp_data, local_size);
        io_free((void **)&(fil->data));
        fil->data = temp_data;
        fil->size = temp_size;
    }
    fil->data[fil->location++] = byte;
    return (FILE_OKAY);
}

/* Memory saving */
void io_file_reused(n_file * fil)
{
    io_erase(fil->data, fil->size);
    fil->location = 0;
}

/*
  This is too slow, consider:

 n_uint io_length(n_string s) {
	n_string start = s;
	while(*s)s++;
	return s - start;
 }
*/

/* this is used for finding the actual length of fixed length strings, max length is enforced */
n_int io_length(n_string value, n_int max)
{
    n_int return_length = -1;
    if (max < 1)
    {
        return -1;
    }
    do
    {
        return_length++;
    }
    while ((value[return_length] != 0) && (return_length < max));
    return return_length;
}

/*

 These are too slow too.

 Consider:

return <0 if s<t, 0 if s==t, >0 if s>t


n_int io_find(n_string s, n_string t)
{
	for ( ; *s == *t; s++, t++)
		if (*s == '\0')
			return 0;
	return *s - *t;
}

 */

n_int io_find(n_string check, n_int from, n_int max, n_string value_find, n_int value_find_length)
{
    n_int  loop = from;
    n_int  check_length = 0;
    while (loop < max)
    {
        if (check[loop] == value_find[check_length])
        {
            check_length++;
            if (check_length == value_find_length)
            {
                return loop + 1;
            }
        }
        else
        {
            check_length = 0;
        }
        loop ++;
    }
    return -1;
}

/* writes a string, adding a new line if required in the OS correct format */
#define FILE_MACRO_WRITE(ch)		if(io_file_write(fil,(ch)) == -1) return -1

n_int io_write(n_file * fil, n_constant_string ch, n_byte new_line)
{
    if (ch[0] != 0)
    {
        n_uint	lp = 0;
        while (ch[lp] != 0)
        {
            FILE_MACRO_WRITE(ch[lp++]);
        }
    }
    if (new_line&1)
    {
#ifndef	_WIN32
        FILE_MACRO_WRITE(10);
#else
        FILE_MACRO_WRITE(13);
        FILE_MACRO_WRITE(10);
#endif
    }
    if (new_line&2)
    {
        FILE_MACRO_WRITE(9);
    }
    return (FILE_OKAY);
}

/* writes a 16-bit or 8-bit number with an end terminator(,/;) and new line if required */
/* n_int error */
n_int io_writenumber(n_file * fil, n_int loc_val, n_uint numer, n_uint denom)
{
    n_byte	number_buffer[14] = {0};
    n_byte	negative;
    n_byte	decimal = 0;
    n_uint	positive_number;
    n_int	location = 12;
    if(loc_val < 0)
    {
        negative = 1;
        positive_number = 0 - loc_val;
    }
    else
    {
        negative = 0;
        positive_number = loc_val;
    }

    if(denom != 0)
    {
        n_uint roll_over = positive_number;

        positive_number = positive_number * numer * 100;

        if(positive_number < roll_over)
            return -1;

        positive_number = positive_number / denom;
        decimal = 1;
    }

    do
    {
        number_buffer[location --] = (n_byte)((positive_number % 10) + '0');
        positive_number = positive_number / 10;
        if(decimal && location == 10)
        {
            number_buffer[location --] = '.';
            if(positive_number == 0)
                number_buffer[location --] = '0';
        }
    }
    while((positive_number>0) || (decimal && (location > 9)));
    if(negative)
        number_buffer[location] = '-';
    else
        location++;
    return io_write(fil, (n_string)&number_buffer[location], 0);
}

n_int io_writenum(n_file * fil, n_int loc_val, n_byte ekind, n_byte new_line)
{
    n_int	return_val = io_writenumber(fil, loc_val,1,0);
    n_byte	ekind_buffer[2] = {0};
    if(return_val!=FILE_OKAY)
        return return_val;
    ekind_buffer[0] = ekind;

    return io_write(fil, (n_string)ekind_buffer, new_line);
}

/* find the variable command */
n_int io_command(n_file * fil, const noble_file_entry * commands)
{
    n_byte   found_text[6];
    n_byte * commands_bytes = (n_byte *) commands[0].characters;

    n_byte2 lp = 0;

    found_text[0] = io_read(fil);
    if (found_text[0] == 0)
        return (FILE_EOF);
    found_text[1] = io_read(fil);
    if (found_text[0] == '}' && found_text[1] == ';')
        return (FILE_END_INCLUSION);
    found_text[2] = io_read(fil);
    found_text[3] = io_read(fil);
    found_text[4] = io_read(fil);
    found_text[5] = io_read(fil);

    while((commands_bytes[0] != 0) || (commands_bytes[1] != 0) ||
            (commands_bytes[2] != 0) || (commands_bytes[3] != 0) ||
            (commands_bytes[4] != 0) || (commands_bytes[5] != 0))
    {
        commands_bytes = (n_byte *) commands[lp].characters;

        if (((commands_bytes[0] == found_text[0]) && (commands_bytes[1] == found_text[1])) &&
                ((commands_bytes[2] == found_text[2]) && (commands_bytes[3] == found_text[3])) &&
                ((commands_bytes[4] == found_text[4]) && (commands_bytes[5] == found_text[5])))
        {
            return (lp);
        }
        lp ++;
    }
    return -1;
}

#define	FILE_MACRO_CONCLUSION(ch)	(((comman_req==1) && (ch) == ',') || \
									 ((comman_req==0) && (ch) == ';'))

/* after the variable command, read the kind of data in specified through the commands array */
/* n_int error */
n_int io_read_data(n_file * fil, n_byte2 command, n_byte * data_read)
{
    n_int  comman_req = ((command & FILE_CONTINUATION) == FILE_CONTINUATION);
    n_byte type_from_command = (n_byte)FILE_TYPE(command);

    if (type_from_command == FILE_TYPE_PACKED)
    {
        n_uint loop = 0;
        n_byte  buffer[5]= {0};
        n_uint output_val;
        n_byte  num_char;

        if (data_read == 0L)
        {
            return (FILE_OKAY);
        }

        while (loop < PACKED_DATA_BLOCK)
        {
            buffer[0] = io_read(fil);
            buffer[1] = io_read(fil);
            buffer[2] = io_read(fil);

            output_val = (buffer[0]-65);
            output_val += (buffer[1]-65) * 41;
            output_val += (buffer[2]-65) * 41 * 41;

            data_read[loop++] = (output_val >> 0) & 255;
            data_read[loop++] = (output_val >> 8) & 255;
        }
        num_char  = io_read(fil);

        if (FILE_MACRO_CONCLUSION(num_char))
            return (FILE_OKAY);
        return SHOW_ERROR("Packed ends incorrectly");
    }

    if ((type_from_command == FILE_TYPE_BYTE) ||
            (type_from_command == FILE_TYPE_BYTE_EXT) ||
            (type_from_command == FILE_TYPE_BYTE2))
    {
        n_uint	number = 0;
        n_byte	num_char;
        n_int   response_code = io_read_byte4(fil, &number, &num_char);

        if (response_code == 0)
            return SHOW_ERROR("Expected number not found");
        if (response_code < 0)
            return SHOW_ERROR("Expected number too big");

        if ((type_from_command == FILE_TYPE_BYTE) || (type_from_command == FILE_TYPE_BYTE_EXT))
        {
            if (number > 0x000000ff)
                return SHOW_ERROR("Expected byte too big");
            data_read[0] = (n_byte) number;
        }

        if (type_from_command == FILE_TYPE_BYTE2)
        {
            n_byte2	* data_read2 = (n_byte2 *)data_read;
            if (number > 0x0000ffff)
                return SHOW_ERROR("Expected two byte too big");
            data_read2[0] = (n_byte2) number;
        }

        if (FILE_MACRO_CONCLUSION(num_char))
            return (FILE_OKAY);
        return SHOW_ERROR("Number ends incorrectly");
    }
    return SHOW_ERROR("Type not found");
}


/**
 @discussion This function takes a file and parses the file date with the commands
 shown into the output data. This function is extremely powerful because it allows
 all different kinds of format-rich text data to be converted into binary
 information based on the command information passed in to this function too.
 @param fil The file data to be parsed.
 @param data The resultant output data.
 @param commands The commands used to parse the output data.
 @return Whether the parsing was successful or -1 on failure.
 */
n_int	io_read_buff(n_file * fil, n_byte * data, const noble_file_entry * commands)
{

    n_int	inclusion_number = 0xffff;
    n_int	result_number = 0;
    do
    {
        result_number = io_command(fil, commands);
        if(result_number == -1)
            return SHOW_ERROR("Unknown command");

        if (result_number > 0x00ff)
        {
            if (result_number != FILE_END_INCLUSION)
                return (result_number);
        }
        else
        {
            n_byte  com_inclusion = FILE_INCL(commands[result_number].incl_kind);
            n_byte  com_kind      = FILE_KIND(commands[result_number].incl_kind);

            n_byte  com_number_of = commands[result_number].number;
            n_byte2 com_location  = commands[result_number].location;

            n_uint	loop = 0;
            n_byte *local_data = &data[com_location];

            if (inclusion_number == 0xffff)
            {
                if (commands[result_number].characters[5] != '{')
                    return SHOW_ERROR("{ expected in the file");
                inclusion_number = com_inclusion;
            }
            if (inclusion_number != com_inclusion)
                return SHOW_ERROR("Wrong start in file");

            while (loop < com_number_of)
            {
                n_byte	local_kind = com_kind;
                if ((loop + 1) != com_number_of)
                    local_kind |= FILE_CONTINUATION;
                if (io_read_data(fil, local_kind, local_data) != FILE_OKAY)
                    return (FILE_ERROR);
                if (com_kind == FILE_TYPE_PACKED)
                {
                    local_data = &local_data[ PACKED_DATA_BLOCK ];
                }
                else
                {
                    if (com_kind == FILE_TYPE_BYTE_EXT)
                    {
                        local_data = &local_data[ 1 ];
                    }
                    else
                    {
                        local_data = &local_data[ com_kind ];
                    }
                }
                loop++;
            }
        }
    }
    while (result_number != FILE_END_INCLUSION);
    return (inclusion_number);
}

#define	IO_CHECK_ERROR(cnd)             \
    {                                   \
        n_int	out_cnd = cnd;          \
        if ( (out_cnd) != FILE_OKAY)	\
            return	out_cnd;			\
    }
/**
 This function takes a block of data and various format-rules (through the
 commands) and applies these rules to produce an output datafile. In many
 regards this can be thought of as the inverse of io_read_buff.
 @param fil The file data that is produced.
 @param data The resultant output data.
 @param commands The commands used to parse data to the output file.
 @param command_num Output the command number block of commands.
 @param func Allows a file specific function to be injected to add additional format requirments.
 @return Whether the parsing was successful.
 */
n_int io_write_buff(n_file * fil, void * data, const noble_file_entry * commands, n_byte command_num, n_file_specific * func)
{
    if (command_num == FILE_COPYRIGHT)
    {
        n_string *fluff = (n_string *) data;

        IO_CHECK_ERROR(io_write(fil, "/*", 3));
        IO_CHECK_ERROR(io_write(fil, fluff[0], 0));
        IO_CHECK_ERROR(io_write(fil, fluff[1], 3));

        IO_CHECK_ERROR(io_write(fil, fluff[2], 0));
        IO_CHECK_ERROR(io_write(fil, fluff[3], 0));
        IO_CHECK_ERROR(io_write(fil, fluff[4], 1));
        IO_CHECK_ERROR(io_write(fil, "*/", 1));
        IO_CHECK_ERROR(io_write(fil, "", 1));

        return (FILE_OKAY);
    }
    {
        const n_byte	*commands_bytes;
        n_byte          writeout_commands[7]= {0};
        n_uint	offset = 0;
        n_int	release = FILE_ERROR;
        do
        {
            commands_bytes = commands[offset].characters;
            if ((commands_bytes[0] == 0) && (commands_bytes[1] == 0) &&
                    (commands_bytes[2] == 0) && (commands_bytes[3] == 0) &&
                    (commands_bytes[4] == 0) && (commands_bytes[5] == 0))
            {
                return SHOW_ERROR("File command not found");
            }

            if (FILE_INCL(commands[offset].incl_kind) == command_num)
                release = FILE_OKAY;
            else
                offset++;
        }
        while (release == FILE_ERROR);
        writeout_commands[0] = commands_bytes[0];
        writeout_commands[1] = commands_bytes[1];
        writeout_commands[2] = commands_bytes[2];
        writeout_commands[3] = commands_bytes[3];
        writeout_commands[4] = commands_bytes[4];
        writeout_commands[5] = commands_bytes[5];

        IO_CHECK_ERROR(io_write(fil, (n_string)writeout_commands, 3));
        release = FILE_ERROR;
        offset++;
        do
        {
            commands_bytes = commands[offset].characters;

            if ((commands_bytes[0] == 0) && (commands_bytes[1] == 0) &&
                    (commands_bytes[2] == 0) && (commands_bytes[3] == 0) &&
                    (commands_bytes[4] == 0) && (commands_bytes[5] == 0))
                release = FILE_OKAY;

            if (FILE_INCL(commands[offset].incl_kind) != command_num)
                release = FILE_OKAY;

            if (release == FILE_ERROR)
            {
                n_uint	loop = 0;
                n_byte	data_type   = FILE_KIND(commands[offset].incl_kind);
                n_uint end_loop    = commands[offset].number;
                n_byte2	data_offset = commands[offset].location;
                n_int	right_ending  = (FILE_INCL(commands[offset+1].incl_kind) != command_num);

                right_ending |= ((commands[offset+1].characters[0] == 0) && (commands[offset+1].characters[1] == 0) &&
                                 (commands[offset+1].characters[2] == 0) && (commands[offset+1].characters[3] == 0) &&
                                 (commands[offset+1].characters[4] == 0) && (commands[offset+1].characters[5] == 0));
                right_ending = 3 - (right_ending * 2);
#if 0
                if (data_type == FILE_TYPE_PACKED)
                {
                    if (unpack_data != 0L)
                    {
                        n_byte  buffer[6]= {0};
                        n_uint output_val;
                        n_byte  *local_unpack_data = unpack_data;
                        writeout_commands[0] = commands_bytes[0];
                        writeout_commands[1] = commands_bytes[1];
                        writeout_commands[2] = commands_bytes[2];
                        writeout_commands[3] = commands_bytes[3];
                        writeout_commands[4] = commands_bytes[4];
                        writeout_commands[5] = commands_bytes[5];
                        IO_CHECK_ERROR(io_write(fil, (n_string)writeout_commands, (n_byte)right_ending));
                        while (loop < end_loop)
                        {
                            n_uint data_loop = 0;
                            while (data_loop < PACKED_DATA_BLOCK)
                            {
                                output_val  = local_unpack_data[data_loop++];
                                output_val |= local_unpack_data[data_loop++] << 8;
                                buffer[0] = (output_val % 41) + 65;
                                output_val /= 41;
                                buffer[1] = (output_val % 41) + 65;
                                output_val /= 41;
                                buffer[2] = (output_val % 41) + 65;

                                if ((data_loop % 80 ) == 0)
                                {
                                    IO_CHECK_ERROR(io_write(fil, (n_string) buffer, (n_byte)right_ending));
                                }
                                else
                                {
                                    IO_CHECK_ERROR(io_write(fil, (n_string) buffer, 0));
                                }
                            }
                            local_unpack_data = &local_unpack_data[PACKED_DATA_BLOCK];
                            loop++;
                            if (loop == end_loop)
                            {
                                IO_CHECK_ERROR(io_write(fil, ";", (n_byte)right_ending));
                            }
                            else
                            {
                                IO_CHECK_ERROR(io_write(fil, ",", (n_byte)right_ending));
                            }
                        }
                    }
                }
                else
#endif
                {
                    n_byte *byte_data = (n_byte *)data;
                    if (data_type == FILE_TYPE_BYTE_EXT)
                    {
                        n_string_block block_code= {0};
                        if (func != 0) (*func)((n_string)block_code, byte_data);
                        IO_CHECK_ERROR(io_write(fil, "", 1));
                        IO_CHECK_ERROR(io_write(fil, "", 2));
                        IO_CHECK_ERROR(io_write(fil, "/* ", 0));

                        IO_CHECK_ERROR(io_writenumber(fil, loop, 1, 0));
                        IO_CHECK_ERROR(io_write(fil, "", 2));
                        IO_CHECK_ERROR(io_write(fil, block_code, 0));
                        IO_CHECK_ERROR(io_write(fil, " */", 2));
                    }

                    writeout_commands[0] = commands_bytes[0];
                    writeout_commands[1] = commands_bytes[1];
                    writeout_commands[2] = commands_bytes[2];
                    writeout_commands[3] = commands_bytes[3];
                    writeout_commands[4] = commands_bytes[4];
                    writeout_commands[5] = commands_bytes[5];

                    IO_CHECK_ERROR(io_write(fil, (n_string)writeout_commands, 0));
                    while (loop < end_loop)
                    {
                        n_byte2	num_write = 0;
                        switch (data_type)
                        {
                        case FILE_TYPE_BYTE_EXT:
                            if((loop != 0) && ((loop % 3) == 0) && (loop != 126))
                            {
                                n_string_block block_code = {0};
                                if (func != 0L) (*func)(block_code, &byte_data[data_offset + loop]);
                                IO_CHECK_ERROR(io_write(fil, "", 1));
                                IO_CHECK_ERROR(io_write(fil, "", 2));
                                IO_CHECK_ERROR(io_write(fil, "/* ", 0));
                                IO_CHECK_ERROR(io_writenumber(fil, loop, 1, 0));
                                IO_CHECK_ERROR(io_write(fil, "", 2));
                                IO_CHECK_ERROR(io_write(fil, (n_string)block_code, 0));
                                IO_CHECK_ERROR(io_write(fil, " */", 2));
                            }
                        case FILE_TYPE_BYTE:
                            num_write = byte_data[data_offset + loop];
                            break;
                        case FILE_TYPE_BYTE2:
                            num_write = ((n_byte2 *) & byte_data[data_offset + (loop * 2)])[0];
                            break;
                        }
                        loop++;

                        if (loop == end_loop)
                        {
                            IO_CHECK_ERROR(io_writenum(fil, num_write, ';', (n_byte)right_ending));
                        }
                        else
                        {
                            IO_CHECK_ERROR(io_writenum(fil, num_write, ',', 0));
                        }
                    }

                }
                offset++;
            }
        }
        while (release == FILE_ERROR);
        IO_CHECK_ERROR(io_write(fil, "};", 1));
        IO_CHECK_ERROR(io_write(fil, "", 1));
    }
    return (FILE_OKAY);
}

/* write the output data with the commands array and the required copyright "fluff" */
/* n_int error */
n_int io_write_csv(n_file * fil, n_byte * data, const noble_file_entry * commands, n_byte command_num, n_byte initial)
{
    if (command_num == FILE_COPYRIGHT)
    {
        return (FILE_OKAY);
    }
    {
        const n_byte * commands_bytes;
        n_uint	offset = 0;
        n_int	  release = FILE_ERROR;
        n_byte  first_entry = 1;
        n_byte  writeout_commands[6]= {0};
        do
        {
            commands_bytes = commands[offset].characters;
            if ((commands_bytes[0] == 0) && (commands_bytes[1] == 0) &&
                    (commands_bytes[2] == 0) && (commands_bytes[3] == 0) &&
                    (commands_bytes[4] == 0) && (commands_bytes[5] == 0))
                return SHOW_ERROR("File command not found");
            if (FILE_INCL(commands[offset].incl_kind) == command_num)
                release = FILE_OKAY;
            else
                offset++;
        }
        while (release == FILE_ERROR);

        release = FILE_ERROR;
        offset++;
        do
        {
            commands_bytes = commands[offset].characters;
            if ((commands_bytes[0] == 0) && (commands_bytes[1] == 0) &&
                    (commands_bytes[2] == 0) && (commands_bytes[3] == 0) &&
                    (commands_bytes[4] == 0) && (commands_bytes[5] == 0))
                release = FILE_OKAY;
            if (FILE_INCL(commands[offset].incl_kind) != command_num)
                release = FILE_OKAY;
            if (release == FILE_ERROR)
            {
                n_uint	loop = 0;
                n_byte	data_type   = FILE_KIND(commands[offset].incl_kind);
                n_uint end_loop    = commands[offset].number;
                n_byte2	data_offset = commands[offset].location;

                if (data_type != FILE_TYPE_PACKED)
                {
                    while (loop < end_loop)
                    {
                        n_byte2	num_write = 0;

                        if (first_entry == 0)
                        {
                            IO_CHECK_ERROR(io_write(fil, ",", 0));
                        }
                        else
                        {
                            first_entry = 0;
                        }

                        if (initial == 1)
                        {
                            writeout_commands[0] = commands_bytes[0];
                            writeout_commands[1] = commands_bytes[1];
                            writeout_commands[2] = commands_bytes[2];
                            writeout_commands[3] = commands_bytes[3];
                            writeout_commands[4] = commands_bytes[4];
                            writeout_commands[5] = commands_bytes[5];

                            IO_CHECK_ERROR(io_write(fil, (n_string)writeout_commands, 0));
                            IO_CHECK_ERROR(io_writenumber(fil, loop, 1, 0));
                        }
                        else
                        {
                            switch (data_type)
                            {
                            case FILE_TYPE_BYTE:
                            case FILE_TYPE_BYTE_EXT:
                                num_write = data[data_offset + loop];
                                break;
                            case FILE_TYPE_BYTE2:
                                num_write = ((n_byte2 *) & data[data_offset + (loop * 2)])[0];
                                break;
                            }
                            IO_CHECK_ERROR(io_writenumber(fil, num_write, 1, 0));
                        }
                        loop++;
                    }
                }
                offset++;
            }
        }
        while (release == FILE_ERROR);
        IO_CHECK_ERROR(io_write(fil, "", 1));
    }
    return (FILE_OKAY);
}

void io_erase(n_byte * buf_offscr, n_uint nestop)
{
    memset(buf_offscr, 0, nestop);
}

void io_search_file_format(const noble_file_entry * format, n_string compare)
{
    n_int              loop = 0;
    n_byte			   print_file_format_exit = 0;
    n_byte			   print_file_place_show_all = 0;
    noble_file_entry * place;
    if (compare == 0L)
    {
        print_file_place_show_all = 2;
    }
    else
    {
        n_byte leave_condition = 0;
        do
        {
            place = (noble_file_entry *)&format[loop++];
            if ((place->characters[0] != 0) && (place->characters[1] != 0) &&
                    (place->characters[2] != 0) && (place->characters[3] != 0) &&
                    (place->characters[4] != 0) && (place->characters[5] != 0))
            {
                if ((place->characters[0] == compare[0]) && (place->characters[1] == compare[1]) &&
                        (place->characters[2] == compare[2]) && (place->characters[3] == compare[3]) &&
                        (place->characters[4] == compare[4]) && (0 == compare[5]))
                {
                    leave_condition = 1;
                }

            }
            else
            {
                print_file_format_exit = 1;
            }
        }
        while((print_file_format_exit == 0) && (leave_condition == 0));

        if (print_file_format_exit == 1)
        {
            (void)SHOW_ERROR("String not found");
            return;
        }
        if(place->characters[5] =='{')
        {
            print_file_place_show_all = 1;
        }
        loop--;
    }
    do
    {
        place = (noble_file_entry *)&format[loop++];
        if ((place->characters[0] != 0) && (place->characters[1] != 0) &&
                (place->characters[2] != 0) && (place->characters[3] != 0) &&
                (place->characters[4] != 0) && (place->characters[5] != 0))
        {
            if (place->characters[5] == '{')
            {
                printf(" %c%c%c%c%c %s\n", place->characters[0],place->characters[1],place->characters[2],
                       place->characters[3],place->characters[4], place->what_is_it);
            }
            else
            {
                printf("  %c%c%c%c%c %s\n", place->characters[0],place->characters[1],place->characters[2],
                       place->characters[3],place->characters[4], place->what_is_it);
            }

            if (print_file_place_show_all == 0)
            {
                return;
            }
            if ((format[loop].characters[5]=='{') && (print_file_place_show_all == 1))
            {
                return;
            }
        }
        else
        {
            print_file_format_exit = 1;
        }
    }
    while(print_file_format_exit == 0);
}

void io_audit_file(const noble_file_entry * format, n_byte section_to_audit)
{
    n_uint      loop = 0;
    n_uint      being_counter = 0;
    n_byte     *local_characters;
    do
    {
        n_byte   local_incl_kind  = format[loop].incl_kind;
        n_byte   local_number     = format[loop].number;
        n_byte2  local_location   = format[loop].location;
        n_string_block   printout_characters = {0};
        local_characters = (n_byte*)format[loop].characters;
        if ((local_incl_kind & 0xF0) == section_to_audit)
        {
            n_byte local_type = local_incl_kind & 0x0F;

            if (local_type == FILE_TYPE_BYTE_EXT)
            {
                local_type = FILE_TYPE_BYTE;
            }

            if ((local_type == FILE_TYPE_BYTE) || (local_type == FILE_TYPE_BYTE2))
            {
                printout_characters[0] = local_characters[0];
                printout_characters[1] = local_characters[1];
                printout_characters[2] = local_characters[2];
                printout_characters[3] = local_characters[3];
                printout_characters[4] = local_characters[4];
                printout_characters[5] = local_characters[5];

                printf("%s \t %d * %d = %d bytes \t reported/actual/diff offset %d / %d / %d\n", printout_characters,
                       local_number, local_type, (local_number * local_type), (int)local_location, (int)being_counter, ((int)local_location - (int)being_counter));

                being_counter += (local_number * local_type);
            }
        }
        loop++;
    }
    while((local_characters[0] != 0) && (local_characters[1] != 0) &&
            (local_characters[2] != 0) && (local_characters[3] != 0));
}

noble_console_command * local_commands = 0L;


void io_three_string_combination(n_string output, n_string first, n_string second, n_string third, n_int count)
{
    n_int command_length = io_length(first, STRING_BLOCK_SIZE);
    n_int addition_length = io_length(second, STRING_BLOCK_SIZE);
    n_int total = count - (command_length + addition_length + 1);
    n_int loop2 = 0;
    n_int position = 0;
    
    io_string_write(output, " ", &position);
    io_string_write(output, first, &position);
    io_string_write(output, " ", &position);
    io_string_write(output, second, &position);
    while (loop2 < total)
    {
        io_string_write(output, " ", &position);
        loop2++;

    }
    io_string_write(output, third, &position);
}

void io_time_to_string(n_string value, n_int minutes, n_int days, n_int centuries)
{
    n_int military_time = (minutes % 60);
    n_int hours = (minutes/60);
    military_time += hours * 100;
    sprintf(value,"%4ld:%ld/%ld",military_time,days,centuries);
}

void io_offset(n_byte * start, n_byte * point, n_string text)
{
    printf("%s %ld\n", text, (n_int)(point - start));
}

void io_help_line(noble_console_command * specific, n_console_output output_function)
{
    n_string_block  string_line = {0};
    io_three_string_combination(string_line, specific->command, specific->addition, specific->help_information, 28);
    output_function(string_line);
}

n_int io_help(void * ptr, n_string response, n_console_output output_function)
{
    n_int loop = 0;
    n_int response_len = 0;
    n_int found = 0;

    if (response != 0L)
    {
        response_len = io_length(response, 1024);
    }

    if (response_len == 0)
    {
        output_function("Commands:");
    }

    do
    {
        if (local_commands[loop].function != 0L)
        {
            if ((local_commands[loop].help_information) && (local_commands[loop].help_information[0] != 0))
            {
                if (response_len == 0)
                {
                    io_help_line(&local_commands[loop], output_function);
                }
                else
                {
                    n_int command_len = io_length(local_commands[loop].command, 1024);
                    n_int count = io_find(response, 0, response_len, local_commands[loop].command, command_len);
                    if (count == command_len)
                    {
                        io_help_line(&local_commands[loop], output_function);
                        found = 1;
                    }
                }
            }
            loop++;
        }
    }
    while (local_commands[loop].function != 0L);
    if ((response_len != 0) && (found == 0))
    {
        (void)SHOW_ERROR("Command not found, type help for more information");
    }
    return 0;
}

n_int io_quit(void * ptr, n_string response, n_console_output output_function)
{
    return 1;
}

n_string io_console_entry_clean(n_string string, n_int length)
{
    return fgets(string, (int)length, stdin);
}

n_string io_console_entry(n_string string, n_int length)
{
    printf(">");
    return io_console_entry_clean(string, length);
}

void io_console_out(n_constant_string value)
{
    printf("%s\n", value);
    fflush(stdout);
}

void io_console_quit(void)
{
    command_line_external_exit = 1;
}

n_int io_console(void * ptr, noble_console_command * commands, n_console_input input_function, n_console_output output_function)
{
    n_string_block buffer;

    local_commands = commands;

    if ((input_function)(buffer, STRING_BLOCK_SIZE) != 0L)
    {
        n_int  loop = 0;
        n_int buffer_len = io_length(buffer, STRING_BLOCK_SIZE);

        if ((commands[0].command == 0L) && (commands[0].function == 0L))
        {
            return SHOW_ERROR("No commands provided");
        }

        /* captures linux, mac and windows line ending issue */
        if (IS_RETURN(buffer[buffer_len-1]))
        {
            buffer[buffer_len-1] = 0;
            buffer_len--;
        }
        if (IS_RETURN(buffer[buffer_len-1]))
        {
            buffer[buffer_len-1] = 0;
            buffer_len--;
        }

        if (buffer_len != 0)
        {
            do
            {
                n_int command_len = io_length(commands[loop].command, 1024);
                n_int count = io_find((n_string)buffer, 0, buffer_len, commands[loop].command, command_len);
                if (count != -1)
                {
                    n_int return_value;
                    n_console * function = commands[loop].function;
                    if (IS_SPACE(buffer[count]))
                    {
                        return_value = (*function)(ptr,(n_string)&buffer[count+1], output_function);
                        if (command_line_external_exit)
                        {
                            return 1;
                        }
                        return return_value;
                    }
                    else if (buffer[count] == 0)
                    {
                        return_value = (*function)(ptr,0L, output_function);
                        if (command_line_external_exit)
                        {
                            return 1;
                        }
                        return return_value;
                    }
                }
                loop++;
            }
            while ((commands[loop].command != 0L) && (commands[loop].function != 0L));

            (void)SHOW_ERROR("Command not found, type help for more information");

            return 0;
        }
        else
        {
            return 0;
        }
    }
    return SHOW_ERROR("Console failure");
}

void io_string_write(n_string dest, n_string insert, n_int * pos)
{
    n_int  loop = 0;
    n_byte character = 127;
    do {
        character = insert [loop++];
        if (character)
        {
            dest[*pos] = character;
            *(pos) += 1;
        }
    } while (character);
    dest[*pos] = 0;
}

n_int io_apescript_error(AE_ENUM value)
{
    n_int    loop = 0;
    AE_ENUM  local_enum;
    n_constant_string local_error;
    do
    {
        local_enum = apescript_errors[loop].enum_value;
        local_error = apescript_errors[loop].error_string;
        if (value == local_enum)
        {
            return SHOW_ERROR(local_error);
        }
        loop++;
    }
    while((local_enum != AE_NO_ERROR) && (local_error != 0L));

    return io_apescript_error(AE_UNKNOWN_ERROR);
}

#ifdef NOBLE_APE_ASSERT

void io_assert(n_string message, n_string file_loc, n_int line)
{
    printf("Assert: %s, %s, %ld\n", message, file_loc, line);
}

#endif

n_file * io_file_ready(n_int entry, n_file * file)
{
    if (entry == 1)
    {
        return 0L;
    }
    return file;
}

void io_file_cleanup(n_int * entry, n_file ** file)
{
    /* This setting to zero may be duplicated in at least one place
     but provides additional protection - it may not be needed following
     a case-by-case review */
    
    *entry = 0;
    
    if (*file)
    {
        io_file_free(file);
    }
}

void io_file_writeon(n_int * entry, n_file ** file, n_byte blocked_write)
{
    if (*entry == 0) return;
#ifndef COMMAND_LINE_DEBUG
    if (*file == 0L) /* io_file_reused */
    {
        *file = io_file_new();
    }
    if(*file == 0L)
    {
        (void)SHOW_ERROR("Could not set up special use file");
        return;
    }
    if (blocked_write)
    {
        *entry = 1;
    }
    else
    {
        *entry = 2;
    }
#endif
}

void io_file_writeoff(n_int * entry, n_file * file)
{
    if (*entry == 0) return;
#ifndef COMMAND_LINE_DEBUG
    if(file != 0L)
    {
        *entry = 0;
    }
#endif
}

void io_file_string(n_int entry, n_file * file, n_constant_string string)
{
    if (entry == 0) return;
    
    if((string != 0L)
#ifndef COMMAND_LINE_DEBUG
       && (file  != 0L)
#endif
       )
    {
#ifndef COMMAND_LINE_DEBUG
        io_write(file, string, 0);
#else
        printf("%s",string);
#endif
    }
}


