/****************************************************************

 script_tests.c

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
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Include the headers we're testing
#include "toolkit.h"
#include "script.h"

// Simple testing framework
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

n_int draw_error(n_constant_string error_text, n_constant_string location, n_int line_number)
{
    printf("ERROR: %s @ %s %ld\n",(const n_string) error_text, location, line_number);
    return -1;
}

#define TEST_ASSERT(condition, message) \
    do { \
        tests_run++; \
        if (condition) { \
            tests_passed++; \
            printf("PASS: %s\n", message); \
        } else { \
            tests_failed++; \
            printf("FAIL: %s (line %d)\n", message, __LINE__); \
        } \
    } while(0)

#define TEST_FUNCTION(func_name) \
    printf("\n--- Testing %s ---\n", #func_name); \
    func_name()

// Mock implementations for testing
static n_int test_script_input(void *individual, n_byte kind, n_int value) {
    // Simple mock that just stores the value
    static n_int stored_values[256] = {0};
    if (kind < 256) {
        stored_values[kind] = value;
        return 0; // Success
    }
    return -1; // Error
}

static n_int test_script_output(void *code, void *individual, n_byte *kind, n_int *number) {
    // Simple mock that returns stored values
    static n_int stored_values[256] = {0};
    if (*kind < 256) {
        *number = stored_values[*kind];
        return 0; // Success
    }
    return -1; // Error
}

static void test_script_external_start(void *individual, void *structure, void *data) {
    // Mock external function - could increment a counter or set a flag
    printf("External start function called\n");
}

static void test_script_external_end(void *individual, void *structure, void *data) {
    // Mock external function - could decrement a counter or clear a flag
    printf("External end function called\n");
}

// Test error enum and structure
void test_apescript_errors(void) {
    // Test that error array is properly terminated
    int i = 0;
    while (apescript_errors[i].enum_value != AE_NO_ERROR) {
        TEST_ASSERT(apescript_errors[i].error_string != NULL, 
                   "Error string should not be NULL");
        TEST_ASSERT(apescript_errors[i].help_string != NULL, 
                   "Help string should not be NULL");
        i++;
    }
    
    // Test specific error values
    TEST_ASSERT(apescript_errors[0].enum_value == AE_UNKNOWN_ERROR, 
               "First error should be AE_UNKNOWN_ERROR");
    
    // Test that we have the expected number of errors
    TEST_ASSERT(i == AE_NUMBER_ERRORS - 1, 
               "Number of errors should match AE_NUMBER_ERRORS - 1");
}

// Test n_brace structure
void test_n_brace_structure(void) {
    n_brace test_brace;
    
    // Test structure size and initialization
    memset(&test_brace, 0, sizeof(n_brace));
    TEST_ASSERT(sizeof(test_brace.evaluate) == SIZE_OF_EVALUATE, 
               "Evaluate array should be SIZE_OF_EVALUATE bytes");
    
    // Test setting values
    test_brace.braces_start = 100;
    TEST_ASSERT(test_brace.braces_start == 100, 
               "Should be able to set braces_start value");
    
    // Test evaluate array
    test_brace.evaluate[0] = 'A';
    test_brace.evaluate[SIZE_OF_EVALUATE - 1] = 'Z';
    TEST_ASSERT(test_brace.evaluate[0] == 'A' && 
               test_brace.evaluate[SIZE_OF_EVALUATE - 1] == 'Z',
               "Should be able to set evaluate array values");
}

// Test n_interpret structure
void test_n_interpret_structure(void) {
    n_interpret test_interpret;
    variable_string test_variables[10];
    
    memset(&test_interpret, 0, sizeof(n_interpret));
    memset(test_variables, 0, sizeof(test_variables));
    
    // Test basic field assignments
    test_interpret.main_entry = 42;
    test_interpret.input_greater = 100;
    test_interpret.special_less = 10;
    test_interpret.variable_strings = test_variables;
    test_interpret.sc_input = test_script_input;
    test_interpret.sc_output = test_script_output;
    
    TEST_ASSERT(test_interpret.main_entry == 42, 
               "Should be able to set main_entry");
    TEST_ASSERT(test_interpret.input_greater == 100, 
               "Should be able to set input_greater");
    TEST_ASSERT(test_interpret.special_less == 10, 
               "Should be able to set special_less");
    TEST_ASSERT(test_interpret.variable_strings == test_variables, 
               "Should be able to set variable_strings pointer");
    TEST_ASSERT(test_interpret.sc_input == test_script_input, 
               "Should be able to set script_input function pointer");
    TEST_ASSERT(test_interpret.sc_output == test_script_output, 
               "Should be able to set script_output function pointer");
    
    // Test number buffer
    test_interpret.number_buffer[0] = 123;
    test_interpret.number_buffer[NUMBER_MAX - 1] = 456;
    TEST_ASSERT(test_interpret.number_buffer[0] == 123 && 
               test_interpret.number_buffer[NUMBER_MAX - 1] == 456,
               "Should be able to access number_buffer array");
}

// Test n_individual_interpret structure
void test_n_individual_interpret_structure(void) {
    n_individual_interpret test_individual;
    
    memset(&test_individual, 0, sizeof(n_individual_interpret));
    
    // Test basic fields
    test_individual.interpret_location = 200;
    test_individual.leave = 1;
    test_individual.localized_leave = 0;
    test_individual.braces_count = 5;
    test_individual.main_status = MAIN_RUN;
    
    TEST_ASSERT(test_individual.interpret_location == 200, 
               "Should be able to set interpret_location");
    TEST_ASSERT(test_individual.leave == 1, 
               "Should be able to set leave flag");
    TEST_ASSERT(test_individual.localized_leave == 0, 
               "Should be able to set localized_leave flag");
    TEST_ASSERT(test_individual.braces_count == 5, 
               "Should be able to set braces_count");
    TEST_ASSERT(test_individual.main_status == MAIN_RUN, 
               "Should be able to set main_status");
    
    // Test arrays
    test_individual.variable_references[0] = 10;
    test_individual.variable_references[VARIABLE_MAX - 1] = 20;
    TEST_ASSERT(test_individual.variable_references[0] == 10 && 
               test_individual.variable_references[VARIABLE_MAX - 1] == 20,
               "Should be able to access variable_references array");
    
    // Test braces array
    test_individual.braces[0].braces_start = 300;
    test_individual.braces[BRACES_MAX - 1].braces_start = 400;
    TEST_ASSERT(test_individual.braces[0].braces_start == 300 && 
               test_individual.braces[BRACES_MAX - 1].braces_start == 400,
               "Should be able to access braces array");
}

// Test macro definitions
void test_macro_definitions(void) {
    // Test ASCII macros
    TEST_ASSERT(ASCII_QUOTE('"'), "ASCII_QUOTE should recognize quote character");
    TEST_ASSERT(!ASCII_QUOTE('a'), "ASCII_QUOTE should not recognize non-quote");
    
    TEST_ASSERT(ASCII_TEXT('a'), "ASCII_TEXT should recognize lowercase letter");
    TEST_ASSERT(ASCII_TEXT('Z'), "ASCII_TEXT should recognize uppercase letter");
    TEST_ASSERT(ASCII_TEXT('_'), "ASCII_TEXT should recognize underscore");
    TEST_ASSERT(!ASCII_TEXT('1'), "ASCII_TEXT should not recognize digit");
    
    TEST_ASSERT(ASCII_SEMICOLON(';'), "ASCII_SEMICOLON should recognize semicolon");
    TEST_ASSERT(!ASCII_SEMICOLON(':'), "ASCII_SEMICOLON should not recognize colon");
    
    TEST_ASSERT(ASCII_EQUAL('='), "ASCII_EQUAL should recognize equals sign");
    TEST_ASSERT(!ASCII_EQUAL('!'), "ASCII_EQUAL should not recognize exclamation");
    
    // Test bracket macros
    TEST_ASSERT(ASCII_BRACKET('('), "ASCII_BRACKET should recognize open paren");
    TEST_ASSERT(ASCII_BRACKET(')'), "ASCII_BRACKET should recognize close paren");
    TEST_ASSERT(!ASCII_BRACKET('['), "ASCII_BRACKET should not recognize square bracket");
    
    TEST_ASSERT(ASCII_BRACES('{'), "ASCII_BRACES should recognize open brace");
    TEST_ASSERT(ASCII_BRACES('}'), "ASCII_BRACES should recognize close brace");
    TEST_ASSERT(!ASCII_BRACES('('), "ASCII_BRACES should not recognize paren");
    
    // Test logical and arithmetic macros
    TEST_ASSERT(ASCII_LOGICAL('&'), "ASCII_LOGICAL should recognize ampersand");
    TEST_ASSERT(ASCII_LOGICAL('|'), "ASCII_LOGICAL should recognize pipe");
    TEST_ASSERT(ASCII_LOGICAL('^'), "ASCII_LOGICAL should recognize caret");
    TEST_ASSERT(ASCII_LOGICAL('!'), "ASCII_LOGICAL should recognize exclamation");
    
    TEST_ASSERT(ASCII_ARITHMETIC('+'), "ASCII_ARITHMETIC should recognize plus");
    TEST_ASSERT(ASCII_ARITHMETIC('-'), "ASCII_ARITHMETIC should recognize minus");
    TEST_ASSERT(ASCII_ARITHMETIC('*'), "ASCII_ARITHMETIC should recognize multiply");
    TEST_ASSERT(ASCII_ARITHMETIC('/'), "ASCII_ARITHMETIC should recognize divide");
}

// Test variable macros
void test_variable_macros(void) {
    n_interpret test_code;
    test_code.input_greater = 100;
    test_code.special_less = 10;
    
    // Test VARIABLE_INPUT macro
    TEST_ASSERT(VARIABLE_INPUT(150, &test_code), 
               "VARIABLE_INPUT should return true for value > input_greater");
    TEST_ASSERT(!VARIABLE_INPUT(50, &test_code), 
               "VARIABLE_INPUT should return false for value <= input_greater");
    
    // Test VARIABLE_SPECIAL macro
    TEST_ASSERT(VARIABLE_SPECIAL(5, &test_code), 
               "VARIABLE_SPECIAL should return true for value < special_less");
    TEST_ASSERT(!VARIABLE_SPECIAL(15, &test_code), 
               "VARIABLE_SPECIAL should return false for value >= special_less");
}

// Test constants
void test_constants(void) {
    TEST_ASSERT(NUMBER_MAX == 256, "NUMBER_MAX should be 256");
    TEST_ASSERT(VARIABLE_MAX == 256, "VARIABLE_MAX should be 256");
    TEST_ASSERT(BRACES_MAX == 16, "BRACES_MAX should be 16");
    TEST_ASSERT(VARIABLE_WIDTH == 32, "VARIABLE_WIDTH should be 32");
    TEST_ASSERT(CYCLE_COUNT_RESET == 4096, "CYCLE_COUNT_RESET should be 4096");
    TEST_ASSERT(MAIN_NOT_RUN == 0, "MAIN_NOT_RUN should be 0");
    TEST_ASSERT(MAIN_RUN == 1, "MAIN_RUN should be 1");
    
    // Test SIZE_OF_EVALUATE calculation
    n_int expected_size = (SIZEOF_NUMBER_WRITE + SIZEOF_NUMBER_WRITE + 1);
    TEST_ASSERT(SIZE_OF_EVALUATE == expected_size, 
               "SIZE_OF_EVALUATE should match calculated size");
}

// Test enums
void test_enums(void) {
    // Test PRIMARY_APESCRIPT enum
    TEST_ASSERT(VARIABLE_FUNCTION == 0, "VARIABLE_FUNCTION should be 0");
    TEST_ASSERT(VARIABLE_RUN == 1, "VARIABLE_RUN should be 1");
    TEST_ASSERT(VARIABLE_WHILE == 2, "VARIABLE_WHILE should be 2");
    TEST_ASSERT(VARIABLE_IF == 3, "VARIABLE_IF should be 3");
    
    // Test SYNTAX_APESCRIPT enum values
    TEST_ASSERT(SYNTAX_MINUS == 0, "SYNTAX_MINUS should be 0");
    TEST_ASSERT(SYNTAX_ADDITION == 1, "SYNTAX_ADDITION should be 1");
    TEST_ASSERT(SYNTAX_EQUALS == 18, "SYNTAX_EQUALS should be last basic syntax");
    
    // Test SYNTAX_ADDITIONAL_BRAINCODE enum
    TEST_ASSERT(SYNTAX_MOVE == SYNTAX_EQUALS + 1, 
               "SYNTAX_MOVE should follow SYNTAX_EQUALS");
    
    // Test APESCRIPT_INTERPRET_TYPES enum
    TEST_ASSERT(APESCRIPT_OPEN_BRACKET == '(', 
               "APESCRIPT_OPEN_BRACKET should equal '('");
    TEST_ASSERT(APESCRIPT_CLOSE_BRACKET == ')', 
               "APESCRIPT_CLOSE_BRACKET should equal ')'");
    TEST_ASSERT(APESCRIPT_SEMICOLON == ';', 
               "APESCRIPT_SEMICOLON should equal ';'");
}

// Test function pointer types
void test_function_pointers(void) {
    // Test that we can assign and call function pointers
    script_input *input_func = test_script_input;
    script_output *output_func = test_script_output;
    script_external *ext_func = test_script_external_start;
    
    TEST_ASSERT(input_func != NULL, "Should be able to assign script_input function");
    TEST_ASSERT(output_func != NULL, "Should be able to assign script_output function");
    TEST_ASSERT(ext_func != NULL, "Should be able to assign script_external function");
    
    // Test calling the functions
    n_int result = input_func(NULL, 1, 42);
    TEST_ASSERT(result == 0, "Mock input function should return success");
    
    n_byte kind = 1;
    n_int number = 0;
    result = output_func(NULL, NULL, &kind, &number);
    TEST_ASSERT(result == 0, "Mock output function should return success");
}

// Test io_int_to_bytes and io_bytes_to_int functions
void test_byte_conversion(void) {
    n_byte bytes[SIZEOF_NUMBER_WRITE];
    n_int original_value = 0x12345678;
    n_int converted_value;
    
    // Test conversion to bytes
    io_int_to_bytes(original_value, bytes);
    
    // Test conversion back to int
    converted_value = io_bytes_to_int(bytes);
    
    TEST_ASSERT(converted_value == original_value, 
               "Byte conversion should be reversible");
    
    // Test with negative number
    original_value = -1000;
    io_int_to_bytes(original_value, bytes);
    converted_value = io_bytes_to_int(bytes);
    
    TEST_ASSERT(converted_value == original_value, 
               "Negative byte conversion should be reversible");
    
    // Test with zero
    original_value = 0;
    io_int_to_bytes(original_value, bytes);
    converted_value = io_bytes_to_int(bytes);
    
    TEST_ASSERT(converted_value == original_value, 
               "Zero byte conversion should work correctly");
}

// Test error reporting function
void test_apescript_error(void) {
    n_individual_interpret individual;
    memset(&individual, 0, sizeof(individual));
    
    // Test error reporting (should return error code)
    n_int error_result = apescript_error(&individual, AE_UNKNOWN_ERROR, __FILE__, __LINE__);
    TEST_ASSERT(error_result == AE_UNKNOWN_ERROR, 
               "apescript_error should return the error enum value");
    
    // Test with different error
    error_result = apescript_error(&individual, AE_NUMBER_EXPECTED, __FILE__, __LINE__);
    TEST_ASSERT(error_result == AE_NUMBER_EXPECTED, 
               "apescript_error should return AE_NUMBER_EXPECTED");
}

// Main test runner
int main(void) {
    printf("Starting ApeScript Unit Tests\n");
    printf("============================\n");
    
    TEST_FUNCTION(test_apescript_errors);
    TEST_FUNCTION(test_n_brace_structure);
    TEST_FUNCTION(test_n_interpret_structure);
    TEST_FUNCTION(test_n_individual_interpret_structure);
    TEST_FUNCTION(test_macro_definitions);
    TEST_FUNCTION(test_variable_macros);
    TEST_FUNCTION(test_constants);
    TEST_FUNCTION(test_enums);
    TEST_FUNCTION(test_function_pointers);
    TEST_FUNCTION(test_byte_conversion);
    TEST_FUNCTION(test_apescript_error);
    
    printf("\n============================\n");
    printf("Test Results:\n");
    printf("Total tests run: %d\n", tests_run);
    printf("Tests passed:    %d\n", tests_passed);
    printf("Tests failed:    %d\n", tests_failed);
    
    if (tests_failed == 0) {
        printf("All tests PASSED!\n");
        return 0;
    } else {
        printf("Some tests FAILED!\n");
        return 1;
    }
}

/* Compilation instructions:
 * gcc -o test_script test_script.c script.c toolkit.c -I. -DSCRIPT_DEBUG
 * 
 * Note: You'll need to link against the actual implementation files
 * (script.c, toolkit.c) and ensure all dependencies are available.
 * The test assumes the existence of the implementation functions.
 */
