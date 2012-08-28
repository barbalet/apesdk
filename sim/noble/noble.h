/****************************************************************

 noble.h

 =============================================================

 Copyright 1996-2012 Tom Barbalet. All rights reserved.

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

/*! \file   noble.h
 *  \brief  This is the interface between the Noble toolkit and what consumes
    the Noble Toolkit.
 */

/*NOBLEMAKE VAR=""*/

/*	Variable Definitions */

#undef   NEW_OPENGL_ENVIRONMENT

#define	 SCRIPT_DEBUG             /* Add all the runtime debug */
#undef   COMMAND_LINE_DEBUG       /* Sends the debug output as printf output */
#undef   ROUGH_CODE_OUT           /* printf outputs the interpret stream in character number format */
#undef   SKIM_TO_BRIANCODE        /* Skims the interpret stream to braincode printf output currently - only with lance */

#undef   DEBUG_NON_FILE_HANDLE    /* Stops there being a file handle for debug output */

#define LATITUDE_MEAN_TEMPERATURE 20000

/*! @define */
#define	SHORT_VERSION_NAME		 "Noble Ape 0.695 "
#define	FULL_DATE				 __DATE__
#define SUBVERSION_VERSION		"SOURCE REVISION 1061"

/*! @define */
#define	VERSION_NUMBER		     695
#define	COPYRIGHT_DATE		     "Copyright 1996 - 2012 "

#define FULL_VERSION_COPYRIGHT "Copyright Tom Barbalet, 1996 - 2012"

/*! @define */
#define	NOBLE_APE_SIGNATURE		    (('N'<< 8) | 'A')
#define	NOBLE_WARFARE_SIGNATURE		(('N'<< 8) | 'W')

#define  COPYRIGHT_NAME		    "Tom Barbalet "
#define  COPYRIGHT_FOLLOW		"All rights reserved "


#define PACKED_DATA_BLOCK   (32*32*32*2)

/*! @typedef n_string
 @discussion This is the string format for the Noble Ape development */
typedef char *          n_string;

#define STRING_BLOCK_SIZE (2048)

typedef char       n_string_block[STRING_BLOCK_SIZE];

/*! @typedef n_byte
@discussion This is a single byte data unit. */
typedef	unsigned char	n_byte;
/*! @typedef n_byte2
@discussion This is a two byte data unit. There is no
 expectations on the byte ordering. */
typedef	unsigned short	n_byte2;


typedef	unsigned int	n_c_uint;
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

/*! @struct
@field signature The program signature defined as NOBLE_APE_SIGNATURE
through the Noble Ape Simulation file handling etc.
@field version The version of the implementation that is the version
number defined as VERSION_NUMBER (thus changes between versions).
@discussion This is a means of tracking the programs and file versions
that are used primarily through the file interface but also through
other implementations over the Noble Ape development. This includes
Noble Warfare currently, but potentially could include other software
that accepts the Noble Ape text based file format.
*/
typedef	struct
{
    n_byte2 signature;
    n_byte2 version;
}
n_version;

/*! @struct
 @field x     X value in the 2d vector.
 @field y     Y value in the 2d vector.
 @discussion  This is the bases of two dimensional vectors in the simulation.
 */
typedef struct
{
    n_int x;
    n_int y;
} n_vect2;


/*! @struct
 @field characters Characters that represent these values in the file.
 @field incl_kind  Included type and the kind of value combined together.
 @field number     Number of these values.
 @field location   Byte location of the start of these values within the struct.
 @field what_is_it Provides a string output for HTML documentation for the user.
 @discussion This is the line by line definition of the Noble Ape file types.
 */
typedef	struct
{
    n_byte  characters[6];
    n_byte  incl_kind;
    n_byte  number;
    n_byte2 location;
    const n_string what_is_it;
}
noble_file_entry;

/* include externally, if needed */
#define	FILE_TYPE_BYTE			0x01
#define	FILE_TYPE_BYTE2			0x02
#define FILE_TYPE_BYTE_EXT		0x03

#define	FILE_TYPE_PACKED		0x05

#define FILE_COPYRIGHT      0x00

#define FILE_INCL(num)      ((num) & 0xf0)
#define FILE_KIND(num)      ((num) & 0x0f)

#define	FILE_EOF				0x0100

#define	ASCII_NUMBER(val)		(((val) >= '0') && ((val) <= '9'))
#define	ASCII_LOWERCASE(val)	(((val) >= 'a') && ((val) <= 'z'))
#define	ASCII_UPPERCASE(val)	(((val) >= 'A') && ((val) <= 'Z'))

#define CHAR_TAB                 (9)
#define CHAR_SPACE               (32)

