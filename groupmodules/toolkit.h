/****************************************************************

 toolkit.h

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

/*! \file   toolkit.h
 *  \brief  This is the interface between the ApeSDK toolkit and what consumes
    the ApeSDK toolkit.
 */

#ifndef _TOOLKIT_H_
#define _TOOLKIT_H_
/*	Variable Definitions */

#include <signal.h> // for SIMULATED_APE_ASSERT

#undef   COMMAND_LINE_DEBUG       /* Sends the debug output as printf output - added through command line build */


// #define COMMON_DEBUG_ON /* should be passed via target definitions via cmd */

#ifdef COMMON_DEBUG_ON
    #define COMMON_DEBUG(format, ...) printf(format, ##__VA_ARGS__)
#else
    #define COMMON_DEBUG(format, ...) /* value */
#endif

#define CHAR_SPACE               (32)
#define IS_RETURN(val)            (((val) == 10) || ((val) == 13))
#define IS_SPACE(val)             ((val) == CHAR_SPACE)

#undef  SIMULATED_APE_ASSERT

#define PACKED_DATA_BLOCK   (32*32*32*2)

typedef	double	n_double;

#define TWO_PI ((n_double)(6.2831853071795864769252867665590057683943))

#define SINE_MAXIMUM (26880)

#define BIG_INTEGER          (2147483647)
#define BIG_NEGATIVE_INTEGER (0-2147483648)

#define NOTHING (0L)

typedef char n_char;

/*! @typedef n_string
 @discussion This is the string format for the Simulated Ape development */
typedef n_char           *n_string;

typedef const n_char     *n_constant_string;

#define STRING_BLOCK_SIZE (4096)

typedef n_char       n_string_block[STRING_BLOCK_SIZE];

/*! @typedef n_byte
@discussion This is a single byte data unit. */
typedef	unsigned char	n_byte;
/*! @typedef n_byte2
@discussion This is a two byte data unit. There is no
 expectations on the byte ordering. */
typedef	unsigned short	n_byte2;

typedef	unsigned int	n_byte4;
typedef	int				n_c_int;

#ifndef _WIN64
/*! @typedef n_uint
@discussion This is a four byte data unit. Please note that
this form may be shown as unsigned int on some 64-bit
implementations. */
typedef	unsigned long	n_uint;

/*! @typedef n_int
@discussion This is the native integer data unit. It can be
whatever length is specified by the implementation. */
typedef	long			n_int;

#else

typedef	unsigned long long	n_uint;
typedef	long long			n_int;

#endif


typedef enum
{
    FILE_TYPE_BYTE        = 0x01,
    FILE_TYPE_BYTE2        = 0x02,
    FILE_TYPE_BYTE_EXT    = 0x03,
    FILE_TYPE_PACKED    = 0x05,
    FILE_TYPE_BYTE4     = 0x06
} file_element_type;


#ifdef SIMULATED_APE_ASSERT

#define NA_ASSERT(test, message) if(!(test)){io_assert(message, __FILE__, __LINE__); raise(SIGINT);}

void io_assert( n_string message, n_string file_loc, n_int line );

#else


//#define NA_ASSERT(test, message) if (!(test)) printf("%s line: %d Soft Assert: %s\n",__FILE__, __LINE__, message)

#define NA_ASSERT(test, message) /* test message */

#endif

typedef union
{
    struct
    {
        n_int x;
        n_int y;
    };
    n_int data[2];
} n_vect2;


typedef union
{
    struct
    {
        n_vect2 top_left;
        n_vect2 bottom_right;
    };
    n_int data[4];
} n_area2;

typedef struct
{
    n_vect2 four[4];
} n_quad;

typedef struct
{
    n_vect2 two[2];
} n_line;

typedef union
{
    struct
    {
        n_double x;
        n_double y;
        n_double z;
    };
    n_double data[3];
} n_vect3;

typedef struct
{
    n_byte a;
    n_byte r;
    n_byte g;
    n_byte b;
} n_rgba;

typedef union
{
    n_rgba  rgba;
    n_byte4 thirtytwo;
} n_rgba32;

