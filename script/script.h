/****************************************************************

 script.h

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

/*! \file   script.h
 *  \brief  This is the interface between ApeScript and the ApeSDK.
 */

#ifndef _SCRIPT_H_
#define _SCRIPT_H_
/*	Variable Definitions */


#define	 SCRIPT_DEBUG             /* Add all the runtime debug */
#undef   ROUGH_CODE_OUT           /* printf outputs the interpret stream in character number format */

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
    n_constant_string error_string;
    n_constant_string help_string;
} n_ae_error;

/** \brief apescript_errors track the errors in ApeScript and additional text for user-manual level documentation */
static const n_ae_error apescript_errors[] =
{
    {AE_UNKNOWN_ERROR,                     "Unknown error",                    "Please contact tom at apesim dot io"},
    {AE_NUMBER_EXPECTED,                   "Number expected",                  "A non-numeric character is included in a number string."},
    {AE_NUMBER_OUT_OF_RANGE,               "Number out of range",              "Number does not fit in the range"},
    {AE_MAXIMUM_NUMBERS_REACHED,           "Maximum numbers reached",          "Please contact tom at apesim dot io"},
    {AE_MAXIMUM_SCRIPT_SIZE_REACHED,       "Maximum script size reached",      "Please contact tom at apesim dot io"},
    {AE_MAXIMUM_VARIABLES_REACHED,         "Maximum variables reached",        "Please contact tom at apesim dot io"},
    {AE_UNKNOWN_SYNTAX_PARSER_BUFFER,      "Unknown syntax (parser buffer)",   "Syntax is incorrect"},
    {AE_UNKNOWN_SYNTAX_PARSER_CONVERT,     "Unknown syntax (parser convert)",  "Syntax is incorrect"},

    {AE_SELECTED_ENTITY_OUT_OF_RANGE,      "Selected entity out of range",     "Selected entity is outside the bounds of the number of entities."},
    {AE_COORDINATES_OUT_OF_RANGE,          "Coordinates out of range",         "Coordinates outside the prescribed range."},
    {AE_VALUE_OUT_OF_RANGE,                "Value out of range",               "Value outside the presecribed range."},

    {AE_TOO_MANY_CLOSE_BRACES,             "Too many }",                       "You have closed too many braces. Go back to the code and see if there is an erroneous additional } in the code."},
    {AE_MAXIMUM_BRACES_REACHED,            "Maximum braces reached",           "Please contact tom at apesim dot io"},
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
    {AE_NO_MAIN_CODE,                      "No main code",                     "ApeScript requires a main function."},

    {AE_NO_ERROR, 0L, 0L}
};


/* "---1---2---3---4---5---6---7--" */ /* length of the errors */

#define APESCRIPT_ERROR(individual, value) (apescript_error(individual, value, __FILE__, __LINE__))

#define SHOW_ERROR_FILE_LINE(val, file, line) (draw_error(val, file, line))

#define	SHOW_ERROR(val)	(draw_error(val, __FILE__, __LINE__))

#define IO_LOWER_CHAR(value)   if(ASCII_UPPERCASE(value)) (value) += 'a' - 'A'

typedef enum
{
    VARIABLE_FUNCTION = 0,
    VARIABLE_RUN,
    VARIABLE_WHILE,
    VARIABLE_IF
} PRIMARY_APESCRIPT;

typedef enum
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
} SYNTAX_APESCRIPT;

typedef enum
{
    SYNTAX_MOVE = SYNTAX_EQUALS + 1,
    SYNTAX_JUMP_TO,
    SYNTAX_JUMP_EQUAL_ZERO,
    SYNTAX_DATA
} SYNTAX_ADDITIONAL_BRAINCODE;

typedef enum
{
    APESCRIPT_OPEN_BRACKET =  ( '(' ),
    APESCRIPT_CLOSE_BRACKET = ( ')' ),
    APESCRIPT_OPEN_BRACE =    ( '{' ),
    APESCRIPT_CLOSE_BRACE =   ( '}' ),
    APESCRIPT_OPERATOR =      ( '=' ),
    APESCRIPT_NUMBER =        ( 'n' ),
    APESCRIPT_TEXT =          ( 't' ),
    APESCRIPT_SEMICOLON =     ( ';' ),
    APESCRIPT_FAILURE =       ( 'F' ),
    APESCRIPT_FUNCTION =      ( 'f' ),
    APESCRIPT_RUN =           ( 'r' ),
    APESCRIPT_STRING =        ( '"' )
} APESCRIPT_INTERPRET_TYPES;

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