/* this is v2 of the file parsing, v3 is soon to be implemented through the scripting interface */
#define CHAR_EOF              	  0
#define	IS_RETURN(val)			  (((val) == 10) || ((val) == 13))
#define IS_TAB(val)               ((val) == CHAR_TAB)
#define IS_SPACE(val)             ((val) == CHAR_SPACE)
#define	IS_WHITE_HORIZON(val)	  (IS_TAB(val)  || IS_SPACE(val))
#define	IS_WHITE_SPACE(val)		  (IS_WHITE_HORIZON((val))||IS_RETURN((val)))
#define FILE_OKAY				        0x0000
#define FILE_END_INCLUSION      0x0101
#define	FILE_TYPE(num)			    ((num)&0x07)
#define	FILE_CONTINUATION		    0x80
#define	FILE_ERROR				      (-1)

typedef n_string (n_console_input)(n_string value, n_int length);

typedef void (n_console_output)(n_string value);

/*! @typedef
 @field px The x location of the window buffer being drawn.
 @field py The y location of the window buffer being drawn.
 @field information Information pointer coming through the n_join
 structure.
 @return If non-zero, this may exit the drawing depending on the
 particular use of the generic draw function.
 @discussion This is the agnostic window buffer interface to
 allow drawing at a high-level without causing problems over the
 two primary window buffer formats used in Noble Ape currently.
 */
typedef n_byte (n_pixel)(n_int px, n_int py, void * information);

typedef n_byte2 (n_patch)(n_byte2 * local);

typedef n_int (n_console)(void * ptr, n_string response, n_console_output output_function);

typedef n_int (n_file_in)(n_byte * buff, n_uint len);

typedef n_byte * (n_file_out)(n_uint * len);

typedef struct
{
    n_console * function;
    n_string    command;
    n_string    addition;
    n_string    help_information;
}
noble_console_command;

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
    n_pixel * pixel_draw;
    void	* information;
}
n_join;

/*! @struct
@field size The size of the file in bytes.
@field location The location of the accessing pointer within the
file. This is useful for both input and output files.
@field data The data stored in bytes.
@discussion This is the primary file handling structure in Noble
Ape. It is used for both input and output files. It is the method
used to pass file information from the platform layer into the
platform independent layers of Noble Ape.
*/
typedef struct
{
    n_uint	size;
    n_uint	location;
    n_byte	*data;
}
n_file;

typedef void (n_file_specific)(n_byte * string, n_byte * reference);

/** \brief new_sd stands for new sine dump and hold the sine and cosine values for the simulation */
const static n_int	new_sd[256] =
{
    0, 659, 1318, 1977, 2634, 3290, 3944, 4595, 5244, 5889, 6531, 7169, 7802,
    8431, 9055, 9673, 10286, 10892, 11492, 12085, 12671, 13249, 13819, 14380,
    14933, 15477, 16012, 16537, 17052, 17557, 18051, 18534, 19007, 19467, 19916,
    20353, 20778, 21190, 21590, 21976, 22349, 22709, 23055, 23387, 23706, 24009,
    24299, 24573, 24833, 25078, 25308, 25523, 25722, 25906, 26074, 26226, 26363,
    26484, 26589, 26677, 26750, 26807, 26847, 26871, 26880, 26871, 26847, 26807,
    26750, 26677, 26589, 26484, 26363, 26226, 26074, 25906, 25722, 25523, 25308,
    25078, 24833, 24573, 24299, 24009, 23706, 23387, 23055, 22709, 22349, 21976,
    21590, 21190, 20778, 20353, 19916, 19467, 19007, 18534, 18051, 17557, 17052,
    16537, 16012, 15477, 14933, 14380, 13819, 13249, 12671, 12085, 11492, 10892,
    10286, 9673, 9055, 8431, 7802, 7169, 6531, 5889, 5244, 4595, 3944, 3290, 2634,
    1977, 1318, 659, 0, -660, -1319, -1978, -2635, -3291, -3945, -4596, -5245,
    -5890, -6532, -7170, -7803, -8432, -9056, -9674, -10287, -10893, -11493,
    -12086, -12672, -13250, -13820, -14381, -14934, -15478, -16013, -16538,
    -17053, -17558, -18052, -18535, -19008, -19468, -19917, -20354, -20779,
    -21191, -21591, -21977, -22350, -22710, -23056, -23388, -23707, -24010,
    -24300, -24574, -24834, -25079, -25309, -25524, -25723, -25907, -26075,
    -26227, -26364, -26485, -26590, -26678, -26751, -26808, -26848, -26872,
    -26880, -26872, -26848, -26808, -26751, -26678, -26590, -26485, -26364,
    -26227, -26075, -25907, -25723, -25524, -25309, -25079, -24834, -24574,
    -24300, -24010, -23707, -23388, -23056, -22710, -22350, -21977, -21591,
    -21191, -20779, -20354, -19917, -19468, -19008, -18535, -18052, -17558,
    -17053, -16538, -16013, -15478, -14934, -14381, -13820, -13250, -12672,
    -12086, -11493, -10893, -10287, -9674, -9056, -8432, -7803, -7170, -6532,
    -5890, -5245, -4596, -3945, -3291, -2635, -1978, -1319, -660
};