//enum window_information
//{
//    TERRAIN_WINDOW_WIDTH        = ( 4096 ),
//    TERRAIN_WINDOW_HEIGHT       = ( 3072 ),
//    TERRAIN_WINDOW_AREA         = ( TERRAIN_WINDOW_WIDTH * TERRAIN_WINDOW_HEIGHT ),
//    CONTROL_WINDOW_WIDTH        = ( 2048 ),
//    CONTROL_WINDOW_HEIGHT       = ( 2048 ),
//    CONTROL_WINDOW_AREA         = ( CONTROL_WINDOW_WIDTH * CONTROL_WINDOW_HEIGHT )
//};

typedef struct
{
    n_vect2 *points;
    n_int no_of_points;
    n_int max_points;
} n_points;

typedef struct
{
    n_byte4  date;
    n_byte2  location[2];
    n_byte4  time;
}
n_spacetime;

/*! @struct
 @field characters Characters that represent these values in the file.
 @field incl_kind  Included type and the kind of value combined together.
 @field number     Number of these values.
 @field location   Byte location of the start of these values within the struct.
 @field what_is_it Provides a string output for HTML documentation for the user.
 @discussion This is the line by line definition of the Simulated Ape file types.
 */
typedef	struct
{
    n_byte  characters[7];
    n_byte  incl_kind;
    n_uint  number_entries;
    n_uint  start_location;
    const n_string what_is_it;
} simulated_file_entry;

typedef struct
{
    n_uint  count;
    n_uint  max;
    n_uint  unit_size;
    n_byte *data;
} memory_list;

typedef memory_list int_list;

typedef struct
{
    int_list * number;
    void * array;
} number_array;

typedef memory_list number_array_list;

#define POPULATED(ch) ((ch[0] != 0) || (ch[1] != 0) || (ch[2] != 0) || (ch[3] != 0) || (ch[4] != 0) || (ch[5] != 0))

/* include externally, if needed */

#define FILE_COPYRIGHT      0x00

#define FILE_INCL(num)      ((num) & 0xf0)
#define FILE_KIND(num)      ((num) & 0x0f)

#define	FILE_EOF				0x0100

#define	ASCII_NUMBER(val)		(((val) >= '0') && ((val) <= '9'))
#define	ASCII_LOWERCASE(val)	(((val) >= 'a') && ((val) <= 'z'))
#define	ASCII_UPPERCASE(val)	(((val) >= 'A') && ((val) <= 'Z'))

#define FILE_OKAY				  0x0000
#define	FILE_ERROR				  (-1)


typedef n_byte ( n_pixel )( n_int px, n_int py, n_int dx, n_int dy, void *information );

typedef n_int ( n_memory_location )( n_int px, n_int py );

typedef n_byte2 ( n_patch )( n_byte2 *local );

typedef n_int ( n_file_in )( n_byte *buff, n_uint len );

typedef n_byte *( n_file_out )( n_uint *len );

/*! @struct
@field pixel_draw The n_pixel function used to draw pixels into
the window buffer.
@field information This is the pointer information passed into
the pixel_draw function.
@discussion This drawing interface was designed to write to
window buffer information where the window buffer could be
either color or monochrome. The interface needed to be relatively
independent to allow for text to be written into either a color
window or a monochrome window or a line to be drawn through
either window buffer without there being any difference in the
implementation algorithm. Think of this method as a way of
translating high-level drawing and low-level drawing.
*/
typedef struct
{
    n_pixel *pixel_draw;
    void	 *information;
} n_join;

typedef struct
{
    n_byte	 *screen;
    n_byte	 *background;
} n_background8;

typedef struct
{
    n_byte   *screen;
    n_byte	  color;
} n_color8;

/*! @struct
@field size The size of the file in bytes.
@field location The location of the accessing pointer within the
file. This is useful for both input and output files.
@field data The data stored in bytes.
@discussion This is the primary file handling structure in the ApeSDK. It is used for both input and output files. It is the method
used to pass file information from the platform layer into the
platform independent layers of Simulated Ape.
*/
typedef struct
{
    n_uint	size;
    n_uint	location;
    n_byte	*data;
} n_file;