#define	VARIABLE_INPUT(num,code)		((num)>((code)->input_greater))
#define	VARIABLE_SPECIAL(num,code)	    ((num)<((code)->special_less))

#define NUMBER_MAX 				256
#define	VARIABLE_MAX			256

#define	BRACES_MAX			    16
#define SIZE_OF_EVALUATE	    (SIZEOF_NUMBER_WRITE+SIZEOF_NUMBER_WRITE+1)  /* (tA=XtB) */

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
typedef n_int ( script_input )( void *individual, n_byte kind, n_int value );

/*! @typedef
@field code The pointer to the n_interpret struct.
@field kind The pointer has two values either a variable or a number.
@field value The resultant value.
@return Any error that is reported.
@discussion The two primay interfaces in ApeScript relate to the getting and
setting of information. This function covers the getting of information.
*/
typedef n_int ( script_output )( void *code, void *individual, n_byte *kind, n_int *number );

typedef void ( script_external )( void *individual, void *structure, void *data );

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


typedef struct
{
    n_file			*binary_code;
    n_int			 number_buffer[NUMBER_MAX]; /* per entry */
    variable_string *variable_strings;
    n_int		     main_entry;
    n_int		     input_greater;
    n_int		     special_less;
    script_input    *sc_input;
    script_output   *sc_output;
} n_interpret;

typedef struct
{
    n_int           interpret_location; /* per entry */
    n_int           leave;              /* per entry */
    n_int           localized_leave;    /* per entry */
    void           *interpret_data;     /* per entry */
    n_int		    variable_references[VARIABLE_MAX]; /* per entry */
    n_int	  	    braces_count;        /* per entry */
    n_brace		    braces[BRACES_MAX];  /* per entry */
    n_byte		    main_status;         /* per entry */
} n_individual_interpret;


/* used for stripping ApeScript errors for documentation */

n_int apescript_error( n_individual_interpret *individual, AE_ENUM value, n_constant_string location, n_int line_number );

n_interpret 	*parse_convert( n_file *input, n_int main_entry, variable_string *variables );

void interpret_individual( n_individual_interpret *individual );

void  interpret_cleanup( n_interpret **to_clean );
n_int interpret_cycle( n_interpret *code, n_individual_interpret *individual, n_int exit_offset,
                       void *structure, void *data,
                       script_external *start, script_external *end );


#ifdef	SCRIPT_DEBUG

n_file *scdebug_file_ready( void );
void     scdebug_file_cleanup( void );

void     scdebug_string( void *ptr, n_constant_string string );
void     scdebug_int( void *ptr, n_int number );
void     scdebug_newline( void *ptr );
void     scdebug_tabstep( void *ptr, n_int steps );
n_string scdebug_variable( n_int variable );

void     scdebug_writeon( void *ptr );
void     scdebug_writeoff( void *ptr );

#define	SC_DEBUG_STRING(ptr, string) scdebug_string(ptr, string)
#define SC_DEBUG_NUMBER(ptr, number) scdebug_int(ptr, number)
#define	SC_DEBUG_NEWLINE(ptr)		 scdebug_newline(ptr)
#define	SC_DEBUG_UP(ptr)			 scdebug_tabstep(ptr,1)
#define	SC_DEBUG_DOWN(ptr)			 scdebug_tabstep(ptr,-1)
#define	SC_DEBUG_ON(ptr)             scdebug_writeon(ptr)
#define SC_DEBUG_OFF(ptr)			 scdebug_writeoff(ptr)

#else

#define	SC_DEBUG_STRING(string)	/* string */
#define SC_DEBUG_NUMBER(number) /* number */
#define	SC_DEBUG_NEWLINE		/* */
#define	SC_DEBUG_UP				/* */
#define	SC_DEBUG_DOWN			/* */
#define	SC_DEBUG_ON(ptr)		/* ptr */
#define SC_DEBUG_OFF			/* */

#endif

#endif /* _SCRIPT_H_ */