/** \brief sine and cosine conversation */
#define	NEW_SD_MULTIPLE			26880
#define	OLD_SD_NEW_SD(x)		((new_sd[(x)&255])>>5)
#define VECT_X(f)         	(OLD_SD_NEW_SD(((f)) + 64))
#define VECT_Y(f)         	(OLD_SD_NEW_SD((f)))

#define TERRAIN_WINDOW_512x512 /* added for RC 694 */

#ifdef TERRAIN_WINDOW_512x512

#define	TERRAIN_WINDOW_WIDTH_BITS		(9)

#define	TERRAIN_WINDOW_WIDTH			(1<<TERRAIN_WINDOW_WIDTH_BITS)
#define	TERRAIN_WINDOW_HEIGHT			(512)

#else

#define	TERRAIN_WINDOW_WIDTH_BITS		(10)

#define	TERRAIN_WINDOW_WIDTH			(1<<TERRAIN_WINDOW_WIDTH_BITS)
#define	TERRAIN_WINDOW_HEIGHT			(768)


#endif

#define	BOTTOM_CHECK				((720*TERRAIN_WINDOW_HEIGHT)/256)


typedef enum
{
    KIND_LOAD_FILE = 0,
    KIND_NEW_SIMULATION,
    KIND_START_UP,
    KIND_MEMORY_SETUP
} KIND_OF_USE;

typedef enum
{
    AE_NO_ERROR = -1,

    AE_UNKNOWN_ERROR,
    AE_NUMBER_EXPECTED,
    AE_NUMBER_OUT_OF_RANGE,
    AE_MAXIMUM_NUMBERS_REACHED,
    AE_MAXIMUM_SCRIPT_SIZE_REACHED,

    AE_MAXIMUM_VARIABLES_REACHED,
    AE_UNKNOWN_SYNTAX_PARSER_BUFFER,
    AE_UNKNOWN_SYNTAX_PARSER_CONVERT,

    AE_SELECTED_ENTITY_OUT_OF_RANGE,

    AE_COORDINATES_OUT_OF_RANGE,
    AE_VALUE_OUT_OF_RANGE,

    AE_TOO_MANY_CLOSE_BRACES,
    AE_MAXIMUM_BRACES_REACHED,
    AE_FIRST_VALUE_FAILED,
    AE_SECOND_VALUE_FAILED,
    AE_UNKNOWN_SYNTAX_MISSING_EQUALS,
    AE_UNKNOWN_SYNTAX_NO_COMMAND,
    AE_WRONG_END,

    AE_LINE_START_INCORRECT,
    AE_OUTPUT_SET_AS_INPUT_VARIABLE,
    AE_IF_WHILE_NOT_FOLLOWED_BY_BRACKET,
    AE_FUNCTION_ISNT_VARIABLE,
    AE_NON_FUNCTION_APPLIED,
    AE_FUNCTION_DEFINED_PRIOR,
    AE_FUNCTION_OUT_OF_RANGE,
    AE_WITHOUT_SEMICOLON,
    AE_WITHOUT_OPEN_BRACE,
    AE_FUNCTION_SETTING_FAILED,
    AE_ERROR_STARTING_MAIN,
    AE_CODE_AFTER_MAIN,
    AE_NO_CLOSE_BRACE_TO_END_OF_FILE,
    AE_CODE_OUTSIDE_FUNCTION,
    AE_INPUT_VARIABLE_WITHOUT_EQUALS,
    AE_ASSIGN_VALUE_FAILED,
    AE_UNKNOWN_SYNTAX_FROM_INTERPRET,
    AE_NO_MAIN_CODE,

    AE_NUMBER_ERRORS
} AE_ENUM;

typedef struct
{
    AE_ENUM  enum_value;
    n_string error_string;
    n_string help_string;
}
n_ae_error;