typedef void ( n_file_specific )( n_string string, n_byte *reference );

typedef struct
{
    void *data;
    n_uint expected_bytes;
    n_uint hash;
    void *next;
} n_file_chain;

typedef enum
{
    OBJECT_EMPTY = 0,
    OBJECT_STRING,
    OBJECT_NUMBER,
    OBJECT_BOOLEAN,
    OBJECT_OBJECT,
    OBJECT_ARRAY = 5,
} n_object_type;

static const n_string n_object_type_string[OBJECT_ARRAY+1] =
{
    "OBJECT_EMPTY",
    "OBJECT_STRING",
    "OBJECT_NUMBER",
    "OBJECT_BOOLEAN",
    "OBJECT_OBJECT",
    "OBJECT_ARRAY"
};

typedef enum
{
    OBJ_TYPE_EMPTY = 0,
    OBJ_TYPE_STRING_NOTATION,
    OBJ_TYPE_NUMBER,
    OBJ_TYPE_COLON,
    OBJ_TYPE_COMMA,
    OBJ_TYPE_BOOLEAN,
    OBJ_TYPE_OBJECT_OPEN,
    OBJ_TYPE_OBJECT_CLOSE,
    OBJ_TYPE_ARRAY_OPEN,
    OBJ_TYPE_ARRAY_CLOSE
} n_object_stream_type;


typedef struct
{
    n_string       data;
    void          *next;
    n_object_type  type;
} n_array;

typedef struct
{
    n_array        primitive;
    n_string       name;
    n_uint         name_hash;
} n_object;

typedef void (memory_execute)(void);

void memory_execute_set(memory_execute * value);
void memory_execute_run(void);

n_array *array_number( n_int set_number );
n_array *array_boolean( n_int set_boolean );
n_array *array_string( n_string set_string );
n_array *array_object( n_object *set_object );
n_array *array_array( n_array *set_array );

n_array *array_add( n_array *array, n_array * element );
void array_add_empty( n_array ** array, n_array * element );

n_object *object_number( n_object *obj, n_string name, n_int number );
n_object *object_boolean( n_object *obj, n_string name, n_int boolean );
n_object *object_string( n_object *obj, n_string name, n_string string );
n_object *object_object( n_object *obj, n_string name, n_object *object );
n_object *object_array( n_object *obj, n_string name, n_array *array );

void object_top_object( n_file *file, n_object *top_level );

void *unknown_file_to_tree( n_file *file, n_object_type *type );
n_file *unknown_json( void *unknown, n_object_type type );
void unknown_free( void **unknown, n_object_type type );

void obj_free( n_object **object );

n_string obj_contains( n_object *base, n_string name, n_object_type type );
n_int    obj_contains_number( n_object *base, n_string name, n_int *number );

n_int    obj_contains_array_numbers( n_object *base, n_string name, n_int *array_numbers, n_int size );
n_int    obj_contains_array_nbyte2( n_object *base, n_string name, n_byte2 *array_numbers, n_int size );

n_array  *obj_get_array( n_string array );
n_object *obj_get_object( n_string object );
n_int     obj_get_number( n_string object );
n_int     obj_get_boolean( n_string object );

n_array  *obj_array_next( n_array *array, n_array *element );
n_int     obj_array_count( n_array *array_obj );

memory_list * object_list_vect2(n_array * vect_array);
n_array *     object_vect2_pointer(n_vect2 * vect_array, n_uint count);
n_int         object_vect2_from_array(n_array * vect_element, n_vect2 * vect_list);

n_array * object_vect2_array(n_vect2 * value);

typedef n_int ( object_unwrap )( n_string pass_through, n_byte * buffer);


void object_output_object(n_object * value);
void object_output_array(n_array * value);

n_array * object_onionskin(n_object * top, n_string value);

memory_list * object_unwrap_array(n_array * general_array, n_uint size, object_unwrap wrap_func, n_object_type type);

n_object * object_vect2_names(n_string names, memory_list * vect_array);
n_object * object_vect2_name( n_object *obj, n_string name, n_vect2 * value);

