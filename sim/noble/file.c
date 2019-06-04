/****************************************************************

 file.c

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

/*! \file   n_file.c
 *  \brief  Covers the low level input and output relating to files.
 */

#include "noble.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>



#define CHAR_TAB                 (9)

/* this is v2 of the file parsing, v3 is soon to be implemented through the scripting interface */
#define CHAR_EOF                    0

#define IS_TAB(val)               ((val) == CHAR_TAB)
#define    IS_WHITE_HORIZON(val)      (IS_TAB(val)  || IS_SPACE(val))
#define    IS_WHITE_SPACE(val)          (IS_WHITE_HORIZON((val))||IS_RETURN((val)))
#define FILE_END_INCLUSION        0x0101
#define    FILE_TYPE(num)              ((num)&0x07)
#define    FILE_CONTINUATION          0x80



/**
 * Allocates a new file.
 * @return a 4k worth of data file pointer.
 */
n_file * io_file_new(void)
{
    n_file * output = memory_new(sizeof(n_file));
    if (output == 0L)
    {
        return 0L;
    }
    output->size = 4096;
    output->data = memory_new(4096);
    if (output->data == 0L)
    {
        memory_free((void **)&output);
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
        if ((*file)->data)
        {
            memory_free((void **)&((*file)->data));
        }
    }
    memory_free((void **)file);
}

void io_int_to_bytes(n_int value, n_byte * bytes)
{
    memory_copy((n_byte *)&value, bytes, sizeof(n_int));
}

n_int io_bytes_to_int(n_byte * bytes)
{
    /*n_uint unsigned_value;*/
    n_int return_value;
    
    memory_copy(bytes, (n_byte *)&return_value, sizeof(n_int));
    return return_value;
}

n_uint io_file_hash(n_file * local_file)
{
    n_uint hash = math_hash((n_byte *)&local_file->location, sizeof(n_uint));
    hash ^= math_hash((n_byte *)&local_file->size, sizeof(n_uint));
    hash ^= math_hash(local_file->data, local_file->size);
    return hash;
}

/**
 * Reads a file from disk.
 * @param local_file the pointer to the n_file data that will have the file stored in it.
 * @param file_name the name of the file to be read.
 * @return FILE_ERROR if there is a problem and FILE_OKAY if it is successful.
 */
static n_int io_disk_read_error(n_file * local_file, n_string file_name, n_byte error_show)
{
    n_uint file_size;
#ifndef _WIN32
    FILE * in_file = fopen(file_name,"rb");
#else
    FILE * in_file = 0L;
    fopen_s(&in_file,file_name,"rb");
#endif
    if (in_file == 0L)
    {
        if (error_show)
        {
            return SHOW_ERROR("File does not exist");
        }
        return -1;
    }
    
#if 1
    fseek(in_file, 0L, SEEK_END);
    file_size = ftell(in_file);
    fseek(in_file, 0L, SEEK_SET);
    
    memory_free((void**)&local_file->data);
    
    local_file->data = memory_new(file_size * 2);
    if (local_file->data == 0L)
    {
        if (error_show)
        {
            return SHOW_ERROR("File data could not be allocated");
        }
        return -1;
    }
    
    memory_erase(local_file->data, file_size * 2);
    
    fread(local_file->data, 1, file_size, in_file);
    
    local_file->size = file_size * 2;
    local_file->location = file_size;
    
#else
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
                    fclose(in_file);
                    return FILE_ERROR;
                }
            }
        }
    }
    
#endif
    fclose(in_file);
    return FILE_OKAY;
}

/**
 * Reads a file from disk.
 * @param local_file the pointer to the n_file data that will have the file stored in it.
 * @param file_name the name of the file to be read.
 * @return FILE_ERROR if there is a problem and FILE_OKAY if it is successful.
 */
n_int io_disk_read(n_file * local_file, n_string file_name)
{
    return io_disk_read_error(local_file, file_name, 1);
}

n_int io_disk_read_no_error(n_file * local_file, n_string file_name)
{
    return io_disk_read_error(local_file, file_name, 0);
}

/**
 * Writes a file to disk.
 * @param local_file the pointer to the n_file data that is written to disk.
 * @param file_name the name of the file to be written.
 * @return FILE_ERROR if there is a problem and FILE_OKAY if it is successful.
 */
n_int io_disk_write(n_file * local_file, n_constant_string file_name)
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
    n_uint    file_location = fil -> location;
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
static n_byte    io_read(n_file * fil)
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
    
    if ((string_point = (n_string *)memory_new(string_point_location*sizeof(n_string *))) == 0L)
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
 * Appends a file to disk.
 * @param local_file the pointer to the n_file data that is written to disk.
 * @param file_name the name of the file to be appended.
 * @return FILE_ERROR if there is a problem and FILE_OKAY if it is successful.
 */