/** \brief apescript_errors track the errors in ApeScript and additional text for user-manual level documentation */
static const n_ae_error apescript_errors[]=
{
    {AE_UNKNOWN_ERROR,                     "Unknown error",                    "Please contact tom at nobleape dot com"},
    {AE_NUMBER_EXPECTED,                   "Number expected",                  "A non-numeric character is included in a number string."},
    {AE_NUMBER_OUT_OF_RANGE,               "Number out of range",              "Number does not fit in the range"},
    {AE_MAXIMUM_NUMBERS_REACHED,           "Maximum numbers reached",          "Please contact tom at nobleape dot com"},
    {AE_MAXIMUM_SCRIPT_SIZE_REACHED,       "Maximum script size reached",      "Please contact tom at nobleape dot com"},
    {AE_MAXIMUM_VARIABLES_REACHED,         "Maximum variables reached",        "Please contact tom at nobleape dot com"},
    {AE_UNKNOWN_SYNTAX_PARSER_BUFFER,      "Unknown syntax (parser buffer)",   "Syntax is incorrect"},
    {AE_UNKNOWN_SYNTAX_PARSER_CONVERT,     "Unknown syntax (parser convert)",  "Syntax is incorrect"},

    {AE_SELECTED_ENTITY_OUT_OF_RANGE,      "Selected entity out of range",     "Selected entity is outside the bounds of the number of entities."},
    {AE_COORDINATES_OUT_OF_RANGE,          "Coordinates out of range",         "Coordinates outside the prescribed range."},
    {AE_VALUE_OUT_OF_RANGE,                "Value out of range",               "Value outside the presecribed range."},

    {AE_TOO_MANY_CLOSE_BRACES,             "Too many }",                       "You have closed too many braces. Go back to the code and see if there is an erroneous additional } in the code."},
    {AE_MAXIMUM_BRACES_REACHED,            "Maximum braces reached",           "Please contact tom at nobleape dot com"},
    {AE_FIRST_VALUE_FAILED,                "First value failed",               "Something is wrong with the first value of an equality, if or while operation. It could be the first and only value in this function."},
    {AE_SECOND_VALUE_FAILED,               "Second value failed",              "Something is wrong with the second number/variable value of an equality, if or while operation."},
    {AE_UNKNOWN_SYNTAX_MISSING_EQUALS,     "Unknown syntax (missing =)",       "Syntax is incorrect"},
    {AE_UNKNOWN_SYNTAX_NO_COMMAND,         "Unknown syntax (no command)",      "Syntax is incorrect"},
    {AE_WRONG_END,                         "Wrong end",                        "A bracket or colon was expected but not found."},

    {AE_LINE_START_INCORRECT,              "Line start incorrect",             "A line of code begins incorrectly. It could start with a number or an operator when if/while or a variable was expected."},
    {AE_OUTPUT_SET_AS_INPUT_VARIABLE,      "Output set as input variable",     "An output only variable is attempting to be set."},
    {AE_IF_WHILE_NOT_FOLLOWED_BY_BRACKET,  "if/while not followed by {",       "All if/while statements require a bracket following the if/while (allowing for any amount of whitespace too)."},
    {AE_FUNCTION_ISNT_VARIABLE,            "Function isn't variable",          "Function must not be a special term."},
    {AE_NON_FUNCTION_APPLIED,              "Non-function applied",             "Expecting a function."},
    {AE_FUNCTION_DEFINED_PRIOR,            "Function defined prior",           "Single function definition only."},
    {AE_FUNCTION_OUT_OF_RANGE,             "Function out of range",            "Function defined outside the range of the code presented."},
    {AE_WITHOUT_SEMICOLON,                 "Without ;",                        "Semi-colon required."},
    {AE_WITHOUT_OPEN_BRACE,                "Without {",                        "All if/while statements expect what is executed through the bracket enclosed statement being correct to be followed by inclusive braces { }. There is no single line if or while statements without { } in ApeScript."},

    {AE_FUNCTION_SETTING_FAILED,           "Function setting failed",          "Function could not be set."},
    {AE_ERROR_STARTING_MAIN,               "Error starting main",              "Main could not be started."},
    {AE_CODE_AFTER_MAIN,                   "Code after main",                  "All the code in ApeScript must exist before the end of main."},
    {AE_NO_CLOSE_BRACE_TO_END_OF_FILE,     "No } to end of file",              "Based on the final main function it is expected that the last meaningful character will be }."},
    {AE_CODE_OUTSIDE_FUNCTION,             "Code outside function",            "All code in ApeScript needs to exist within functions."},
    {AE_INPUT_VARIABLE_WITHOUT_EQUALS,     "Input variable without equals",    "All variables set require an equals following the variable."},
    {AE_ASSIGN_VALUE_FAILED,               "Assign value failed",              "Something is wrong with the variable set by an equality."},
    {AE_UNKNOWN_SYNTAX_FROM_INTERPRET,     "Unknown syntax (from interpret)",  "Syntax is incorrect"},
    {AE_NO_MAIN_CODE,                      "No main code",                     "APeScript requires a main function."},

    {AE_NO_ERROR, 0L, 0L}
};

extern n_int draw_error(n_string error_text);

/* "---1---2---3---4---5---6---7--" */ /* length of the errors */

#define	SHOW_ERROR(val)	(draw_error(val))

#define IO_LOWER_CHAR(value)   if(ASCII_UPPERCASE(value)) (value) += 'a' - 'A'