n_int   object_name_vect2(n_string name, n_vect2 * value, n_object * input_json);

n_object * object_line(n_vect2 * values);
n_object * object_quad(n_vect2 * values);

n_int vect2_unwrap_quad( n_string pass_through, n_byte * buffer);
n_int vect2_unwrap_line( n_string pass_through, n_byte * buffer);

n_int object_unwrap_four_vect2( n_string pass_through, n_byte * buffer);
n_int object_unwrap_two_vect2( n_string pass_through, n_byte * buffer);
n_int object_unwrap_vect2( n_string pass_through, n_byte * buffer);

n_int object_count_name_vect2(n_vect2 * vect_array, n_uint count, object_unwrap * wrap_func, n_string name, n_object * object);

void object_init(n_uint * hashes, n_uint hash_count);
void object_close(void);
void object_ptr_debug(void * ptr);

n_uint object_get_hash_count(void);

n_string object_type_string(n_object_type objtype);

/** \brief sine and cosine conversation */
#define	NEW_SD_MULTIPLE			26880

extern n_int draw_error( n_constant_string error_text, n_constant_string location, n_int line_number );

#define	SHOW_ERROR(val)	(draw_error(val, __FILE__, __LINE__))

#define SHOW_SUCCESS (1)
#define SHOW_NO_OP   (0)
#define SHOW_FAILURE (-1)

#define IO_LOWER_CHAR(value)   if(ASCII_UPPERCASE(value)) (value) += 'a' - 'A'

typedef n_int ( execute_function )( void *general_data, void *read_data, void *write_data );
typedef void ( execute_thread_stub )( execute_function function, void *general_data, void *read_data, void *write_data );

void  execute_group( execute_function *function, void *general_data, void *read_data, n_int count, n_int size );

void area2_add( n_area2 *area, n_vect2 *vect, n_byte first );


n_int vect2_distance_under( n_vect2 *first, n_vect2 *second, n_int distance );

void  vect2_byte2( n_vect2 *converter, n_byte2 *input );
void  vect2_add( n_vect2 *equals, n_vect2 *initial, n_vect2 *second );
void  vect2_center( n_vect2 *center, n_vect2 *initial, n_vect2 *second );

void  vect2_subtract( n_vect2 *equals, n_vect2 *initial, n_vect2 *second );
void  vect2_divide( n_vect2 *equals, n_vect2 *initial, n_vect2 *second, n_int divisor );
void  vect2_multiplier(
    n_vect2 *equals, n_vect2 *initial,
    n_vect2 *second, n_int multiplier, n_int divisor );
void  vect2_d(
    n_vect2 *initial, n_vect2 *second,
    n_int multiplier, n_int divisor );
n_int vect2_dot(
    n_vect2 *initial, n_vect2 *second,
    n_int multiplier, n_int divisor );
void vect2_rotate90( n_vect2 *rotation );
void vect2_direction( n_vect2 *initial, n_int direction, n_int divisor );
void vect2_delta( n_vect2 *initial, n_vect2 *delta );
void vect2_offset( n_vect2 *initial, n_int dx, n_int dy );
void vect2_back_byte2( n_vect2 *converter, n_byte2 *output );
void vect2_copy( n_vect2 *to, n_vect2 *from );
void vect2_populate( n_vect2 *value, n_int x, n_int y );
void vect2_rotation( n_vect2 *location, n_vect2 *rotation );
void vect2_rotation_bitshift( n_vect2 *location, n_vect2 *rotation );

n_int vect2_nonzero( n_vect2 *nonzero );

n_vect2 *vect2_min_max_init( void );
void vect2_min_max_permutation( n_vect2 * points, n_vect2 * minmax);

void vect2_min_max( n_vect2 *points, n_int number, n_vect2 *maxmin );
void vect2_min_max_permutation( n_vect2 * points, n_vect2 * minmax);

void vect2_scalar_multiply( n_vect2 *value, n_int multiplier );
void vect2_scalar_divide( n_vect2 *value, n_int divisor );
void vect2_scalar_bitshiftdown( n_vect2 *value, n_int bitshiftdown );

