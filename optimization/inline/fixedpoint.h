#ifndef _FIXEDPOINT_H_
#define _FIXEDPOINT_H_

/**
 * @file fixedpoint.h
 * @brief Basic fixed point math library for use on systems without a floating point unit
 * @details This library is designed to be used on systems without a floating point unit. Specifically ARMv6 based processors.
 *          Written for University of Victoria's Software Engineering 440 class.
 * @version 0.1
 */

/*
TODO: Format description Q17.15 (17 bits for integer part, 15 bits for fractional part)

(NOTE: 1):  Arm registers are 32 bits (in armv6) wide and support up to 32*32 multiplication (64 bit result)
            16 bits could also be used if reduced precision is acceptable

(NOTE: 2):  17 bits was picked so the full input range could be directly represented
            Range: [-65536,65535]

(NOTE: 3):  15 bits were dedicated to the fractional part to allow for a reasonable precision
            Resolution: 1 / 2^15 ~= 0.00003

(NOTE: 4):  Addition and subtraction can be done directly with the built in operators
*/

#define BIT_WIDTH 32                               // (NOTE: 1)
#define INTEGER_BITS 17                            // 17 bits for integer part, (NOTE: 2)
#define FRACTIONAL_BITS (BIT_WIDTH - INTEGER_BITS) // 15 bits for fractional part, (NOTE: 3)
#define STRING_DECIMALS 8                          // Number of decimal places to print

/*
    Redefine standard types to improve code clarity and avoid using default multiplication, division, etc.
*/
#include <stdint.h>

typedef int32_t fixedpoint_t;
typedef uint32_t ufixedpoint_t;
typedef int64_t long_fixedpoint_t;
typedef uint64_t ulong_fixedpoint_t;

/*
    Define bit masks for integer and fractional parts
*/

/*
    Define commonly used constants
*/
#define FIXEDPOINT_ZERO (fixedpoint_t)0
#define FIXEDPOINT_ONE (fixedpoint_t)(1 << FRACTIONAL_BITS)
#define FIXEDPOINT_TWO (fixedpoint_t)(FIXEDPOINT_ONE + FIXEDPOINT_ONE)
#define FIXEDPOINT_HALF (fixedpoint_t)(1 << (FRACTIONAL_BITS - 1))

#define FIXEDPOINT_PI fixedpoint_from_real(3.1415926535897932)
#define FIXEDPOINT_SQRT2 fixedpoint_from_real(1.4142135623730951)

/*
    Define commonly used macros
*/
#define fixedpoint_from_real(Val) (fixedpoint_t)((Val)*FIXEDPOINT_ONE + ((Val) >= 0 ? 0.5 : -0.5)) // Rounding conversion
#define fixedpoint_from_int(Val) (fixedpoint_t)(Val << FRACTIONAL_BITS)                            // Conversion from integer

#define fixedpoint_to_int(Val) (int32_t)(Val >> FRACTIONAL_BITS) // Truncating conversion to integer

#define fixedpoint_fractional_part(Val) (Val & ((1 << FRACTIONAL_BITS) - 1)) // Get fractional part

/*
    Define commonly used functions
*/
inline fixedpoint_t fixedpoint_add(fixedpoint_t a, fixedpoint_t b)
{
    // (NOTE: 4)
    return a + b;
}

inline fixedpoint_t fixedpoint_sub(fixedpoint_t a, fixedpoint_t b)
{
    // (NOTE: 4)
    return a - b;
}

inline fixedpoint_t fixedpoint_mul(fixedpoint_t a, fixedpoint_t b)
{
    // First cast up to long to avoid overflow, then shift out the added fractional bits
    return (fixedpoint_t)(((long_fixedpoint_t)a * (long_fixedpoint_t)b) >> FRACTIONAL_BITS);
}

inline fixedpoint_t fixedpoint_div(fixedpoint_t a, fixedpoint_t b)
{
    // First cast up to long to avoid overflow
    return (((long_fixedpoint_t)a << FRACTIONAL_BITS) / (long_fixedpoint_t)b);
}

/*
    Define printing functions
*/
inline char *fixedpoint_str(fixedpoint_t a)
{
    static char str[32]; // Arbitrarily chosen string size

    int count = 0;
    int str_pos = 0;
    char tmp[12] = {0};
    ulong_fixedpoint_t fractional;
    ulong_fixedpoint_t integer;

    const ulong_fixedpoint_t one = (ulong_fixedpoint_t)1 << BIT_WIDTH;
    const ulong_fixedpoint_t mask = one - 1;

    // First place the negative sign if needed and negate the number
    if (a < 0)
    {
        str[str_pos++] = '-';
        a *= -1;
    }

    // Calculate the base 10 integer representation
    integer = fixedpoint_to_int(a);
    do
    {
        tmp[count++] = '0' + integer % 10; // last digit of integer part added to the value of char '0' to get the ascii value of the digit
        integer /= 10;                     // divide the decimal number by 10 to remove the last digit
    } while (integer != 0);                // repeat until the integer part is 0

    // Place the integer part in the string
    while (count > 0)
    {
        str[str_pos++] = tmp[--count];
    }

    // Place the decimal point
    str[str_pos++] = '.';

    // Shift the fractional part into the integer part
    fractional = (fixedpoint_fractional_part(a) << INTEGER_BITS) & mask;
    do
    {
        fractional = (fractional & mask) * 10;                 // multiply the fractional part by 10 to get the next digit
        str[str_pos++] = '0' + (fractional >> BIT_WIDTH) % 10; // ones digit of the fractional part added to the value of char '0' to get the ascii value of the digit
        count++;
    } while (fractional != 0 && count < STRING_DECIMALS); // repeat until the fractional part is 0 or the desired precision is reached

    // Remove trailing 0s
    if (count > 1 && str[str_pos - 1] == '0')
        str[str_pos - 1] = '\0';
    else
        str[str_pos] = '\0';

    return str;
}
#endif // FIXEDPOINT_H