void  vect2_byte2(n_vect2 * converter, n_byte2 * input);
void  vect2_add(n_vect2 * equals, n_vect2 * initial, n_vect2 * second);
void  vect2_subtract(n_vect2 * equals, n_vect2 * initial, n_vect2 * second);
void  vect2_multiplier(
    n_vect2 * equals, n_vect2 * initial,
    n_vect2 * second, n_int multiplier, n_int divisor);
void  vect2_d(
    n_vect2 * initial, n_vect2 * second,
    n_int multiplier, n_int divisor);
n_int vect2_dot(
    n_vect2 * initial, n_vect2 * second,
    n_int multiplier, n_int divisor);
void  vect2_direction(n_vect2 * initial, n_byte direction, n_int divisor);
void  vect2_offset(n_vect2 * initial, n_int dx, n_int dy);
void  vect2_back_byte2(n_vect2 * converter, n_byte2 * output);

n_uint  math_hash(n_byte * values, n_uint length);
void    math_bilinear_512_4096(n_byte * side512, n_byte * data);
n_uint  math_newton_root(n_uint squ);
n_byte  math_turn_towards(n_int px, n_int py, n_byte fac, n_byte turn);
n_byte2 math_random(n_byte2 * local);
void    math_random3(n_byte2 * local);
n_byte  math_join(n_int sx, n_int sy, n_int dx, n_int dy, n_join * draw);
n_int   math_spread_byte(n_byte val);
void    math_patch(n_byte * local_map, n_byte * scratch,
                   n_patch * func, n_byte2 * arg,
                   n_int patch_bits,
                   n_byte refined_start, n_byte refined_end,
                   n_byte rotate);

n_byte     io_entry_execution(n_byte value);
void       io_lower(n_string value, n_int length);
void       io_whitespace(n_file * input);
void       io_audit_file(const noble_file_entry * format, n_byte section_to_audit);
void       io_search_file_format(const noble_file_entry * format, n_string compare);
void       io_string_write(n_string dest, n_string insert, n_int * pos);
n_int      io_read_bin(n_file * fil, n_byte * local_byte);
n_int      io_file_write(n_file * fil, n_byte byte);
void       io_file_reused(n_file * fil);
n_int      io_write(n_file * fil, n_string ch, n_byte new_line);
n_int      io_writenumber(n_file * fil, n_int loc_val, n_uint numer, n_uint denom);
n_int      io_length(n_string value, n_int max);
n_int      io_find(n_string check, n_int from, n_int max, n_string value_find, n_int value_find_length);
n_int      io_read_buff(n_file * fil, n_byte * data, const noble_file_entry * commands);
n_int      io_write_buff(n_file * fil, void * data, const noble_file_entry * commands, n_byte command_num, n_file_specific * func);
n_int      io_write_csv(n_file * fil, n_byte * data, const noble_file_entry * commands, n_byte command_num, n_byte initial) ;
void       io_copy(n_byte * from, n_byte * to, n_uint number);
void *     io_new(n_uint bytes);
void       io_free(void * ptr);
void *     io_new_range(n_uint memory_min, n_uint *memory_allocated);
n_file *   io_file_new(void);
void       io_file_free(n_file * file);
n_int      io_file_xml_open(n_file * file, n_string name);
n_int      io_file_xml_close(n_file * file, n_string name);
n_int      io_file_xml_string(n_file * file, n_string name, n_string string);
n_int      io_file_xml_int(n_file * file, n_string name, n_int number);
n_int      io_number(n_string number_string, n_int * actual_value, n_int * decimal_divisor);


n_int      io_command_line_execution(void);
n_int      io_quit(void * ptr, n_string response, n_console_output output_function);
n_int      io_help(void * ptr, n_string response, n_console_output output_function);
n_string   io_console_entry_clean(n_string string, n_int length);
n_string   io_console_entry(n_string string, n_int length);
void       io_console_out(n_string value);
n_int      io_console(void * ptr, noble_console_command * commands, n_console_input input_function, n_console_output output_function);
void       io_erase(n_byte * buf_offscr, n_uint nestop);
n_int      io_disk_read(n_file * local_file, n_string file_name);
n_int      io_disk_write(n_file * local_file, n_string file_name);
n_int      io_disk_append(n_file * local_file, n_string file_name);
n_int      io_disk_check(n_string file_name);
n_string * io_tab_delimit_to_n_string_ptr(n_file * tab_file, n_int * size_value, n_int * row_value);

/*
 This provides the land interface into entity but not the universe.

 This should reduce the use of universe.h in the entity .c/h files.
 */

/*	Land - Screen conversion */

#ifdef SMALL_LAND

#define MAP_BITS                      (8)

#else

#define MAP_BITS                      (9)

#endif