n_array * vect2_memory_list_number_array(memory_list * list, n_int number);
n_int vect2_unwrap_number_entry( n_string pass_through, n_byte * buffer, n_int number);
n_int vect2_unwrap_number( n_array * array, n_vect2 * entry, n_int number);

void vect3_double( n_vect3 *converter, n_double *input );
void vect3_add( n_vect3 *equals, n_vect3 *initial, n_vect3 *second );
void vect3_center( n_vect3 *center, n_vect3 *initial, n_vect3 *second );
void vect3_subtract( n_vect3 *equals, n_vect3 *initial, n_vect3 *second );
void vect3_divide( n_vect3 *equals, n_vect3 *initial, n_vect3 *second, n_double divisor );
void vect3_multiplier( n_vect3 *equals, n_vect3 *initial, n_vect3 *second,
                       n_double multiplier, n_double divisor );
void vect3_d( n_vect3 *initial, n_vect3 *second, n_double multiplier, n_double divisor );
n_double vect3_dot( n_vect3 *initial, n_vect3 *second, n_double multiplier, n_double divisor );
void vect3_delta( n_vect3 *initial, n_vect3 *delta );
void vect3_offset( n_vect3 *initial, n_double dx, n_double dy, n_double dz );
void vect3_back_double( n_vect3 *converter, n_double *output );
void vect3_copy( n_vect3 *to, n_vect3 *from );
void vect3_populate( n_vect3 *value, n_double x, n_double y, n_double z );
n_int vect3_nonzero( n_vect3 *nonzero );

n_byte *math_general_allocation( n_byte *bc0, n_byte *bc1, n_int i );

void math_general_execution( n_int instruction, n_int is_constant0, n_int is_constant1,
                             n_byte *addr0, n_byte *addr1, n_int value0, n_int *i,
                             n_int is_const0, n_int is_const1,
                             n_byte *pspace,
                             n_byte *bc0, n_byte *bc1,
                             n_int braincode_min_loop );

n_byte4  math_hash_fnv1( n_constant_string key );
n_uint   math_hash( n_byte *values, n_uint length );

n_uint  math_root( n_uint squ );
n_int   math_tan( n_vect2 *p );

#undef DEBUG_RANDOM
#undef VERBOSE_DEBUG_RANDOM

#ifdef DEBUG_RANDOM

#include <stdio.h>

#define math_random(num)  math_random_debug(num, __FILE__, __LINE__)
#define math_random3(num)  math_random_debug(num, __FILE__, __LINE__)

#define mrdc(string) math_random_debug_count(string)

n_byte2 math_random_debug( n_byte2 *local, n_string file_string, n_int line_number );

void math_random_debug_count( n_string place );

#else

#define mrdc(string) /* math_random_debug_count(string) */

n_byte2 math_random( n_byte2 *local );
void    math_random3( n_byte2 *local );

#endif

n_byte  math_join( n_int sx, n_int sy, n_int dx, n_int dy, n_join *draw );
n_int   math_spread_byte( n_byte val );

n_int  math_sine( n_int direction, n_int divisor );

n_byte math_join_vect2( n_int sx, n_int sy, n_vect2 *vect, n_join *draw );
n_byte math_line_vect( n_vect2 *point1, n_vect2 *point2, n_join *draw );
n_byte math_line( n_int x1, n_int y1, n_int x2, n_int y2, n_join *draw );

n_int  math_seg14( n_int character );

n_byte math_do_intersect( n_vect2 *p1, n_vect2 *q1, n_vect2 *p2, n_vect2 *q2 );

void       io_number_to_string( n_string value, n_uint number );
void       io_string_number( n_string output_string, n_string input_string, n_uint number );
void       io_three_strings( n_string output_string, n_string first_string, n_string second_string, n_string third_string, n_byte new_line );

void       io_entry_execution( n_int argc, n_string *argv );
void       io_command_line_execution_set( void );
n_int      io_command_line_execution( void );