static n_int io_disk_append(n_file * local_file, n_string file_name)
{
    n_uint written_length;
#ifndef _WIN32
    FILE * out_file = fopen(file_name,"a");
#else
    FILE * out_file = 0L;
    
    fopen_s(&out_file,file_name,"a");
#endif
    
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
 * Adds XML open to the named string.
 * @param file the pointer to the n_file data that is written.
 * @param name the string that is wrapped.
 * @return FILE_ERROR if there is a problem and FILE_OKAY if it is successful.
 */
static n_int io_file_xml_open(n_file * file, n_string name)
{
    if (io_write(file,"<", 0) == -1) return -1;
    if (io_write(file,name, 0) == -1) return -1;
    if (io_write(file,">", 1) == -1) return -1;
    return 0;
}

/**
 * Adds XML close to the named string.
 * @param file the pointer to the n_file data that is written.
 * @param name the string that is wrapped.
 * @return FILE_ERROR if there is a problem and FILE_OKAY if it is successful.
 */
static n_int io_file_xml_close(n_file * file, n_string name)
{
    if (io_write(file,"</", 0) == -1) return -1;
    if (io_write(file,name, 0) == -1) return -1;
    if (io_write(file,">", 1) == -1) return -1;
    return 0;
}

/**
 * Wraps a string with XML open and close
 * @param file the pointer to the n_file data that is written.
 * @param name the string that is the wrapper.
 * @param string the string that is wrapped.
 * @return FILE_ERROR if there is a problem and FILE_OKAY if it is successful.
 */
static n_int io_file_xml_string(n_file * file, n_string name, n_string string)
{
    if (io_file_xml_open(file,name) == -1) return -1;
    if (io_write(file,string, 0) == -1) return -1;
    if (io_file_xml_close(file,name) == -1) return -1;
    return 0;
}

/**
 * Wraps an integer with XML open and close
 * @param file the pointer to the n_file data that is written.
 * @param name the string that is the wrapper.
 * @param number the integer that is wrapped.
 * @return FILE_ERROR if there is a problem and FILE_OKAY if it is successful.
 */
static n_int io_file_xml_int(n_file * file, n_string name, n_int number)
{
    if (io_file_xml_open(file,name) == -1) return -1;
    if (io_writenumber(file, number, 1, 0) == -1) return -1;
    if (io_file_xml_close(file,name) == -1) return -1;
    return 0;
    
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


#define    ASCII_WHITESPACE(num) ((((num)>8)&&((num)<14))||((num)==32))

/**
 * Removes the whitespace from the initial file - CRs, LFs, tabs and spaces.
 * @param input the file pointer that will have the white space removed.
 */
void io_whitespace(n_file * input)
{
    n_uint    loop = 0, out_loop = 0;
    n_uint    end_loop = input->size;
    n_byte    *local_data = input->data;
    
    while(loop < end_loop)
    {
        n_byte    temp = local_data[loop++];
        if((temp == '/') && (loop != end_loop))
        {
            n_byte    check_twice[2]= {'/', 0};
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

void io_whitespace_json(n_file * input)
{
    n_uint    loop = 0, out_loop = 0;
    n_uint    end_loop = input->size;
    n_byte    *local_data = input->data;
    n_int     inside_string = 0;
    
    while(loop < end_loop)
    {
        n_byte    temp = local_data[loop++];
        if (temp == '"')
        {
            inside_string ^= 1;
            local_data[out_loop++] = temp;
        }
        else if ((ASCII_WHITESPACE(temp) == 0) || inside_string)
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
    input->location = 0;
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
        n_uint    temp_size;
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
        temp_data = memory_new(temp_size);
        if (temp_data == 0L)
        {
            return(SHOW_ERROR("Attempted file overwrite"));
        }
        memory_copy(fil->data, temp_data, local_size);
        memory_free((void **)&(fil->data));
        fil->data = temp_data;
        fil->size = temp_size;
    }
    fil->data[fil->location++] = byte;
    return (FILE_OKAY);
}

/* Memory saving */
void io_file_reused(n_file * fil)
{
    memory_erase(fil->data, fil->size);
    fil->location = 0;
}



/* writes a string, adding a new line if required in the OS correct format */
#define FILE_MACRO_WRITE(ch)        if(io_file_write(fil,(ch)) == -1) return -1

n_int io_write(n_file * fil, n_constant_string ch, n_byte new_line)
{
    if (ch[0] != 0)
    {
        n_uint    lp = 0;
        while (ch[lp] != 0)
        {
            FILE_MACRO_WRITE(ch[lp++]);
        }
    }
    if (new_line&1)
    {
#ifdef    _WIN32
        FILE_MACRO_WRITE(13);
#endif
        FILE_MACRO_WRITE(10);
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
    n_byte    number_buffer[14] = {0};
    n_byte    negative;
    n_byte    decimal = 0;
    n_uint    positive_number;
    n_int    location = 12;
    if(loc_val < 0)
    {
        negative = 1;
        positive_number = (n_uint)(0 - loc_val);
    }
    else
    {
        negative = 0;
        positive_number = (n_uint)(loc_val);
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
    n_int    return_val = io_writenumber(fil, loc_val,1,0);
    n_byte    ekind_buffer[2] = {0};
    if(return_val!=FILE_OKAY)
        return return_val;
    ekind_buffer[0] = ekind;
    
    return io_write(fil, (n_string)ekind_buffer, new_line);
}

/* find the variable command */
n_int io_command(n_file * fil, const noble_file_entry * commands)
{
    n_byte   found_text[7] = {0};
    n_byte * commands_bytes = (n_byte *) commands[0].characters;
    
    n_byte2 lp = 0;
    
    found_text[0] = io_read(fil);
    if (found_text[0] == 0)
    {
        return (FILE_EOF);
    }
    found_text[1] = io_read(fil);
    if (found_text[0] == '}' && found_text[1] == ';')
    {
        return (FILE_END_INCLUSION);
    }
    found_text[2] = io_read(fil);
    found_text[3] = io_read(fil);
    found_text[4] = io_read(fil);
    found_text[5] = io_read(fil);
    
    while (POPULATED(commands_bytes))
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
    
    /*io_output_contents(fil);*/
    
    printf("String length : %ld\n", io_length((n_string)fil->data, 0xffff));
    printf("Actual size : %lu\n", fil->size);
    printf("String location : %lu\n", fil->location);
    
    printf("Failed text %s\n", found_text);
    
    printf("Failed text %d\n", found_text[0]);
    printf("Failed text %d\n", found_text[1]);
    printf("Failed text %d\n", found_text[2]);
    printf("Failed text %d\n", found_text[3]);
    printf("Failed text %d\n", found_text[4]);
    printf("Failed text %d\n", found_text[5]);
    
    NA_ASSERT(0, "Failed here");
    
    return SHOW_ERROR((n_constant_string)found_text);
}

/* find the largest size data unit to handle the file copying to data structures */
n_uint io_find_size_data(noble_file_entry * commands)
{
    n_uint   max_entry = 0;
    n_int   lp = 1;
    n_byte  last_incl = FILE_INCL(commands[0].incl_kind);
    n_byte *last_characters;
    do
    {
        n_byte    data_incl = FILE_INCL(commands[lp].incl_kind);
        last_characters = commands[lp].characters;
        if (last_incl != data_incl)
        {
            n_uint   local_kind = FILE_KIND(commands[lp-1].incl_kind);
            n_uint   data_size;
            n_uint   running_entry = commands[lp-1].start_location;
            
            switch(local_kind)
            {
                case FILE_TYPE_BYTE4:
                    data_size = 4;
                    break;
                case FILE_TYPE_BYTE2:
                    data_size = 2;
                    break;
                default:
                    data_size = 1;
                    break;
            }
            
            running_entry += data_size * commands[lp-1].number_entries;
            if (running_entry > max_entry)
            {
                max_entry = running_entry;
            }
            last_incl = data_incl;
        }
        lp++;
    }
    while (POPULATED(last_characters));
    return max_entry;
}

#define    FILE_MACRO_CONCLUSION(ch)    (((comman_req==1) && (ch) == ',') || \
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
        (type_from_command == FILE_TYPE_BYTE2) ||
        (type_from_command == FILE_TYPE_BYTE4))
    {
        n_uint    number = 0;
        n_byte    num_char;
        n_int   response_code = io_read_byte4(fil, &number, &num_char);
        
        if (response_code == 0)
        {
            return SHOW_ERROR("Expected number not found");
        }
        if (response_code < 0)
        {
            return SHOW_ERROR("Expected number too big");
        }
        
        if ((type_from_command == FILE_TYPE_BYTE) || (type_from_command == FILE_TYPE_BYTE_EXT))
        {
            if (number > 0x000000ff)
                return SHOW_ERROR("Expected byte too big");
            data_read[0] = (n_byte) number;
        }
        
        if (type_from_command == FILE_TYPE_BYTE2)
        {
            n_byte2    * data_read2 = (n_byte2 *)data_read;
            if (number > 0x0000ffff)
                return SHOW_ERROR("Expected two byte too big");
            data_read2[0] = (n_byte2) number;
        }
        
        if (type_from_command == FILE_TYPE_BYTE4)
        {
            n_byte4    * data_read4 = (n_byte4 *)data_read;
            data_read4[0] = (n_byte4) number;
        }
        
        if (FILE_MACRO_CONCLUSION(num_char))
            return (FILE_OKAY);
        return SHOW_ERROR("Number ends incorrectly");
    }
    return SHOW_ERROR("Type not found");
}

void io_output_contents(n_file * file)
{
    n_uint loop = 0;
    printf("--------------------FILE--------------------\n");
    printf("Location %ld\n", file->location);
    printf("Size %ld\n", file->size);
    printf("* * * * * * \n");
    
    while (loop < file->location)
    {
        printf("%c", file->data[loop++]);
    }
    printf("--------------------------------------------n");
    
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
n_int    io_read_buff(n_file * fil, n_byte * data, const noble_file_entry * commands)
{
    
    n_int    inclusion_number = 0xffff;
    n_int    result_number = 0;
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
            
            n_uint  com_number_of = commands[result_number].number_entries;
            n_uint  com_location  = commands[result_number].start_location;
            
            n_uint    loop = 0;
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
                n_byte    local_kind = com_kind;
                if ((loop + 1) != com_number_of)
                {
                    local_kind |= FILE_CONTINUATION;
                }
                if (io_read_data(fil, local_kind, local_data) != FILE_OKAY)
                {
                    return (FILE_ERROR);
                }
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

#define    IO_CHECK_ERROR(cnd)             \
{                                   \
n_int    out_cnd = cnd;          \
if ( (out_cnd) != FILE_OKAY)    \
return    out_cnd;            \
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
        const n_byte    *commands_bytes;
        n_byte          writeout_commands[7]= {0};
        n_uint    offset = 0;
        n_int    release = FILE_ERROR;
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
                n_uint    loop = 0;
                n_byte    data_type    = FILE_KIND(commands[offset].incl_kind);
                n_uint  end_loop     = commands[offset].number_entries;
                n_uint    data_offset  = commands[offset].start_location;
                n_int    right_ending = (FILE_INCL(commands[offset+1].incl_kind) != command_num);
                
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
                        n_byte4 num_write = 0;
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
                            {
                                num_write = ((n_byte2 *)  &byte_data[data_offset + (loop * 2)])[0];
                            }
                                break;
                            case FILE_TYPE_BYTE4:
                                num_write = ((n_byte4 *) &byte_data[data_offset + (loop * 4)])[0];
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
        n_uint    offset = 0;
        n_int      release = FILE_ERROR;
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
                n_uint    loop = 0;
                n_byte    data_type   = FILE_KIND(commands[offset].incl_kind);
                n_uint  end_loop    = commands[offset].number_entries;
                n_uint    data_offset = commands[offset].start_location;
                
                if (data_type != FILE_TYPE_PACKED)
                {
                    while (loop < end_loop)
                    {
                        n_byte2    num_write = 0;
                        
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

void io_search_file_format(const noble_file_entry * format, n_string compare)
{
    n_int              loop = 0;
    n_byte               print_file_format_exit = 0;
    n_byte               print_file_place_show_all = 0;
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
        n_uint   local_number     = format[loop].number_entries;
        n_uint   local_location   = format[loop].start_location;
        n_string_block   printout_characters = {0};
        local_characters = (n_byte*)format[loop].characters;
        if ((local_incl_kind & 0xF0) == section_to_audit)
        {
            n_uint local_type = local_incl_kind & 0x0F;
            
            if (local_type == FILE_TYPE_BYTE_EXT)
            {
                local_type = FILE_TYPE_BYTE;
            }
            
            if ((local_type == FILE_TYPE_BYTE) || (local_type == FILE_TYPE_BYTE2) || (local_type == FILE_TYPE_BYTE4))
            {
                printout_characters[0] = (n_char)local_characters[0];
                printout_characters[1] = (n_char)local_characters[1];
                printout_characters[2] = (n_char)local_characters[2];
                printout_characters[3] = (n_char)local_characters[3];
                printout_characters[4] = (n_char)local_characters[4];
                printout_characters[5] = (n_char)local_characters[5];
                
                printf("%s \t %lu * %lu = %lu bytes \t reported/actual/diff offset %d / %d / %d\n", printout_characters,
                       local_number, local_type, (local_number * local_type), (int)local_location, (int)being_counter, ((int)local_location - (int)being_counter));
                
                being_counter += (local_number * local_type);
            }
        }
        loop++;
    }
    while((local_characters[0] != 0) && (local_characters[1] != 0) &&
          (local_characters[2] != 0) && (local_characters[3] != 0));
}



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

void io_file_debug(n_file * file)
{
    n_uint loop = 0;
    while (loop < file->location)
    {
        printf("%c", file->data[loop]);
        loop++;
    }
    printf("\n\n");
}