#define MAP_DIMENSION                 (1<<(MAP_BITS))
#define MAP_AREA                      (1<<(2*MAP_BITS))

#define APESPACE_TO_MAPSPACE(num)     ((num)>>6)

#ifndef SMALL_LAND

#define HI_RES_MAP_BITS               (12)

#define HI_RES_MAP_DIMENSION          (1<<(HI_RES_MAP_BITS))
#define HI_RES_MAP_AREA               (1<<(2*HI_RES_MAP_BITS))

#define APESPACE_TO_HR_MAPSPACE(num)  ((num)>>3)

#endif

#define MAPSPACE_TO_APESPACE(num)     ((num)<<6)

#define MAP_APE_RESOLUTION_SIZE       (MAPSPACE_TO_APESPACE(MAP_DIMENSION))
#define	APESPACE_BOUNDS               ((MAP_APE_RESOLUTION_SIZE)-1)

#define	APESPACE_CONFINED(num)        (n_byte2)((num)>APESPACE_BOUNDS ? APESPACE_BOUNDS : ((num)<0 ? 0 : (num)))
#define APESPACE_WRAP(num)            (n_byte2)((num + APESPACE_BOUNDS)&(APESPACE_BOUNDS-1))

#define LAND_TILE_EDGE                 (256)
#define	POSITIVE_LAND_COORD(num)       ((num+(3*MAP_DIMENSION))&(MAP_DIMENSION-1))
#define	POSITIVE_LAND_COORD_HIRES(num) ((num+(3*HI_RES_MAP_DIMENSION))&(HI_RES_MAP_DIMENSION-1))
#define NUMBER_LAND_TILES              (MAP_DIMENSION/LAND_TILE_EDGE)


#define LAND_DITHER(x,y,z)             (((x+y+z)&15)-(((x&y)|z)&7)-((x|(y&z))&7))

#define SECONDS_PER_SIMULATION_STEP (60)

#define	WEATHER_CLOUD		(32768)
#define	WEATHER_RAIN		(98304)

#define TIME_HOUR_MINUTES           (60)
#define TIME_DAY_MINUTES            (TIME_HOUR_MINUTES * 24)
#define TIME_MONTH_MINUTES          (TIME_DAY_MINUTES * 28)
#define TIME_YEAR_MINUTES           (TIME_MONTH_MINUTES * 13)
#define TIME_YEAR_DAYS              (7 * 52)				/*364 also = 13 * 28 */
#define	TIME_CENTURY_DAYS           (TIME_YEAR_DAYS * 100)
#define TIME_IN_DAYS(time)          (n_uint)(((time)[1]*TIME_CENTURY_DAYS)+(time)[0])

#define LUNAR_ORBIT_MINS            39312

#define	QUICK_LAND(local, px, py) (((local)->map)[POSITIVE_LAND_COORD(px)|(POSITIVE_LAND_COORD(py)<<MAP_BITS)])
#define	WATER_TEST(pz,w)				((pz)<(w))
#define	MAP_WATERTEST(local,px,py)		WATER_TEST(QUICK_LAND(local,(px),(py)),local->tide_level)

#define	WATER_MAP		      128

#define TIDE_AMPLITUDE_LUNAR  8
#define TIDE_AMPLITUDE_SOLAR  2

#define TIDE_MAX              (WATER_MAP + TIDE_AMPLITUDE_LUNAR + TIDE_AMPLITUDE_SOLAR)

/* Night/Day definitions */
#define IS_NIGHT(num)		((((num)>>5) <  (11))||(((num)>>5) >  (36)))
#define	IS_DAWNDUSK(num)	((((num)>>5) == (11))||(((num)>>5) == (36)))

/*! @struct
 @field atmosphere The atmosphere is represented by two 128 x 128 integer.
 buffers, one containing the static flow map and one containing the dynamic
 buffer showing the actual pressure map.
 @field total_pressure This is the normalized total pressure that is used
 like a pressure cooker release valve to stop a weather simulation explosion.
 @discussion As with a majority of the Noble Ape Simulation, the weather
 method is completely self-rolled.
 */
typedef struct
{
    n_c_int			total_pressure;
    n_c_int			atmosphere[ MAP_AREA / 2];
}
n_weather;

/*! @struct
 @field time          Time in minutes from midnight.
 @field date          Date including days and centuries.
 @field genetics      Generator seed used to create the landscape.
 @field map           Map for z height.
 @field weather       Convenience pointer used for bad weather line-of-sight.
 @discussion This structure provides the basics for the static land, time and tides.
 */
typedef	struct
{
    n_byte2     time;
    n_byte2     date[2];
    n_byte2     genetics[2];
    n_byte      tide_level;
    n_byte     *map;
}
n_land;