void       io_lower( n_string value, n_int length );
void       io_whitespace( n_file *input );
void       io_whitespace_json( n_file *input );
void       io_audit_file( const simulated_file_entry *format, n_byte section_to_audit );
void       io_search_file_format( const simulated_file_entry *format, n_string compare );
void       io_string_write( n_string dest, n_string insert, n_int *pos );
n_int      io_read_bin( n_file *fil, n_byte *local_byte );
n_int      io_file_write( n_file *fil, n_byte byte );
void       io_file_reused( n_file *fil );
n_file *   io_file_duplicate(n_file * initial);
n_int      io_write( n_file *fil, n_constant_string ch, n_byte new_line );
n_int      io_writenumber( n_file *fil, n_int loc_val, n_uint numer, n_uint denom );
n_int      io_length( n_string value, n_int max );
n_int      io_find( n_string check, n_int from, n_int max, n_string value_find, n_int value_find_length );
n_int      io_read_buff( n_file *fil, n_byte *data, const simulated_file_entry *commands );
n_int      io_write_buff( n_file *fil, void *data, const simulated_file_entry *commands, n_byte command_num, n_file_specific *func );
n_int      io_write_csv( n_file *fil, n_byte *data, const simulated_file_entry *commands, n_byte command_num, n_byte initial ) ;

void       memory_erase( n_byte *buf_offscr, n_uint nestop );

void       memory_copy( n_byte *from, n_byte *to, n_uint number );
void      *memory_new( n_uint bytes );
void       memory_free( void **ptr );
void      *memory_new_range( n_uint memory_min, n_uint *memory_allocated );

memory_list *memory_list_new( n_uint size, n_uint number );
void memory_list_copy( memory_list *list, n_byte *data, n_uint size);
void memory_list_free( memory_list **value );

int_list *int_list_new( n_uint number );
void int_list_copy( int_list *list, n_int int_add);
void int_list_free( int_list **value );

n_int int_list_find( int_list *list,  n_int location, n_int * error);
void int_list_debug( int_list * debug_list);

n_file    *io_file_new( void );
n_file    *io_file_new_from_string_block(n_string_block contents);
n_file    *io_file_new_from_string(n_string string, n_uint string_length);

void       io_file_free( n_file **file );
void       io_file_debug( n_file *file );

n_int      io_number( n_string number_string, n_int *actual_value, n_int *decimal_divisor );

n_int      io_disk_read( n_file *local_file, n_string file_name );
n_int      io_disk_read_no_error( n_file *local_file, n_string file_name );

n_int      io_disk_write( n_file *local_file, n_constant_string file_name );
n_int      io_disk_check( n_constant_string file_name );
n_string *io_tab_delimit_to_n_string_ptr( n_file *tab_file, n_int *size_value, n_int *row_value );

void       io_three_string_combination( n_string output, n_string first, n_string second, n_string third, n_int count );
void       spacetime_to_string( n_string value );
n_string   io_string_copy( n_string string );
void       io_string_copy_buffer( n_string string, n_string buffer );

n_int      io_read_byte4( n_file *fil, n_uint *actual_value, n_byte *final_char );
n_int      io_writenum( n_file *fil, n_int loc_val, n_byte ekind, n_byte new_line );
n_int      io_command( n_file *fil, const simulated_file_entry *commands );
n_int      io_read_data( n_file *fil, n_byte2 command, n_byte *data_read );

void       io_output_contents( n_file *file );

n_uint     io_file_hash( n_file *file );

n_file *io_file_ready( n_int entry, n_file *file );

void io_file_cleanup( n_int *entry, n_file **file );

void io_file_writeon( n_int *entry, n_file **file, n_byte blocked_write );

void io_file_writeoff( n_int *entry, n_file *file );

void io_file_string( n_int entry, n_file *file, n_constant_string string );
n_uint io_find_size_data( simulated_file_entry *commands );


number_array_list * number_array_list_new(void);
void number_array_list_free(number_array_list ** nal);
number_array * number_array_list_find(number_array_list * nal, void * array);
number_array * number_array_list_find_add(number_array_list * nal, void * array);

void number_array_not_number(number_array * na);
void number_array_number(number_array * na, n_int number);
n_int number_array_get_number(number_array * na, n_int location, n_int * error);
n_int number_array_get_size(number_array * na);