void  weather_init(n_weather * local_weather, n_land * local_land);
void  weather_wind_vector(n_weather * wea, n_int px, n_int py, n_int * wind_dx, n_int * wind_dy);
n_int weather_pressure(n_weather * wea, n_int px, n_int py);
n_int weather_temperature(n_land * local_land, n_weather * wea, n_int px, n_int py);
void  weather_wind_vector(n_weather * wea, n_int px, n_int py, n_int * wind_dx, n_int * wind_dy);
void  weather_cycle(n_weather * local_weather);
n_int weather_seven_values(n_land * local_land, n_weather * local_weather, n_int px, n_int py);

void  land_init(n_land * local, n_byte * scratch, n_byte2 start);
void  land_clear(n_land * local, KIND_OF_USE kind);
void  land_cycle(n_land * local_land);
void  land_vect2(n_vect2 * output, n_int * actual_z, n_land * local, n_vect2 * location);
n_int land_operator_interpolated(n_land * local_land, n_weather * local_weather, n_int locx, n_int locy, n_byte * kind);


/*0*/

enum PRIMARY_APESCRIPT
{
    VARIABLE_FUNCTION = 0,
    VARIABLE_RUN,
    VARIABLE_WHILE,
    VARIABLE_IF
};

enum SYNTAX_APESCRIPT
{
    SYNTAX_MINUS = 0,
    SYNTAX_ADDITION,
    SYNTAX_MULTIPLY,
    SYNTAX_AND,
    SYNTAX_XOR,
    SYNTAX_OR,

    SYNTAX_GREATER_THAN,
    SYNTAX_LESS_THAN,
    SYNTAX_EQUAL_TO,
    SYNTAX_NOT_EQUAL_TO,
    SYNTAX_CONDITIONAL_AND,
    SYNTAX_CONDITIONAL_OR,
    SYNTAX_DIVISION,

    SYNTAX_MODULUS,

    SYNTAX_BITSHIFT_RIGHT,
    SYNTAX_BITSHIFT_LEFT,

    SYNTAX_LESS_EQUAL,
    SYNTAX_GREATER_EQUAL,

    SYNTAX_EQUALS
};

enum SYNTAX_ADDITIONAL_BRAINCODE
{
    SYNTAX_MOVE = SYNTAX_EQUALS + 1,
    SYNTAX_JUMP_TO,
    SYNTAX_JUMP_EQUAL_ZERO,
    SYNTAX_DATA
};

#define ASCII_TEXT(num)		  ((ASCII_UPPERCASE(num) || ASCII_LOWERCASE(num)) || ((num) == '_'))

#define	ASCII_SEMICOLON(num)  ((num) == ';')

#define	ASCII_EQUAL(num)	  ((num) == '=')

#define	ASCII_BRACKET(num)    (((num) == '(')||((num) == ')'))
#define	ASCII_BRACES(num)     (((num) == '{')||((num) == '}'))

#define ASCII_LOGICAL(num)	  ((((num) == '&')||((num) == '|'))||(((num) == '^')||((num) == '!')))
#define ASCII_ARITHMETIC(num) ((((num) == '+')||((num) == '-'))||(((num) == '*')||((num) == '/')))

#define ASCII_DIRECTIONAL(num) (((num)=='<')||((num)=='>'))

#define	CODE_VALUE_REQUIRED(num)	(((num) == '=' || (num) == 'n') || ((num) == 't'))

#define	SIZEOF_NUMBER_WRITE      2

#define	INT_TO_BYTES(byt,num)	(byt)[0] = (n_byte)(((num)&255)); (byt)[1] = (n_byte)(((num)>>8)&255)
#define	BYTES_TO_INT(byt)		((byt)[0] | ((byt)[1] << 8))

#define	VARIABLE_INPUT(num,code)		((num)>((code)->input_greater))
#define	VARIABLE_SPECIAL(num,code)	    ((num)<((code)->special_less))

#ifndef ABS
#define ABS(a)	                        (((a) < 0) ? -(a) : (a))
#endif

#define NUMBER_MAX 				256
#define	VARIABLE_MAX			256

#define	BRACES_MAX			    16
#define SIZE_OF_EVALUATE	    8  /* (tA=XtB) */

#define CYCLE_COUNT_RESET	    4096

#define	MAIN_NOT_RUN		    0
#define	MAIN_RUN			    1

/*! @typedef
@field code The pointer to the n_interpret struct.
@field kind The variable in question - this could be thought of as an index to a
variable array.
@field value The value to set the variable in question.
@return Any error that is reported.
@discussion The two primay interfaces in ApeScript relate to the getting and
setting of information. This function covers the setting of information.
*/
typedef n_int (script_input )(void * code, n_byte kind, n_int value);