#define ASCII_QUOTE(num)      ((num) == '"')

#define ASCII_TEXT(num)		  ((ASCII_UPPERCASE(num) || ASCII_LOWERCASE(num)) || ((num) == '_'))

#define ASCII_SEMICOLON(num)  ((num) == ';')
#define ASCII_COLON(num)      ((num) == ':')

#define ASCII_COMMA(num)      ((num) == ',')

#define	ASCII_EQUAL(num)	  ((num) == '=')

#define	ASCII_BRACKET(num)    (((num) == '(')||((num) == ')'))
#define	ASCII_BRACES(num)     (((num) == '{')||((num) == '}'))

#define ASCII_LOGICAL(num)	  ((((num) == '&')||((num) == '|'))||(((num) == '^')||((num) == '!')))
#define ASCII_ARITHMETIC(num) ((((num) == '+')||((num) == '-'))||(((num) == '*')||((num) == '/')))

#define ASCII_DIRECTIONAL(num) (((num)=='<')||((num)=='>'))

#define	CODE_VALUE_REQUIRED(num) (((num) == APESCRIPT_OPERATOR || (num) == APESCRIPT_NUMBER) || ((num) == APESCRIPT_TEXT))

#define	SIZEOF_NUMBER_WRITE      (sizeof(n_int))

void io_int_to_bytes( n_int value, n_byte *bytes );
n_int io_bytes_to_int( n_byte *bytes );


#ifndef ABS
#define ABS(a)	                        (((a) < 0) ? -(a) : (a))
#endif


typedef    short    n_audio;

#define AUDIO_FFT_MAX_BITS      (15)
#define AUDIO_FFT_MAX_BUFFER    (1<<AUDIO_FFT_MAX_BITS)

void audio_fft( n_byte inverse, n_uint power_sample );
void audio_new_fft( n_uint       power_sample,
                    n_int      InverseTransform,
                    n_double    *RealIn,
                    n_double    *ImagIn,
                    n_double    *RealOut,
                    n_double    *ImagOut );
void   audio_clear_buffers( n_uint length );
void   audio_clear_output( n_audio *audio, n_uint length );

void   audio_equal_output( n_audio *audio, n_uint length );

void   audio_multiply_output( n_audio *audio, n_uint length );
void   audio_set_frequency( n_uint entry, n_uint value );

void   audio_low_frequency( n_audio *buffer, n_int number_freq, n_int debug );

void audio_buffer_clear( n_audio *buffer, n_int size );
void audio_buffer_double_clear( n_double *buffer, n_int size );

void audio_buffer_copy_to_audio( n_double *buffer_double, n_audio *buffer_audio, n_int size );
void audio_buffer_copy_to_double( n_audio *buffer_audio, n_double *buffer_double, n_int size );
void audio_buffer_copy_to_double_double( n_double *buffer_double1, n_double *buffer_double2, n_int size );
void audio_buffer_copy_to_double_double( n_double *buffer_double_to, n_double *buffer_double_from, n_int size );

typedef n_string ( n_console_input )( n_string value, n_int length );

typedef void ( n_console_output )( n_constant_string value );

typedef n_int ( n_console )( void *ptr, n_string response, n_console_output output_function );

typedef struct
{
    n_console *function;
    n_string    command;
    n_string    addition;
    n_string    help_information;
} simulated_console_command;

void       audio_aiff_header( void *fptr, n_uint total_samples );
n_int      audio_aiff_is_header( void *fptr, n_uint *samples );

void       audio_aiff_body( void *fptr, n_audio *samples, n_uint number_samples );

n_int      io_quit( void *ptr, n_string response, n_console_output output_function );
n_int      io_help( void *ptr, n_string response, n_console_output output_function );
n_string   io_console_entry_clean( n_string string, n_int length );
n_string   io_console_entry( n_string string, n_int length );
void       io_console_out( n_constant_string value );
n_int      io_console( void *ptr, simulated_console_command *commands, n_console_input input_function, n_console_output output_function );

void       io_help_line( simulated_console_command *specific, n_console_output output_function );

void       io_console_quit( void );

#endif /* _TOOLKIT_H_ */