/*! @typedef
@field code The pointer to the n_interpret struct.
@field kind The pointer has two values either a variable or a number.
@field value The resultant value.
@return Any error that is reported.
@discussion The two primay interfaces in ApeScript relate to the getting and
setting of information. This function covers the getting of information.
*/
typedef n_int (script_output)(void * code, n_byte * kind, n_int * number);

typedef void (script_external)(void * code, void * structure, n_int identifier);

#define	VARIABLE_WIDTH		    32

typedef	n_byte				    variable_string[VARIABLE_WIDTH];


/*! @struct
@field evaluate The length of the evaluated string.
@field braces_start The location where the braces start.
@discussion This structure is used for the evaluation of if/then or while
checks in ApeScript and it shows where the braces code should return to in
the case of a while loop.
*/
typedef struct
{
    n_byte			 evaluate[SIZE_OF_EVALUATE];
    n_int			 braces_start;
} n_brace;

/*! @struct
@field binary_code The file structure that contains the binary code to be interpreted.
@field number_buffer The array of const-like stored number values.
@field variable_strings The array linked to the strings used for parsing and linking to the variable index.
@field variable_references The array of values of the variable index.
@field braces_count The number of braces currently in use.
@field braces The array of braces structs currently in use.
@field main_status Whether a main section of code has been reached or exists in the execution. (CHECK FOR VALIDITY)
@field main_entry The variable index that is classified as main in ApeScript. In this impementation
it is the function name "being".
@field input_greater Everything greater than this variable index number is an input variable.
@field special_less Everything less than this variable index number is a special variable.
@field sc_input This is the function pointer to the input handling function which is external to ApeScript.
@field sc_output This is the function pointer to the output handling function which is external to ApeScript.
@field specific This defines which specific ApeScript instance is being run and is defined outside ApeScript and
remains constant through the execution cycle.
@field location         DOX_TEXT_MISSING
@field leave            DOX_TEXT_MISSING
@field localized_leave  DOX_TEXT_MISSING
@discussion A good majority of this data should be considered opaque and not altered through the course of
normal use of ApeScript. It may be interesting to change some of this information in experimenting with ApeScript
for future development. It is planned in the future that the front of the ApeScript struct will be identical to the
n_file struct for ease of transitioning between both structs.
*/
typedef struct
{
    n_file			*binary_code;

    n_int			 number_buffer[NUMBER_MAX];

    variable_string *variable_strings;
    n_int		    *variable_references;

    n_int	  	     braces_count;
    n_brace		     braces[BRACES_MAX];
    n_byte		     main_status;

    n_int		     main_entry;
    n_int		     input_greater;
    n_int		     special_less;

    script_input    *sc_input;
    script_output   *sc_output;

    n_int			specific;

    n_int           location;
    n_int           leave;
    n_int           localized_leave;
} n_interpret;

/* used for stripping ApeScript errors for documentation */

n_int io_apescript_error(AE_ENUM value);

n_interpret *	parse_convert(n_file * input, n_int main_entry, variable_string * variables);

void  interpret_cleanup(n_interpret * to_clean);
n_int interpret_cycle(n_interpret * code, n_int exit_offset, void * structure, n_int identifier, script_external * start, script_external * end);

#ifdef SKIM_TO_BRIANCODE
void skim_show(n_file * binary_code);
#endif

#ifdef	SCRIPT_DEBUG

n_file * scdebug_file_ready(void);
void     scdebug_file_cleanup(void);

void     scdebug_string(n_string string);
void     scdebug_int(n_int number);
void     scdebug_newline(void);
void     scdebug_tabstep(n_int steps);
n_string scdebug_variable(n_int variable);
void     scdebug_writeon(n_byte value);

#define	SC_DEBUG_STRING(string)	scdebug_string(string)
#define SC_DEBUG_NUMBER(number) scdebug_int(number)
#define	SC_DEBUG_NEWLINE		scdebug_newline()
#define	SC_DEBUG_UP				scdebug_tabstep(1)
#define	SC_DEBUG_DOWN			scdebug_tabstep(-1)
#define	SC_DEBUG_ON				scdebug_writeon(1)
#define SC_DEBUG_OFF			scdebug_writeon(0)

#else

#define	SC_DEBUG_STRING(string)	/* string */
#define SC_DEBUG_NUMBER(number) /* number */
#define	SC_DEBUG_NEWLINE		/* */
#define	SC_DEBUG_UP				/* */
#define	SC_DEBUG_DOWN			/* */
#define	SC_DEBUG_ON				/* */
#define SC_DEBUG_OFF			/* */

#endif

/*NOBLEMAKE END=""*/

/*NOBLEMAKE SET="io.c"*/
/*NOBLEMAKE SET="nthreads.c"*/
/*NOBLEMAKE SET="parse.c"*/
/*NOBLEMAKE SET="interpret.c"*/
/*NOBLEMAKE SET="math.c"*/



